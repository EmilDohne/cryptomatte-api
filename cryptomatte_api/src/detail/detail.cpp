#include "detail/detail.h"

#include <format>
#include <cstdint>
#include <string>

namespace NAMESPACE_CRYPTOMATTE_API
{

	namespace detail
	{

		// -----------------------------------------------------------------------------------
		// -----------------------------------------------------------------------------------
		uint32_t hex_str_to_uint32_t(const std::string_view hex)
		{
			if (hex.size() != 8u)
			{
				throw std::invalid_argument(
					std::format(
						"Invalid hex string '{}' encountered, expected exactly 8 characters to form a valid 32-bit number but instead got {} chars",
						hex, hex.size()
					)
				);
			}

			// decode using base-16
			unsigned long long res = 0;
			try
			{
				res = std::stoull(std::string(hex), nullptr, 16);
			}
			catch (const std::exception& e)
			{
				throw std::runtime_error(
					std::format(
						"Invalid hex string '{}' encountered: {}",
						hex, e.what()
					)
				);
			}
			if (res > std::numeric_limits<uint32_t>::max())
			{
				throw std::runtime_error(
					std::format(
						"Unable to convert hex string '{}' into a uint32_t as the resulting value would exceed the number of values we can store in a uint32_t.",
						hex
					)
				);
			}

			return static_cast<uint32_t>(res);
		}

		// -----------------------------------------------------------------------------------
		// -----------------------------------------------------------------------------------
		std::string uint32_t_to_hex_str(uint32_t value)
		{
			return std::format("{:08x}", value);
		}

	} // detail

} // NAMESPACE_CRYPTOMATTE_API