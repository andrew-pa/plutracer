#pragma once

#include <iostream>
#include <string>
#include <sstream>
#include <cassert>
#include <functional>
#include <vector>
#include <list>
#include <map>
#include <set>
#include <tuple>
using namespace std;

// the urn language interpreter
// see docs/lang.mg and docs/scene-def.md for documentation on the language and defining scenes, respectivly 
namespace urn {

	// represents a basic lexical token
	struct token {
		uint32_t line, col;
		enum tktype {
			id, num, str, ctrl
		} type;
		string value;

		token() {}
		token(uint32_t ln, uint32_t cl, tktype t, const string& val)
			: line(ln), col(cl), type(t), value(val) {}
	};

	// takes a std::istream and provides an interface to read tokens out of it
	class token_stream {
		uint32_t curln, curcol;
		vector<string> lines;
		inline void next_line() {
			curcol = 0; curln++;
		}
		inline void next_char(int num = 1) {
			curcol += num;
			if (curcol > lines[curln].size() - 1) {
				curcol = 0; curln++;
			}
		}
		inline void next_ws() {
			while (more() && (isspace(lines[curln][curcol]) || iscntrl(lines[curln][curcol]))) next_char();
		}
		inline bool isterm(char c) {
			return isspace(c) || c == '(' || c == ')' || c == '[' || c == ']';
		}
		pair<token, int> get(int of = 0) {
			int i = curcol + of;
			while (isspace(lines[curln][i])) i++;
			string* lb = &lines[curln];
			if (i >= lb->size()) {
				lb = &lines[curln+1]; i = 0;
				while (isspace((*lb)[i])) i++;
			}
			char c = (*lb)[i];
			if (c == '(' || c == ')' || c == '[' || c == ']') {
				return{ token(curln, i, token::ctrl, string(1, c)), 1 };
			}
			else if (c == '-' || isdigit(c)) {
				string v;
				do {
					v += (*lb)[i++];
				} while (i < (*lb).size() && ((*lb)[i] == '.' || isdigit((*lb)[i])));
				return{ token(curln, i, token::num, v), v.size() };
			}
			else if (c == '"') {
				i++;
				string v;
				while (i < (*lb).size() && (*lb)[i] != '"') {
					v += (*lb)[i++];
				}
				if ((*lb)[i] != '"') throw runtime_error("string literal didn't close");
				i++;
				return{ token(curln, i, token::str, v), v.size() + 2 };
			}
			else {
				string v;
				while (i < (*lb).size() && !isterm((*lb)[i])) {
					v += (*lb)[i++];
				}
				return{ token(curln, i, token::id, v), v.size() };
			}
		}
	public:
		token_stream(istream&& i) : curln(0), curcol(0) {
			string s;
			while (i.good()) {
				getline(i, s);
				lines.push_back(s);
			}
		}

		token next() {
			next_ws();
			auto x = get();
			next_char(x.second);
			return x.first;
		}
		token peek(int offset = 0) {
			return get(offset).first;
		}
		bool more() {
			return curln < lines.size() && curcol < lines[curln].size();
		}

		//void expect(token::tktype t, const string& v = "");
	};

	struct value;

	namespace detail {
		template<typename T>
		struct get_helper { 
			inline static T get(value const* v); 
		};
	}

	// a value in urn
	struct value {
		struct func_t;

		// see docs/lang.md for details on each of these, esp Var/Id/Def/Val differences
		enum {
			Null,
			Int,		//	123
			Float,		//	1.23
			String,		//	"Hello, World!"
			Var,		//	thing
			Id,			//	'thing
			Def,		//	thing: <value>
			Val,		//	:thing
			Block,		//	[ <values...> ]
			Group,		//	( <values...> )
			Func,		//	func [a b] [a + b]
			NativeValue	//	<native value>
		} type;
		

		union {
			int64_t i;
			double f;
			string* s;
			func_t* fn;
			function<value()>* nvfn;
		};
		vector<value> vs;
		
protected:
		value(decltype(type) t, const string& v) : type(t), s(new string(v)) {}
		value(decltype(type) t, const vector<value>& v) : type(t), vs(v) {}

public:

		value(token_stream& ts);

		value() : type(Null), i(0) {}

		value(int64_t v) : type(Int), i(v) {}
		value(double v) : type(Float), f(v) {}
		value(const string& v) : value(String, v) {}
		value(const string& i, const value& v) : type(Def), s(new string(i)), vs({ v }) {}
		value(const vector<value>& vs) : value(Block, vs) {}
		value(const vector<string>& an, const value& b);
		value(function<value()> f) : type(NativeValue), nvfn(new function<value()>(f)) {}

		static value var(const string& i) { return value(Var, i); }
		static value id (const string& i) { return value(Id,  i); }
		static value val(const string& i) { return value(Val, i); }
		static value group(const vector<value>& v) { return value(Group, v); }
		// ^ these static methods are effectivly constructors, but they all have the same signature as
		//   the constructor that makes a string value, so they need to have names	

		friend ostream& operator <<(ostream& os, const value& v);

		inline bool is_null() const { return type == Null; }

		// obtain a C++ value for a urn value
		// this _does not_ evaluate the value
		template<typename T>
		inline T get() const { return detail::get_helper<T>::get(this); }

		inline const string& get_var() const {
			if (type != Var) throw runtime_error("expected value of type Var"); return *s;
		}
		inline const string& get_val() const {
			if (type != Val) throw runtime_error("expected value of type Val"); return *s;
		}
		inline const string& get_id() const {
			if (type != Id) throw runtime_error("expected value of type Id"); return *s;
		}

		inline double get_num() const {
			if (type == Int) return i;
			else if (type == Float) return f;
			else throw runtime_error("expected value of type Int or Float");
		}

		value(const value& v);
		value& operator=(const value& x);

		// get a named value out of a block
		// calling named_block_val("a") on the block '[a: 3 b: 7]' will return the value 3
		value named_block_val(const string& nm) const;

		inline value operator[](size_t ix) const {
			if (!(type == Block || type == Group)) throw runtime_error("expected value of type Block or Group");
			return vs[ix];
		}

		~value();
	};
	
	struct value::func_t {
		vector<string> argnames;
		value body;
		func_t(const vector<string>& an = {}, const value& b = value()) : argnames(an), body(b) {}
	};
	namespace detail {
		typedef pair<string, value> pair_string_value;
		typedef vector<value> vector_value;
#define getf(E, T) template<> struct get_helper<T> { inline static T get(value const* v) { if(v->type != value:: E) throw runtime_error("expected value of type " #E); return 
		getf(Int, int64_t) v->i; } };
		getf(Float, double) v->f; } };
		getf(String, string) *v->s; } };
		getf(Def, pair_string_value) { *v->s, v->vs[0] }; }};
		getf(Block, vector_value) v->vs; }};
		getf(NativeValue, value) (*v->nvfn)(); }};
#undef getf
		template<> struct get_helper<value::func_t> {
			inline static value::func_t get(value const* v) {
				if(v->type != value::Func) throw runtime_error("expected value of type Func");
				return *v->fn;
			}
		};	
	}

	// represents a evaluation context
	struct eval_context {
		list<map<string, value>> scope_values;
		void push_scope() { scope_values.push_front({}); }
		void pop_scope() { scope_values.pop_front(); }
		const value& name_value(const string& name, const value& v) {
			return scope_values.front()[name] = v;
		}
		const value& named_value(const string& name) {
			for (auto& sc : scope_values) {
				auto v = sc.find(name);
				if (v != sc.end()) return v->second;
			}
			return value();
		}


		set<string> binary_ops;

		bool is_binary_op(const string& nm) {
			return binary_ops.find(nm) != binary_ops.end();
		}

		eval_context() {
			push_scope();
		}

		// initialize the interpreter with some std functions
		// !! TODO: this is probably a silly way to do this and should change
		void create_std_funcs();
			
		// evaluate a value, but don't assume that blocks are executable
		value eval1(const value& v); 

		// evaluate a value and return the new value that is the result
		// this is effectivly urn's do function
		value eval(const value& v);

		// evaluate a value and return the new resulting value
		// this differs from eval because when it encounters blocks it creates a new block
		// that contains the value of evaluating all of the expressions in side of the block
		// so it effectivly urn's reduce function
		value reduce(const value& v);
	private:
		value evaluate_expr(const vector<value>& Vs, int& i);
	};
}
