#pragma once

#include "macros.h"

#include <OpenImageIO/imageio.h>


namespace NAMESPACE_CRYPTOMATTE_API
{

	namespace detail
	{

		/// \brief Find any channels that do not match the given `compare` type
		///
		/// \param spec The image spec to check against
		/// \param channel_indices The channel indices for which to check if they have the right type
		/// \param compare The type to check that all given channels have.
		/// 
		/// \returns A vector of the subset of `channel_indices` which do not match `compare`
		std::vector<int> find_mismatched_channels(
			const OIIO::ImageSpec& spec,
			std::vector<int> channel_indices,
			OIIO::TypeDesc compare
		);

		/// \brief Find any channels that do not match the given `compare` type
		///
		/// \param spec The image spec to check against
		/// \param channel_names The channel names for which to check if they have the right type
		/// \param compare The type to check that all given channels have.
		/// 
		/// \returns A vector of the subset of `channel_names` which do not match `compare`
		std::vector<std::string> find_mismatched_channels(
			const OIIO::ImageSpec& spec,
			std::vector<std::string> channel_names,
			OIIO::TypeDesc compare
		);

	} // detail


} // NAMESPACE_CRYPTOMATTE_API