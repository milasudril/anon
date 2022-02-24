//@	{"target":{"name":"obejct.test"}}

#include "./object.hpp"

#include "testfwk/testfwk.hpp"

TESTCASE(anon_object_insert_or_assign)
{
	anon::object obj;
	REQUIRE_EQ(obj.contains("foobar"), false);

	obj.insert_or_assign("foobar", 1);

	EXPECT_EQ(std::get<int64_t>(obj["foobar"]), 1);
	EXPECT_EQ(std::size(obj), 1);
	EXPECT_EQ(std::begin(obj)->first, "foobar");

	obj.insert_or_assign("foobar", "kaka");

	EXPECT_EQ(std::get<std::string>(obj["foobar"]), "kaka");
	EXPECT_EQ(std::size(obj), 1);
	EXPECT_EQ(std::begin(obj)->first, "foobar");
}