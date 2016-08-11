#include "urn.h"
#include <algorithm>

namespace urn {
	value::value(const vector<string>& an, const value& b) : type(Func), fn(new func_t(an, b)) {}

	value::value(token_stream& ts) {
		auto t = ts.next();
		if (t.type == token::num) {
			if (t.value.find('.') != t.value.npos) {
				type = Float;
				f = atof(t.value.c_str());
			}
			else {
				type = Int;
				i = atoll(t.value.c_str());
			}
		}
		else if (t.type == token::str) {
			type = String;
			s = new string(t.value);
		}
		else if(t.type == token::id) {
			if (t.value[0] == ':') {
				type = Val; 
				s = new string(t.value.substr(1));
			}
			else if (t.value[0] == '\'') {
				type = Id;
				s = new string(t.value.substr(1));
			}
			else if (t.value[t.value.size()-1] == ':') {
				type = Def;
				s = new string(t.value.substr(0, t.value.size()-1));
				vs = { value(ts) };
			}
			else {
				type = Var;
				s = new string(t.value);
			}
		}
		else if (t.type == token::ctrl) {
			string end_char;
			if (t.value == "(") {
				end_char = ")";
				type = Group;
			}
			else if (t.value == "[") {
				end_char = "]";
				type = Block;
			}
			else 
				throw;
			vs = vector<value>();
			int cnt = 0;
			while (ts.more()) {
				auto nt = ts.peek();
				if (nt.type == token::ctrl && nt.value == end_char) {
					ts.next(); break;
				}
				vs.push_back(value(ts));
				cnt++;
			}
		}
	}
	ostream& operator<<(ostream& os, const value& v) {
		switch (v.type)
		{
		case value::Int: os << v.i; break;
		case value::Float: os << v.f; break;
		case value::String:
			os << "\"" << *v.s << "\"";
			break;
		case value::Var:
			os << *v.s;
			break;
		case value::Val:
			os << ":" << *v.s;
			break;
		case value::Id:
			os << "'" << *v.s;
			break;
		case value::Def: os << *v.s << ":" << v.vs[0]; break;
		case value::Block:
			os << "[ ";
			for (const auto& x : v.vs) os << x << " ";
			os << "]";
			break;
		case value::Group:
			os << "( ";
			for (const auto& x : v.vs) os << x << " ";
			os << ")";
			break;
		case value::Func:
			os << "func [ ";
			for (const auto& n : v.fn->argnames) os << n << " ";
			os << "] " << v.fn->body;
			break;
		case value::NativeValue:
			os << "<native value>";
			break;
		}
		return os;
	}


	value::value(const value& v) {
		type = v.type;
		switch (type)
		{
		case String:
		case Var:
		case Id:
		case Val:
			s = new string(*v.s);
			break;
		case Def:
			s = new string(*v.s);
		case Block:
		case Group:
			vs = v.vs;
			break;
		case Func:
			fn = new func_t(*v.fn);
			break;
		case NativeValue:
			nvfn = new function<value()>(*v.nvfn);
			break;
		default:
			i = v.i;
			break;
		}
	}

	value& value::operator=(const value& v) {
		type = v.type;
		switch (type)
		{
		case String:
		case Var:
		case Id:
		case Val:
			s = new string(*v.s);
			break;
		case Def:
			s = new string(*v.s);
		case Block:
		case Group:
			vs = v.vs;
			break;
		case Func:
			fn = new func_t(*v.fn);
			break;
		case NativeValue:
			nvfn = new function<value()>(*v.nvfn);
			break;
		default:
			i = v.i;
			break;
		}
		return *this;
	}

	value::~value() {
		switch (type) {
		case Def:
		case String:
		case Var:
		case Val:
		case Id:
			delete s;
			break;
		case Func:
			delete fn;
			break;
		case NativeValue:
			delete nvfn;
			break;
		}
	}

	value value::named_block_val(const string& nm) const {
		auto vs = get<vector<value>>();
		auto v = find_if(vs.begin(), vs.end(), [&nm](value v) {
			return v.type == Def && *v.s == nm;
		});
		return v == vs.end() ? value() : v->vs[0];
	}
	bool value::has_block_val_named(const string& nm) const {
		auto vs = get<vector<value>>();
		auto v = find_if(vs.begin(), vs.end(), [&nm](value v) {
			return v.type == Def && *v.s == nm;
		});
		return v != vs.end();
	}

	value eval_context::eval1(const value& v) { //resolve var/val/defs while retaining blocks as unevaluated
		switch (v.type) {
		case value::Int:
		case value::String:
		case value::Float:
		case value::Id:
		case value::Block:
			return v;
		case value::Group: {
			int i = 0;
			auto rv = evaluate_expr(v.vs, i);
			if (i < v.vs.size() - 1) throw;
			return rv;
		}
		case value::Var:
		case value::Val:
			return named_value(*v.s);
		case value::Def:
			return name_value(*v.s, eval1(v.vs.at(0)));
		case value::NativeValue:
			return v.get<value>();
		}
	}


	value eval_context::eval(const value& v) {
		if (v.type == value::Block) {
			push_scope();
			value rv;
			for (int i = 0; i < v.vs.size();) {
				rv = evaluate_expr(v.vs, i);
			}
			pop_scope();
			return rv;
		}
		return eval1(v);
	}

	value eval_context::reduce(const value& v) {
		if (v.type == value::Block) {
			push_scope();
			vector<value> rv;
			for (int i = 0; i < v.vs.size();) {
				rv.push_back(evaluate_expr(v.vs, i));
			}
			pop_scope();
			return value(rv);
		}
		return eval1(v);
	}


	value eval_context::evaluate_expr(const vector<value>& Vs, int& i) {
		auto fv = eval1(Vs[i]);
		if (Vs[i].type != value::Val && !fv.is_null() && fv.type == value::Func) { //is indeed a function
			auto F = fv.get<value::func_t>();
			push_scope();
			i++;
			for (int j = 0; i < Vs.size() && j < F.argnames.size(); ++i, ++j) {
				name_value(F.argnames[j], eval1(Vs[i]));
			}
			auto rv = eval(F.body);
			pop_scope();
			return rv;
		}
		else {
			while (i + 1 < Vs.size() && Vs[i + 1].type == value::Var) {
				auto op = *Vs[i + 1].s;
				if (is_binary_op(op)) {
					i++;
					auto F = named_value(op).get<value::func_t>();
					push_scope();
					name_value(F.argnames[0], fv);
					name_value(F.argnames[1], eval1(Vs[++i]));
					fv = eval(F.body);
					pop_scope();
				}
				else break;
			}
			i++;
			return fv;
		}
	}





	value block_format(const vector<value>& fmt, const vector<value>& vals) {
		vector<value> vs;
		for (int i = 0; i < fmt.size(); ++i) {
			if (fmt[i].type == value::Val) {
				try {
					auto idx = stoi(fmt[i].get_val());
					vs.push_back(vals[idx]);
				}
				catch (const invalid_argument& e) {
					vs.push_back(fmt[i]);
				}
			}
			else if (fmt[i].type == value::Block) {
				vs.push_back(block_format(fmt[i].get<vector<value>>(), vals));
			}
			else {
				vs.push_back(fmt[i]);
			}
		}
		return value(vs);
	}

	void eval_context::create_std_funcs() {

		// for now literally the bare minimum to create a grid of spheres and some stuff i've already done
		binary_ops.insert("+"); // someone should really go write the rest of these math functions
		name_value("+", value(vector<string>{"a", "b"}, value([this]()->value {
			auto a = named_value("a");
			auto b = named_value("b");
			if (a.type == value::Int) { //there should be a better, more general, more exposed to the user way of doing function overloading
				if (b.type == value::Int) {
					return value(a.get<int64_t>() + b.get<int64_t>());
				}
				else if (b.type == value::Float) {
					return value((double)a.get<int64_t>() + b.get<double>());
				}
			}
			else if (a.type == value::Float) {
				if (b.type == value::Int) {
					return value(a.get<double>() + (double)b.get<int64_t>());
				}
				else if (b.type == value::Float) {
					return value(a.get<double>() + b.get<double>());
				}
			}
			else if (a.type == value::Block && b.type == value::Block) {
				vector<value> vs;
				auto as = a.get<vector<value>>(), bs = b.get<vector<value>>();
				vs.insert(vs.end(), as.begin(), as.end());
				vs.insert(vs.end(), bs.begin(), bs.end());
				return value(vs);
			}
			return value();
		})));
		name_value("do", value(vector<string>{"v"}, value([this]()->value {
			return eval(named_value("v"));
		})));
		name_value("reduce", value(vector<string>{"v"}, value([this]()->value {
			return reduce(named_value("v"));
		})));
		name_value("print", value(vector<string>{"v"}, value([this]()->value {
			cout << named_value("v");
			return value();
		})));
		name_value("func", value(vector<string>{"args", "body"}, value([this]()->value {
			vector<string> an;
			auto ag = named_value("args").get<vector<value>>();
			for (const auto& v : ag)
				an.push_back(v.get_var());
			return value(an, named_value("body"));
		})));
		name_value("concat-all", value(vector<string>{"vs"}, value([this]()->value { //this could be removed with a implementation of fold
			auto vs = named_value("vs").get<vector<value>>();
			vector<value> vls;
			for (const auto& v : vs) {
				auto vsvs = v.get<vector<value>>();
				vls.insert(vls.end(), vsvs.begin(), vsvs.end());
			}
			return value(vls);
		})));
		name_value("append", value(vector<string>{"block", "value"}, value([this]()->value {
			auto vs = named_value("block").get<vector<value>>();
			vs.push_back(named_value("value"));
			return value(vs);
		})));
		name_value("collect-range", value(vector<string>{"var", "range", "body"}, value([this]()->value {
			// this has many problems, esp performance
			// should: 1) have optional step 2) work with things that aren't ints 3) not be super slow
			auto rng = named_value("range").get<vector<value>>();
			auto start = rng[0].get<int64_t>();
			auto end = rng[1].get<int64_t>();
			auto id = named_value("var").get_id();
			auto body = named_value("body");
			push_scope();
			vector<value> vs;
			for (int64_t i = start; i < end; ++i) {
				name_value(id, value(i));
				vs.push_back(eval(body));
			}
			pop_scope();
			return value(vs);
		})));
		name_value("block-format", value(vector<string>{"format", "values"}, value([this]()->value {
			// block-format [ stuff :0 [other stuff] :1 ] [ "hi" 7 ] ====> [stuff "hi" [other stuff] 7]
			// block-format [ junk [:0 :1] "hi" :2 ] [ "a" "b" 9 ] ====> [ junk ["a" "b"] "hi" 9 ]
			
			auto fmt = named_value("format").get<vector<value>>();
			auto vls = reduce(named_value("values")).get<vector<value>>();
			return block_format(fmt, vls);
		})));
	}
}
