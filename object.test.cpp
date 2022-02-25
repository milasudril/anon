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
	EXPECT_EQ(obj.contains("foobar"), true);

	obj.insert_or_assign("foobar", "kaka");

	EXPECT_EQ(std::get<std::string>(obj["foobar"]), "kaka");
	EXPECT_EQ(std::size(obj), 1);
	EXPECT_EQ(std::begin(obj)->first, "foobar");
}

TESTCASE(anon_object_insert_and_assign)
{
	anon::object obj;

	obj.insert("kaka", 12);

	EXPECT_EQ(std::get<int64_t>(obj["kaka"]), 12);
	EXPECT_EQ(std::size(obj), 1);
	EXPECT_EQ(std::begin(obj)->first, "kaka");

	REQUIRE_EQ(obj.contains("kaka"), true);
	obj.assign("kaka", "bulle");
	EXPECT_EQ(std::get<std::string>(obj["kaka"]), "bulle");
	EXPECT_EQ(std::size(obj), 1);
	EXPECT_EQ(std::begin(obj)->first, "kaka");

	try
	{
		REQUIRE_EQ(obj.contains("kaka"), true);
		obj.insert("kaka", 34);
		testcaseFailed();
	}
	catch(...)
	{}

	try
	{
		REQUIRE_EQ(obj.contains("foobar"), false);
		obj.assign("foobar", "nop");
		testcaseFailed();
	}
	catch(...)
	{
	}
}

TESTCASE(anon_object_key_access_not_found)
{
	anon::object obj;
	REQUIRE_EQ(obj.contains("foobar"), false);
	try
	{
		auto val = obj["foobar"];
		testcaseFailed();
	}
	catch(...)
	{}

	EXPECT_EQ(obj.find("foobar") == std::end(obj), true);
}

TESTCASE(anon_object_key_access_found)
{
	anon::object obj;
	obj.insert("val", 123);
	REQUIRE_EQ(obj.contains("val"), true);
	EXPECT_EQ(std::get<int64_t>(obj["val"]), 123);

	auto i = obj.find("val");
	EXPECT_EQ(i != std::end(obj), true);

	REQUIRE_EQ(i != std::end(obj), true);
	EXPECT_EQ(i->first, "val");
	EXPECT_EQ(std::get<int64_t>(i->second), 123);
}