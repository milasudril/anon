//@	{"target":{"name":"serialzer.test"}}

#include "./serializer.hpp"

#include "./deserializer.hpp"

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

	struct writebuff
	{
		std::string buffer;
	};

	void write(char ch, writebuff& buff)
	{
		buff.buffer+=ch;
	}

	void write(char const* data, writebuff& buff)
	{
		buff.buffer+=data;
	}
}

TESTCASE(anon_load_store_and_load)
{
	buffer buff_in{R"(
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
\}blah)"};

	auto obj_1 = anon::load(buff_in);

	writebuff buff_out{};
	store(obj_1, buff_out);

	auto obj_2 = anon::load(buffer{buff_out.buffer});

	EXPECT_EQ(obj_1, obj_2);
}