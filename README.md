# Anon -- Another Object Notation

Anon is a text-based format used to store data. Its structure is similar to JSON in sense that Anon
stores "objects", as a recursive key-value pair structure. Unlike JSON, chunks of anon-encoded data
can be embedded inside another stream. For practicality and portability reasons, anon has
restrictions on valid property names. See key.hpp for more more information about key validation.
Also, anon requires type annotations, and for arrays, they
must be homogenous.

The syntax of anon has been designed to minimize the need for escaping strings. To achieve this, the
parser's value mode will accept any character except `\`, which switches to control character mode.
In this mode `;` and `}` is treated as delimiters. Everything else is accepted, and the parser
returns to the "value" mode.

## Example syntax

```
obj{
	an_object: obj{
		a_string: str{this is a test with \\ and { } \}
		a_second_string: str{foobar\}
		a_third_level: obj{
			kaka:str{bulle\}
		\}
	\}
	a_string: str{a string\}
	an_array_of_strings: str*{First string\;Second string\;Third string\;\}
	an_array_of_objects: obj*{
		foobar:str*{A\;B\;C\;\}
		kaka:str*{D\;E\;F\;\}\;

		key_in_second_obj:str{Hello world\}\;
	\}
	an_i32: i32{1\}
	an_array_of_i32: i32*{1\;2\;3\;\}
	an_i64: i64{1\}
	an_array_of_i64: i64*{1\;2\;3\;\}
	an_u32: u32{1\}
	an_array_of_u32: u32*{1\;2\;3\;\}
	an_u64: u64{1\}
	an_array_of_u64: u64*{1\;2\;3\;\}
	an_f32: f32{1\}
	an_array_of_f32: f32*{1\;2\;3\;\}
	an_f64: f64{1\}
	an_array_of_f64: f64*{1\;2\;3\;\}
\}
```