#include "doctest.h"

#include <vector>
#include <string>

#include "util.h"

#include "cryptomatte/detail/detail.h"

using namespace NAMESPACE_CRYPTOMATTE_API;

// -----------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------
TEST_CASE("detail::hex_str_to_uint32_t: Valid hex string")
{
	CHECK(detail::hex_str_to_uint32_t("00000000") == 0x00000000);
	CHECK(detail::hex_str_to_uint32_t("00000001") == 0x00000001);
	CHECK(detail::hex_str_to_uint32_t("ffffffff") == 0xFFFFFFFF);
	CHECK(detail::hex_str_to_uint32_t("deadbeef") == 0xDEADBEEF);

	CHECK(detail::uint32_t_to_hex_str(0x00000000) == "00000000");
	CHECK(detail::uint32_t_to_hex_str(0x00000001) == "00000001");
	CHECK(detail::uint32_t_to_hex_str(0xFFFFFFFF) == "ffffffff");
	CHECK(detail::uint32_t_to_hex_str(0xDEADBEEF) == "deadbeef");
}

// -----------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------
TEST_CASE("detail::hex_str_to_uint32_t: Case insensitivity")
{
	CHECK(detail::hex_str_to_uint32_t("DEADBEEF") == 0xDEADBEEF);
	CHECK(detail::hex_str_to_uint32_t("deadbeef") == 0xDEADBEEF);
	CHECK(detail::hex_str_to_uint32_t("DeadBeEf") == 0xDEADBEEF);
}

// -----------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------
TEST_CASE("detail::hex_str_to_uint32_t: Invalid hex length")
{
	CHECK_THROWS_AS(detail::hex_str_to_uint32_t("123"), std::invalid_argument);
	CHECK_THROWS_AS(detail::hex_str_to_uint32_t("123456789"), std::invalid_argument);
	CHECK_THROWS_AS(detail::hex_str_to_uint32_t(""), std::invalid_argument);
}


// -----------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------
TEST_CASE("detail::hex_str_to_uint32_t: Invalid characters in hex string")
{
	CHECK_THROWS_AS(detail::hex_str_to_uint32_t("zzzzzzzz"), std::runtime_error);
}


// -----------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------
TEST_CASE("detail::hex_str_to_uint32_t: Max boundary value")
{
	CHECK(detail::hex_str_to_uint32_t("ffffffff") == std::numeric_limits<uint32_t>::max());
}