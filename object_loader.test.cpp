//@	{"target":{"name":"object_loader.test"}}

#include "./object_loader.hpp"

#include "testfwk/testfwk.hpp"

namespace
{
	struct buffer
	{
		explicit buffer(std::string_view sv):data{sv}, ptr{std::begin(data)}
		{}

		std::string_view data;
		char const* ptr;
	};

	std::optional<char> read_byte(buffer& buff)
	{
		auto ret = buff.ptr != std::end(buff.data)? std::optional{*buff.ptr} : std::nullopt;
		++buff.ptr;
		return ret;
	}
}

TESTCASE(anon_load)
{
	buffer buff{R"(
obj{
	an_object: obj{
		a_string: str{this is a test with \\ and { } \}
		a_second_string: str{foobar\}
		a_third_level: obj{
			kaka:str{bulle\}
		\}
	\}
	a_string: str{a string\}
	an_array_of_strings: str...{First string\;Second stirng\;Third string\}
	an_array_of_objects: obj...{
		foobar:str...{A\;B\;C\}
		kaka:str...{D\;E\;F\}\;

		second_obj:str{Hello world\}
	\}
	an_i32: i32{1\}
	an_array_of_i32: i32...{1\;2\;3\}
	an_i64: i64{1\}
	an_array_of_i64: i64...{1\;2\;3\}
	an_u32: u32{1\}
	an_array_of_u32: u32...{1\;2\;3\}
	an_u64: u64{1\}
	an_array_of_u64: u64...{1\;2\;3\}
	an_f32: f32{1\}
	an_array_of_f32: f32...{1\;2\;3\}
	an_f64: f64{1\}
	an_array_of_f64: f64...{1\;2\;3\}
\}blah)"};

	auto obj = anon::load(buff);
	EXPECT_EQ(std::size(obj), 16);
	EXPECT_EQ(*buff.ptr, 'b');
}