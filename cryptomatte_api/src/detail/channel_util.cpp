#include "detail/channel_util.h"

#include <algorithm>
#include <set>
#include <vector>
#include <string>
#include <string_view>
#include <map>

namespace NAMESPACE_CRYPTOMATTE_API
{

	namespace detail
	{

		namespace
		{
			/// \brief Utility function to split a string on the last dot
			/// \param input The input string
			/// \return A pair of the first component and the second component split on the dot, does not return the dot itself.
			///			If the string does not have a dot, the dot is at the start or at the end we return the input
			///			as the first element in the pair and an empty "" as the second element.
			inline std::pair<std::string, std::string> split_on_last_dot(std::string_view input) noexcept
			{
				auto pos = input.rfind('.');
				if (pos == std::string_view::npos || pos == 0 || pos == input.size() - 1)
				{
					// No dot, dot at start, or dot at end — treat as no extension
					return { std::string(input), "" };
				}

				return {
					std::string(input.substr(0, pos)),
					std::string(input.substr(pos + 1))
				};
			}

		} // private namespace


		// -----------------------------------------------------------------------------------
		// -----------------------------------------------------------------------------------
		channel_type map_to_channel_type(std::string channel_extension)
		{
			if (channel_extension == "R" || channel_extension == "r" || channel_extension == "red")
			{
				return channel_type::red;
			}
			else if (channel_extension == "G" || channel_extension == "g" || channel_extension == "green")
			{
				return channel_type::green;
			}
			else if (channel_extension == "B" || channel_extension == "b" || channel_extension == "blue")
			{
				return channel_type::blue;
			}
			else if (channel_extension == "A" || channel_extension == "a" || channel_extension == "alpha")
			{
				return channel_type::alpha;
			}

			throw std::runtime_error(
				std::format(
					"Unable to identify a channel type from the incoming string {}, must be"
					" r|R|red, g|G|green, b|B|blue or a|A|alpha", channel_extension
				)
			);
		}


		// -----------------------------------------------------------------------------------
		// -----------------------------------------------------------------------------------
		channel_repr::channel_repr(std::string s)
		{
			auto [name, extension] = split_on_last_dot(s);
			if (extension == "")
			{
				throw std::runtime_error(
					std::format(
						"Unable to decode channel representation from string '{}'. Expected the following format: "
						"{{typename}}00.r", s
					)
				);
			}

			// The name must have at least two digits for the index.
			if (name.size() < 2)
			{
				throw std::runtime_error(
					std::format(
						"Unable to decode channel representation from string '{}' as there is not enough characters"
						" to decode two numbers for the index. Expected the following format: {{typename}}00.r", s
					)
				);
			}

			auto type_name = name.substr(0, name.size() - 2);
			auto index_str = name.substr(name.size() - 2);

			// Ensure index_str is numeric
			if (!std::isdigit(index_str[0]) || !std::isdigit(index_str[1]))
			{
				throw std::runtime_error(
					std::format(
						"Invalid index digits '{}' in string '{}'. Expected a 2-digit number before the extension.",
						index_str, s
					)
				);
			}
			int idx = std::stoi(index_str);

			this->_typename = type_name;
			this->index = idx;
			this->type = map_to_channel_type(extension);
			this->m_OriginalChannelName = s;
		}


		// -----------------------------------------------------------------------------------
		// -----------------------------------------------------------------------------------
		std::string channel_repr::channel_name() const
		{
			return m_OriginalChannelName;
		}


		// -----------------------------------------------------------------------------------
		// -----------------------------------------------------------------------------------
		bool channel_repr::operator<(const channel_repr& other) const
		{
			// If the index mismatches, sort by that first.
			if (this->index != other.index)
			{
				return this->index < other.index;
			}

			// Then sort by the channel type, uses our custom operator< overload for `channel_type`
			return this->type < other.type;
		}


		// -----------------------------------------------------------------------------------
		// -----------------------------------------------------------------------------------
		std::vector<std::string> sort_and_validate_channels(const std::vector<std::string>& input_channels)
		{
			if (input_channels.size() % 2 != 0)
			{
				throw std::runtime_error("Channel count must be divisible by 2.");
			}

			std::vector<channel_repr> as_repr;
			as_repr.reserve(input_channels.size());

			for (const auto& chname : input_channels)
			{
				// Uses channel_repr(std::string), will throw if format is invalid
				as_repr.emplace_back(chname); 
			}

			// Sort using the overloaded operator<
			std::sort(as_repr.begin(), as_repr.end());

			// Perform validation, throwing std::runtime_error on invalid sequences.
			{
				// Group channels by index
				std::map<int, std::set<channel_type>> index_to_channels;
				for (const auto& repr : as_repr)
				{
					index_to_channels[repr.index].insert(repr.type);
				}

				// Validate index continuity and full channel sets except for the last index
				if (index_to_channels.empty())
				{
					throw std::runtime_error("No valid channels provided.");
				}

				int expected_index = 0;
				int max_index = index_to_channels.rbegin()->first;

				for (const auto& [index, channels] : index_to_channels)
				{
					if (index != expected_index)
					{
						throw std::runtime_error(
							std::format(
								"Missing cryptomatte index: expected {}, got {}", expected_index, index
							)
						);
					}

					// check if all 4 channel types are present, or alternatively if this is the last set of cryptomatte
					// channels we allow 2 (the cryptomatte specification is not clear on whether this is allowed or not so
					// we assume it is).
					if (index != max_index && channels.size() != 4)
					{
						throw std::runtime_error(
							std::format(
								"Incomplete channel set at index {} — expected 4 channels, got {}", index, channels.size()
							)
						);
					}
					else if (index == max_index && channels.size() != 4 && channels.size() != 2)
					{
						throw std::runtime_error(
							std::format(
								"Incomplete channel set at index {} — expected 2 or 4 channels, got {}", index, channels.size()
							)
						);
					}

					++expected_index;
				}
			}


			// Rebuild the sorted channel names
			std::vector<std::string> sorted_channels;
			sorted_channels.reserve(as_repr.size());

			for (const auto& repr : as_repr)
			{
				sorted_channels.push_back(repr.channel_name());
			}

			return sorted_channels;
		}

	} // detail

} // NAMESPACE_CRYPTOMATTE_API