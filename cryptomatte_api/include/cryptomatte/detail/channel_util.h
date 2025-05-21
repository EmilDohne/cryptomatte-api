#pragma once

#include "macros.h"
#include "string_util.h"

#include <string_view>
#include <string>
#include <format>

namespace NAMESPACE_CRYPTOMATTE_API
{

	namespace detail
	{

		/// \brief Enum representing supported channel types for cryptomatte layers.
		enum class channel_type
		{
			red		= 0,
			green	= 1,
			blue	= 2,
			alpha	= 3
		};


		/// \brief Custom operator< overload for channel_type, using std::underlying_type_t to compare the values.
		/// \param left The left-hand channel_type to compare.
		/// \param right The right-hand channel_type to compare.
		/// \return True if left < right based on underlying integer values; otherwise false.
		inline bool operator<(channel_type left, channel_type right) 
		{
			return static_cast<std::underlying_type_t<channel_type>>(left) < 
				static_cast<std::underlying_type_t<channel_type>>(right);
		}


		/// \brief Map the channel extension into a channel type, supporting r, g, b, and a channels.
		/// \param channel_extension The extension to parse. Can be variants like 'r', 'R', 'red', 'G', 'alpha', etc.
		/// \throws std::runtime_error if the channel extension cannot be mapped.
		/// \return The mapped channel_type.
		channel_type map_to_channel_type(std::string channel_extension);


		/// \brief Structure representing a parsed channel name and its metadata.
		struct channel_repr
		{
			std::string _typename; ///< The base typename extracted from the channel string.
			int index;             ///< The numerical index of the layer.
			channel_type type;     ///< The type of channel (r, g, b, a).

			/// Deserialize the channel representation from a channel name in the format 
			/// 
			/// {typename}00.r|R|red etc.
			/// 
			/// extracting the individual components into the resulting `channel_repr` struct.
			/// 
			/// \throws std::runtime_error if the passed string is not already in the required format.
			/// 
			/// \returns the channel representation of this incoming string
			channel_repr(std::string s);

			/// \brief Get the original channel name this instance was instantiated with.
			/// \return The original channel string passed to the constructor.
			std::string channel_name() const;

			/// \brief Compare two channel_repr instances for sorting.
			///
			/// First compares by index, then by channel type. Useful for sorting to maintain correct cryptomatte
			/// ordering, ensuring rank and coverage channel pairs are adjacent.
			///
			/// \param other The channel_repr to compare with.
			/// \return True if this object should come before \p other; otherwise false.
			bool operator<(const channel_repr& other) const;

		private:
			std::string m_OriginalChannelName; ///< Internal storage of the original channel name.
		};
		

		/// \brief Sort and validate cryptomatte channel names.
		///
		/// Parses, validates, and sorts a list of cryptomatte channel names to ensure they are well-formed
		/// and ordered correctly. Each input string must follow the format `{typename}NN.channel`, where:
		/// 
		/// - `{typename}` is an arbitrary base name,
		/// - `NN` is a two-digit index,
		/// - and `channel` is one of the supported channel identifiers: `r`, `g`, `b`, or `a`
		///   (case-insensitive and alias-friendly, e.g., `red`, `alpha`, etc.).
		///
		/// The function performs the following steps:
		/// 1. Parses each channel string into a `channel_repr` structure.
		/// 2. Validates that the channel format is correct and mappable.
		/// 3. Sorts the channels by index first, then by channel type (`r`, `g`, `b`, `a`).
		///
		/// This ensures a stable and correct channel order, e.g.:
		/// ```
		/// {typename}00.r
		/// {typename}00.g
		/// {typename}00.b
		/// {typename}00.a
		/// {typename}01.r
		/// ...
		/// ```
		///
		/// \param input_channels Vector of cryptomatte channel strings to be validated and sorted.
		/// \return A sorted and validated vector of channel names.
		/// \throws std::runtime_error If any channel string is malformed or cannot be parsed.
		std::vector<std::string> sort_and_validate_channels(const std::vector<std::string>& input_channels);


	} // detail


} // NAMESPACE_CRYPTOMATTE_API