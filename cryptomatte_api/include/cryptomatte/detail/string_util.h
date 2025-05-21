#pragma once

#include "macros.h"

#include <string_view>
#include <string>
#include <vector>
#include <span>
#include <ranges>
#include <algorithm>

namespace NAMESPACE_CRYPTOMATTE_API
{

	namespace str
	{
		/// Split the string `s` based on the `delimiter`, returning a vector of all the 
		/// components post-split
		/// 
		/// \tparam max_elems A compile-time limit to how many iterations the split should go through before aborting
		///					  and appending the rest of the string to the end of the vector. Precaution in order to avoid
		///					  infinite loops. Defaults to 16384 split elements.
		template <size_t max_elems = 16384>
		std::vector<std::string> split(std::string s, std::string delimiter) 
		{
			size_t pos_start = 0;
			size_t pos_end = 0;
			size_t delim_len = delimiter.length();

			std::vector<std::string> res;

			size_t count = 0;
			while (
				(pos_end = s.find(delimiter, pos_start)) != std::string::npos &&
				count < max_elems
				) 
			{
				auto token = s.substr(pos_start, pos_end - pos_start);
				pos_start = pos_end + delim_len;
				res.push_back(token);
				++count;
			}

			res.push_back(s.substr(pos_start));
			return res;
		}


		inline std::string join(std::span<std::string> in, std::string delimiter)
		{
			if (in.empty())
			{
				return "";
			}

			std::string result;

			result += in[0];
			for (auto idx : std::views::iota(size_t{ 1 }, in.size()))
			{
				result += delimiter;
				result += in[idx];
			}

			return result;
		}


		/// Strips the string `strip` from the string `s` repeatedly until `s` no longer
		/// starts with this value. This is upper-bounded by the template parameter `max_strips` to avoid 
		/// infinite loops. This defaults to 16384 strip iterations.
		template <size_t max_strips = 16384>
		inline std::string lstrip(std::string s, std::string strip) noexcept
		{
			size_t start_pos = 0;
			size_t _count = 0;

			// Continuously advance the start_pos until the string no longer starts with it.
			// Doing this with ranges would likely be faster than this method as std::basic_string::substr
			// returns a new string but for our use cases this is likely irrelevant.
			while (s.substr(start_pos).starts_with(strip) && _count < max_strips)
			{
				start_pos += strip.size();
			}

			// std::basic_string::substr() may throw if we index past the end, here instead we just return an empty
			// string.
			if (start_pos > s.size())
			{
				return "";
			}

			return s.substr(start_pos);
		}
		

		/// Transforms the string s into its casefold representation, returning a copy of it.
		inline std::string casefold(const std::string& s)
		{
			std::string result = s;
			std::transform(s.begin(), s.end(), result.begin(), [](unsigned char c) { return std::tolower(c); });
			return result;
		}


	} // str

} // NAMESPACE_CRYPTOMATTE_API