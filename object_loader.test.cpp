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
	auto obj = anon::load(buffer{R"(    obj    {
	a_key: obj{
		a_string: str{this is a test with \\ and { } \}
		a_second_string: str{foobar\}
		\}
	\}
blah)"});
}