#include "detail/oiio_util.h"


namespace NAMESPACE_CRYPTOMATTE_API
{

	namespace detail
	{

		// -----------------------------------------------------------------------------------
		// -----------------------------------------------------------------------------------
		std::vector<int> find_mismatched_channels(
			const OIIO::ImageSpec& spec, 
			std::vector<int> channel_indices, 
			OIIO::TypeDesc compare
		)
		{
			// If the whole file has a single format, we just check if that matches with the given compare
			if (spec.channelformats.size() == 0)
			{
				if (spec.format == compare)
				{
					return {};
				}
				return channel_indices;
			}

			// Otherwise, check which indices are mismatched (if any)
			std::vector<int> mismatched_indices;
			for (auto idx : channel_indices)
			{
				assert(idx > 0);

				OIIO::TypeDesc format = spec.channelformats.at(static_cast<size_t>(idx));
				if (format != compare)
				{
					mismatched_indices.push_back(idx);
				}
			}

			return mismatched_indices;
		}

		std::vector<std::string> find_mismatched_channels(
			const OIIO::ImageSpec& spec, 
			std::vector<std::string> channel_names, 
			OIIO::TypeDesc compare
		)
		{
			// If the whole file has a single format, we just check if that matches with the given compare
			if (spec.channelformats.size() == 0)
			{
				if (spec.format == compare)
				{
					return {};
				}
				return channel_names;
			}

			// Otherwise, check which indices are mismatched (if any)
			std::vector<std::string> mismatched_names;
			for (auto& name : channel_names)
			{

				OIIO::TypeDesc format = spec.channelformat(spec.channelindex(name));
				if (format != compare)
				{
					mismatched_names.push_back(name);
				}
			}

			return mismatched_names;
		}

	} // namespace detail

} // namespace NAMESPACE_CRYPTOMATTE_API
