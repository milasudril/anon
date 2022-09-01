//@	{"target":{"name":"object_loader.test"}}

#include "./object_deserializer.hpp"

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
		a_string: str{this is a test with ; \\ and { } \}
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
	an_empty_array_1: i32*{\}
	an_empty_array_2: obj*{\}
\}blah)"};

	auto obj = anon::load(buff);
	EXPECT_EQ(*buff.ptr, 'b');

	EXPECT_EQ(std::size(obj), 18);

	{
		auto& an_object = std::get<anon::object>(obj["an_object"]);
		EXPECT_EQ(std::size(an_object), 3);
		EXPECT_EQ(std::get<std::string>(an_object["a_string"]), R"(this is a test with ; \ and { } )");
		EXPECT_EQ(std::get<std::string>(an_object["a_second_string"]), "foobar");
		auto& a_third_level = std::get<anon::object>(an_object["a_third_level"]);
		EXPECT_EQ(std::size(a_third_level), 1)
		EXPECT_EQ(std::get<std::string>(a_third_level["kaka"]), "bulle");
	}

	EXPECT_EQ(std::get<std::string>(obj["a_string"]), "a string");

	{
		auto& an_array_of_strings = std::get<std::vector<std::string>>(obj["an_array_of_strings"]);
		REQUIRE_EQ(std::size(an_array_of_strings), 3);
		EXPECT_EQ(an_array_of_strings[0], "First string");
		EXPECT_EQ(an_array_of_strings[1], "Second string");
		EXPECT_EQ(an_array_of_strings[2], "Third string");
	}

	{
		auto& an_array_of_objects = std::get<std::vector<anon::object>>(obj["an_array_of_objects"]);
		REQUIRE_EQ(std::size(an_array_of_objects), 2);
		{
			auto& obj = an_array_of_objects[0];
			{
				auto& foobar = std::get<std::vector<std::string>>(obj["foobar"]);
				REQUIRE_EQ(std::size(foobar), 3);
				EXPECT_EQ(foobar[0], "A");
				EXPECT_EQ(foobar[1], "B");
				EXPECT_EQ(foobar[2], "C");
			}

			{
				auto& kaka = std::get<std::vector<std::string>>(obj["kaka"]);
				REQUIRE_EQ(std::size(kaka), 3);
				EXPECT_EQ(kaka[0], "D");
				EXPECT_EQ(kaka[1], "E");
				EXPECT_EQ(kaka[2], "F");
			}
		}

		{
			auto& obj = an_array_of_objects[1];
			{
				auto& key_in_second_obj = std::get<std::string>(obj["key_in_second_obj"]);
				EXPECT_EQ(key_in_second_obj, "Hello world");
			}
		}
	}

	EXPECT_EQ(std::get<int32_t>(obj["an_i32"]), 1);
	EXPECT_EQ(std::get<int64_t>(obj["an_i64"]), 1);
	EXPECT_EQ(std::get<float>(obj["an_f32"]), 1.0f);
	EXPECT_EQ(std::get<double>(obj["an_f64"]), 1.0);
	EXPECT_EQ(std::size(std::get<std::vector<int32_t>>(obj["an_empty_array_1"])), 0);
	EXPECT_EQ(std::size(std::get<std::vector<anon::object>>(obj["an_empty_array_2"])), 0);
}