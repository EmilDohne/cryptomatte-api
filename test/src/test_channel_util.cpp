#include "doctest.h"

#include <vector>
#include <string>

#include "util.h"

#include "cryptomatte/detail/channel_util.h"

using namespace NAMESPACE_CRYPTOMATTE_API;

// -----------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------
TEST_CASE("Test detail::map_to_channel_type valid values")
{
	auto res_r_1 = detail::map_to_channel_type("r");
	auto res_r_2 = detail::map_to_channel_type("R");
	auto res_r_3 = detail::map_to_channel_type("red");
	CHECK(res_r_1 == detail::channel_type::red);
	CHECK(res_r_2 == detail::channel_type::red);
	CHECK(res_r_3 == detail::channel_type::red);

	auto res_g_1 = detail::map_to_channel_type("g");
	auto res_g_2 = detail::map_to_channel_type("G");
	auto res_g_3 = detail::map_to_channel_type("green");
	CHECK(res_g_1 == detail::channel_type::green);
	CHECK(res_g_2 == detail::channel_type::green);
	CHECK(res_g_3 == detail::channel_type::green);

	auto res_b_1 = detail::map_to_channel_type("b");
	auto res_b_2 = detail::map_to_channel_type("B");
	auto res_b_3 = detail::map_to_channel_type("blue");
	CHECK(res_b_1 == detail::channel_type::blue);
	CHECK(res_b_2 == detail::channel_type::blue);
	CHECK(res_b_3 == detail::channel_type::blue);

	auto res_a_1 = detail::map_to_channel_type("a");
	auto res_a_2 = detail::map_to_channel_type("A");
	auto res_a_3 = detail::map_to_channel_type("alpha");
	CHECK(res_a_1 == detail::channel_type::alpha);
	CHECK(res_a_2 == detail::channel_type::alpha);
	CHECK(res_a_3 == detail::channel_type::alpha);
}


// -----------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------
TEST_CASE("Test detail::map_to_channel_type invalid value"
	* doctest::no_breaks(true)
	* doctest::no_output(true)
	* doctest::should_fail(true)
)
{
	auto _ = detail::map_to_channel_type("foo");
}


// -----------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------
TEST_CASE("Test detail::channel_repr from valid strings")
{
	auto repr_1 = detail::channel_repr("MyCrypto00.R");
	auto repr_2 = detail::channel_repr("SomeVal00.a");
	auto repr_3 = detail::channel_repr("typename99.R");

	CHECK(repr_1._typename == "MyCrypto");
	CHECK(repr_1.index == 0);
	CHECK(repr_1.type == detail::channel_type::red);
	CHECK(repr_1.channelname() == "MyCrypto00.R");

	CHECK(repr_2._typename == "SomeVal");
	CHECK(repr_2.index == 0);
	CHECK(repr_2.type == detail::channel_type::alpha);
	CHECK(repr_2.channelname() == "SomeVal00.a");


	CHECK(repr_3._typename == "typename");
	CHECK(repr_3.index == 99);
	CHECK(repr_3.type == detail::channel_type::red);
	CHECK(repr_3.channelname() == "typename99.R");

}


// -----------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------
TEST_CASE("Test detail::channel_repr no typename"
	* doctest::no_breaks(true)
	* doctest::no_output(true)
	* doctest::should_fail(true)
)
{
	auto _ = detail::channel_repr("00.R");
}


// -----------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------
TEST_CASE("Test detail::channel_repr no index"
	* doctest::no_breaks(true)
	* doctest::no_output(true)
	* doctest::should_fail(true)
)
{
	auto _ = detail::channel_repr("CryptoAsset.R");
}


// -----------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------
TEST_CASE("Test detail::channel_repr no channel type"
	* doctest::no_breaks(true)
	* doctest::no_output(true)
	* doctest::should_fail(true)
)
{
	auto _ = detail::channel_repr("CryptoAsset00.");
}


// -----------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------
TEST_CASE("Test detail::channel_repr single index padding"
	* doctest::no_breaks(true)
	* doctest::no_output(true)
	* doctest::should_fail(true)
)
{
	auto _ = detail::channel_repr("CryptoAsset1.R");
}


// -----------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------
TEST_CASE("Test detail::channel_repr index 3 digits"
	* doctest::no_breaks(true)
	* doctest::no_output(true)
	* doctest::should_fail(true)
)
{
	auto _ = detail::channel_repr("CryptoAsset123.R");
}


// -----------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------
TEST_CASE("Test detail::channel_repr ordering")
{
	SUBCASE("Order by index")
	{
		auto a = detail::channel_repr("CryptoAsset01.R");
		auto b = detail::channel_repr("CryptoAsset02.R");

		CHECK(a < b);
	}

	SUBCASE("Order by channel type r and a")
	{
		auto a = detail::channel_repr("CryptoAsset00.R");
		auto b = detail::channel_repr("CryptoAsset00.alpha");

		CHECK(a < b);
	}
	SUBCASE("Order by channel type g and r")
	{
		auto a = detail::channel_repr("CryptoAsset00.red");
		auto b = detail::channel_repr("CryptoAsset00.g");

		CHECK(a < b);
	}

	SUBCASE("Order by index and channel type")
	{
		// Since the index takes precedence, a should be less than b even though
		// b is less by channel type.
		auto a = detail::channel_repr("CryptoAsset00.alpha");
		auto b = detail::channel_repr("CryptoAsset01.R");

		CHECK(a < b);
	}
}


// -----------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------
TEST_CASE("Test detail::sort_and_validate_channels already sorted")
{
	std::vector<std::string> input_channels = {
		"Cryptomatte00.r",
		"Cryptomatte00.g",
		"Cryptomatte00.b",
		"Cryptomatte00.a",
		"Cryptomatte01.r",
		"Cryptomatte01.g",
		"Cryptomatte01.b",
		"Cryptomatte01.a"
	};
	auto sorted = detail::sort_and_validate_channels(input_channels);
	test_util::check_vector_verbose(sorted, input_channels);
}


// -----------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------
TEST_CASE("Test detail::sort_and_validate_channels out of order")
{
	std::vector<std::string> input_channels = {
		"Cryptomatte00.r",
		"Cryptomatte00.b",
		"Cryptomatte00.a",
		"Cryptomatte00.g", // <-- This one is wrong
		"Cryptomatte01.r",
		"Cryptomatte01.g",
		"Cryptomatte01.a",
		"Cryptomatte01.b"  // <-- So is this
	};
	std::vector<std::string> expected_channels = {
		"Cryptomatte00.r",
		"Cryptomatte00.g",
		"Cryptomatte00.b",
		"Cryptomatte00.a",
		"Cryptomatte01.r",
		"Cryptomatte01.g",
		"Cryptomatte01.b",
		"Cryptomatte01.a"
	};
	auto sorted = detail::sort_and_validate_channels(input_channels);
	test_util::check_vector_verbose(sorted, expected_channels);
}


// -----------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------
TEST_CASE("Test detail::sort_and_validate_channels partial rank-coverage")
{
	std::vector<std::string> input_channels = {
		"Cryptomatte00.r",
		"Cryptomatte00.b",
		"Cryptomatte00.a",
		"Cryptomatte00.g", // <-- This one is wrong
		"Cryptomatte01.r",
		"Cryptomatte01.g"  // <-- This is just 2 rank-coverage pairs, but we accept this.
	};
	std::vector<std::string> expected_channels = {
		"Cryptomatte00.r",
		"Cryptomatte00.g",
		"Cryptomatte00.b",
		"Cryptomatte00.a",
		"Cryptomatte01.r",
		"Cryptomatte01.g"
	};
	auto sorted = detail::sort_and_validate_channels(input_channels);
	test_util::check_vector_verbose(sorted, expected_channels);
}


// -----------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------
TEST_CASE("Test detail::sort_and_validate_channels missing rank-coverage"
	* doctest::no_breaks(true)
	* doctest::no_output(true)
	* doctest::should_fail(true)
)
{
	std::vector<std::string> input_channels = {
		"Cryptomatte00.r",
		"Cryptomatte00.g",
		"Cryptomatte00.b",
		"Cryptomatte00.a",
		"Cryptomatte01.r",
		"Cryptomatte01.g",
		"Cryptomatte01.b"
		// Missing the alpha here!
	};
	// This should now throw.
	auto sorted = detail::sort_and_validate_channels(input_channels);
	test_util::check_vector_verbose(sorted, expected_channels);
}



// -----------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------
TEST_CASE("Test detail::sort_and_validate_channels missing channel"
	* doctest::no_breaks(true)
	* doctest::no_output(true)
	* doctest::should_fail(true)
)
{
	std::vector<std::string> input_channels = {
		"Cryptomatte00.r",
		"Cryptomatte00.g",
		// channel b is missing here!
		// channel a is missing here!
		"Cryptomatte01.r",
		"Cryptomatte01.g",
		"Cryptomatte01.b",
		"Cryptomatte01.a"
	};
	// This should now throw.
	auto sorted = detail::sort_and_validate_channels(input_channels);
}


// -----------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------
TEST_CASE("Test detail::sort_and_validate_channels missing index"
	* doctest::no_breaks(true)
	* doctest::no_output(true)
	* doctest::should_fail(true)
)
{
	std::vector<std::string> input_channels = {
		"Cryptomatte00.r",
		"Cryptomatte00.g",
		"Cryptomatte00.b",
		"Cryptomatte00.a",
		// We start with 02 right away, but 01 is missing!
		"Cryptomatte02.r",
		"Cryptomatte02.g",
		"Cryptomatte02.b",
		"Cryptomatte02.a"
	};
	// This should now throw.
	auto sorted = detail::sort_and_validate_channels(input_channels);
}


// -----------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------
TEST_CASE("Test detail::sort_and_validate_channels missing index"
	* doctest::no_breaks(true)
	* doctest::no_output(true)
	* doctest::should_fail(true)
)
{
	std::vector<std::string> input_channels = {
		"Cryptomatte00.r",
		"Cryptomatte00.g",
		"Cryptomatte00.b",
		"Cryptomatte00.a",
		// We start with 02 right away, but 01 is missing!
		"Cryptomatte02.r",
		"Cryptomatte02.g",
		"Cryptomatte02.b",
		"Cryptomatte02.a"
	};
	// This should now throw.
	auto sorted = detail::sort_and_validate_channels(input_channels);
}


// -----------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------
TEST_CASE("Test detail::sort_and_validate_channels empty"
	* doctest::no_breaks(true)
	* doctest::no_output(true)
	* doctest::should_fail(true)
)
{
	std::vector<std::string> input_channels = {};
	// This should now throw.
	auto sorted = detail::sort_and_validate_channels(input_channels);
}