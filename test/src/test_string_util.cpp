#include "doctest.h"

#include <string>
#include <vector>
#include <span>

#include "util.h"

#include "cryptomatte/detail/string_util.h"

using namespace NAMESPACE_CRYPTOMATTE_API;


// -----------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------
TEST_CASE("Test str::split with basic comma-separated string")
{
	std::string input = "a,b,c";
	auto result = str::split(input, ",");
	CHECK(result == std::vector<std::string>{"a", "b", "c"});
}


// -----------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------
TEST_CASE("Test str::split with delimiter not present")
{
	std::string input = "abc";
	auto result = str::split(input, ",");
	CHECK(result == std::vector<std::string>{"abc"});
}


// -----------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------
TEST_CASE("Test str::split with consecutive delimiters")
{
	std::string input = "a,,b";
	auto result = str::split(input, ",");
	CHECK(result == std::vector<std::string>{"a", "", "b"});
}


// -----------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------
TEST_CASE("Test str::split with max_elems limit")
{
	std::string input = "a,b,c,d,e";
	auto result = str::split<2>(input, ",");
	CHECK(result == std::vector<std::string>{"a", "b", "c,d,e"});
}


// -----------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------
TEST_CASE("Test str::join with multiple strings")
{
	std::vector<std::string> input = { "a", "b", "c" };
	std::string result = join(input, ",");
	CHECK(result == "a,b,c");
}


// -----------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------
TEST_CASE("Test str::join with empty vector")
{
	std::vector<std::string> input = {};
	std::string result = join(input, ",");
	CHECK(result == "");
}


// -----------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------
TEST_CASE("Test str::lstrip basic usage")
{
	std::string input = "xxxHello";
	std::string result = str::lstrip(input, "x");
	CHECK(result == "Hello");
}


// -----------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------
TEST_CASE("Test str::lstrip with no matching prefix")
{
	std::string input = "Hello";
	std::string result = str::lstrip(input, "x");
	CHECK(result == "Hello");
}


// -----------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------
TEST_CASE("Test str::lstrip with full strip")
{
	std::string input = "xxxx";
	std::string result = str::lstrip(input, "x");
	CHECK(result == "");
}


// -----------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------
TEST_CASE("Test str::lstrip with limit on max_strips")
{
	std::string input = "xxxHello";
	std::string result = str::lstrip<2>(input, "x");
	CHECK(result == "xHello");
}


// -----------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------
TEST_CASE("Test str::casefold with mixed case")
{
	std::string input = "HeLLo";
	std::string result = str::casefold(input);
	CHECK(result == "hello");
}


// -----------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------
TEST_CASE("Test str::casefold with numbers and symbols")
{
	std::string input = "He110@WORLD!";
	std::string result = str::casefold(input);
	CHECK(result == "he110@world!");
}