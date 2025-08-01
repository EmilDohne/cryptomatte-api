#pragma once

#include "macros.h"

#include <string_view>
#include <cstdint>
#include <string>

namespace NAMESPACE_CRYPTOMATTE_API
{

	namespace detail
	{

		/// Utility function for converting a hex string into a 32-bit
		/// unsigned value. 
		/// 
		/// \param hex The hex string to decode, must be a valid hex string as well as being 8 chars long
		/// 
		/// \throws std::invalid_argument if `hex` is not exactly 8 chars
		/// \throws std::runtime_error if the hex string would decode to a number greater than the numeric
		///							   limit of a uint32_t
		/// 
		/// \returns the decoded uint32_t
		uint32_t hex_str_to_uint32_t(const std::string_view hex);

		std::string uint32_t_to_hex_str(uint32_t value);

	} // detail

} // NAMESPACE_CRYPTOMATTE_API