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
		case value::Var:
		case value::Val:
		case value::Id:
			os << *v.s;
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


	value eval_context::evaluate_expr(const vector<value>& Vs, int& i) {
		auto fv = eval(Vs[i]);
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
}
