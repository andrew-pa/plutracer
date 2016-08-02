# urn language

*Aside: this is very much a work in progress*

The urn language is very similar to REBOL. It provides both a way to notate data as well as express executable code.

# syntax

There are a few different types of basic values
Although some may seem suited/named for the purpose of evaluation, they are general purpose and also can be used for whatever purpose in specifying data or DSLs

+ Numbers

	Exactly like one might expect, floating point specified by simply including a decimal point
	Ex: `123` `123.345`

+ Strings

	Quoted with double quotes: `"like so"`

+ Identifiers (called words by REBOL)

	a series of non-reserved characters surrounded by whitespace or reserved characters
	urn only reserves a few characters: `[]()':`
	As you can see, urn doesn't reserve any of the binary operators you might expect from an C-like language, so you have to include whitespace around them

	Additionally, modifiers can be applied to identifiers. These modifiers are `'` and `:`. They have special meaning for evaluation, and parse as distinctly different value types, but outside of the evaluation context are just markers on an identifier.

+ Definitions

	An identifier followed directly by a colon, then whitespace and then a value
	Definitions only take the value *directly* after the definition so to include more than one value (or specify a whole expression in evaluation) use a group or block to contain the values
	Ex: `stuff: 3` `foo: "hi" bar: 7 cat: foo` `stuff: [ 3 4 6 ]`

+ Blocks

	Blocks are an integral part of urn. They are specified by putting values inside of square brackets (`[]`)
	Ex: `[ 2 3 4 5 6 ]` `[ stuff: 7 other-stuff: "this is stuff" example7: 10.3903 ]`

+ Groups

	Groups are like blocks, but they use parenthesis (`()`) instead of square brackets
	Ex: `(1 + 2)` `(1.03 * 6.33 + 2.03)`

# evaluation

Evaluation is separate from the syntax in urn. That is to say that unless specifically evaluated, a block or group will stay the way it is, and can be passed around the program as a data structure. This homoiconicity is what makes the REBOL family of languages interesting, as it is distinct from the LISP flavor of homoiconicity.

To evaluate a value, one uses the `do` function. Additionally the top level value is implicitly evaluated.
In the context of evaluation, the basic values have additional meaning to evaluation, as well as rules to what their resulting values are. In addition, they are grouped into expressions

## expressions

Expressions in their most basic form are a value that will evaluate to a function followed by its arguments. The expression will only span to include as many values as there are arguments. Additionally, identifiers that have been registered as binary operators can be placed after a value, and will receive both the proceeding value and the next one as function arguments
Examples:

- `1 + 3`
- `concat [1 2] [3 6]`
- `print ((1 + 2) * 3)`
- `do [ a: 9 b: 11 (a * 100) + b ]`
- `reduce [ 1 + 8 9 * 11 200 / 3 "Hello, world!"]`
- `run-something-interesting [ junk: "something" api-key:"089adf09f3u093" ] "http://something.example.com/is/happening/here" 90 329`

There are some interesting things of note for some of the basic values

+ Groups

	Groups are treated as mathematical parenthesis. They evaluate the expression that is inside them, they only allow one expression inside them total. Groups also _do not_ create a new scope

+ Identifiers

	As seen in the syntax section, there are three different 'styles' of identifier. These each have a special meaning when they are evaluated.

	- Identifiers with no modifier

		These identifiers evaluate to the value that has been named for them in the current context. However, if they are functions they are instead evaluated as a function call with the arguments specified as the values that follow the identifier

	- Identifiers with the `'` modifier

		These identifiers evaluate to themselves, without any value lookup or function calling. They are useful to specify names of things.

	- Identifiers with the `:` modifier

		These identifiers evaluate to the value they have been named for, but they don't count for creating function calls, and so are useful in passing the actual function value around

+ Blocks

	Blocks, unlike __any other values__ are not evaluated when they are passed to functions. This allows them to be used to create data structures. Blocks, unlike groups, _do_ create a new scope. This means that definitions inside of a block are local to that block. Blocks can be evaluated using a variety of functions which operate slightly differently:

	- `do` evaluates expressions out of the block until it reaches the end, then returns the result of the last expression
	- `reduce` evaluates expressions out of the block and stores the resulting value in a new block in the order of the expression of the old

+ Definitions

	Definitions, when evaluated, store the evaluated result of the value they are specified under the definition name. The return value is that of the stored value.


