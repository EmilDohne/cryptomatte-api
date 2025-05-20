#include "cryptomatte.h"

#include "metadata.h"
#include "detail/channel_util.h"

#include <compressed/image.h>

namespace NAMESPACE_CRYPTOMATTE_API
{


	// -----------------------------------------------------------------------------------
	// -----------------------------------------------------------------------------------
	cryptomatte::cryptomatte(
		std::unordered_map<std::string, compressed::channel<float32_t>> channels, 
		const NAMESPACE_CRYPTOMATTE_API::metadata& metadata
	)
	{
		m_Metadata = metadata;

		std::vector<std::string> cryptomatte_channels;
		std::vector<std::string> legacy_channels;

		for (const auto& [name, _] : channels)
		{
			if (m_Metadata.is_valid(name))
			{
				cryptomatte_channels.push_back(name);
			}
			else if (m_Metadata.is_valid_legacy(name))
			{
				legacy_channels.push_back(name);
			}
		}

		// Validate these and ensure they are ordered for later access.
		cryptomatte_channels = detail::sort_and_validate_channels(cryptomatte_channels);
		for (const auto& name : cryptomatte_channels)
		{
			m_Channels.emplace_back(name, std::move(channels.at(name)));
		}
		// Push them back in any order, doesn't matter.
		for (const auto& name : legacy_channels)
		{
			m_LegacyChannels[name] = std::move(channels.at(name));
		}
	}

	
	// -----------------------------------------------------------------------------------
	// -----------------------------------------------------------------------------------
	cryptomatte::cryptomatte(
		std::unordered_map<std::string, std::vector<float32_t>> channels,
		size_t width,
		size_t height,
		const NAMESPACE_CRYPTOMATTE_API::metadata& metadata
	)
	{
		m_Metadata = metadata;

		std::vector<std::string> cryptomatte_channels;
		std::vector<std::string> legacy_channels;

		for (const auto& [name, _] : channels)
		{
			if (m_Metadata.is_valid(name))
			{
				cryptomatte_channels.push_back(name);
			}
			else if (m_Metadata.is_valid_legacy(name))
			{
				legacy_channels.push_back(name);
			}
		}

		// Validate these and ensure they are ordered for later access.
		cryptomatte_channels = detail::sort_and_validate_channels(cryptomatte_channels);
		for (const auto& name : cryptomatte_channels)
		{
			auto& channel_vec = channels.at(name);
			auto span = std::span<const float32_t>(channel_vec.begin(), channel_vec.end());
			auto channel = compressed::channel<float32_t>(span, width, height);
			m_Channels.emplace_back(name, std::move(channel));
		}
		// Push them back in any order, doesn't matter.
		for (const auto& name : legacy_channels)
		{
			auto& channel_vec = channels.at(name);
			auto span = std::span<const float32_t>(channel_vec.begin(), channel_vec.end());
			m_LegacyChannels[name] = compressed::channel<float32_t>(span, width, height);
		}
	}

	// -----------------------------------------------------------------------------------
	// -----------------------------------------------------------------------------------
	std::vector<cryptomatte> cryptomatte::load(std::filesystem::path file, bool load_preview)
	{
		// Load the OIIO image, mostly for reading the spec. compressed::image will take 
		// care of loading the pixels.
		auto input_ptr = OIIO::ImageInput::open(file.string());
		if (!input_ptr)
		{
			throw std::invalid_argument(
				std::format(
					"cryptomatte: Invalid filepath provided, unable to open file {}",
					file.string()
				)
			);
		}

		// Retrieve the metadatas associated with this image
		auto metadatas = NAMESPACE_CRYPTOMATTE_API::metadata::from_spec(input_ptr->spec());
		// Short-circuit if not metadata was found -> no cryptomatte.
		if (metadatas.size() == 0)
		{
			return {};
		}

		std::vector<cryptomatte> out;
		out.reserve(metadatas.size());

		// Store the channelnames per metadata instance as well as all of the channels to load.
		std::vector<std::vector<std::string>> channel_names;
		std::vector<std::string> all_channel_names;

		// Get all of the channel names and store them
		for (const auto& meta : metadatas)
		{
			auto channelnames = meta.channel_names(input_ptr->spec().channelnames);
			if (load_preview)
			{
				auto preview_channelnames = meta.legacy_channel_names(input_ptr->spec().channelnames);
				channelnames.insert(channelnames.end(), preview_channelnames.begin(), preview_channelnames.end());
			}

			channel_names.push_back(channelnames);
			all_channel_names.insert(all_channel_names.end(), channelnames.begin(), channelnames.end());
		}

		// Load all the channels in one go, we split these up later.
		auto image = compressed::image<float32_t>::read(std::move(input_ptr), all_channel_names);

		// Split up the cryptomattes and load them into their own instances.
		size_t idx = 0;
		for (const auto& chnames : channel_names)
		{
			std::unordered_map<std::string, compressed::channel<float32_t>> channels;
			for (const auto& chname : chnames)
			{
				channels[chname] = image.extract_channel(chname);
			}

			out.push_back(cryptomatte(std::move(channels), metadatas[idx]));
			++idx;
		}

		return std::move(out);
	}

	// -----------------------------------------------------------------------------------
	// -----------------------------------------------------------------------------------
	bool cryptomatte::has_preview() const
	{
		return m_LegacyChannels.size() > 0;
	}

	// -----------------------------------------------------------------------------------
	// -----------------------------------------------------------------------------------
	std::vector<std::vector<float32_t>> cryptomatte::preview() const
	{
		std::vector<std::vector<float32_t>> out;

		for (const auto& [name, channel] : m_LegacyChannels)
		{
			out.push_back(channel.get_decompressed());
		}

		return out;
	}

	// -----------------------------------------------------------------------------------
	// -----------------------------------------------------------------------------------
	std::unordered_map<std::string, compressed::channel<float32_t>> cryptomatte::extract_compressed()
	{
		return std::move(m_LegacyChannels);
	}

	// -----------------------------------------------------------------------------------
	// -----------------------------------------------------------------------------------
	size_t cryptomatte::num_levels() const noexcept
	{
		return m_Channels.size() / 2;
	}

	// -----------------------------------------------------------------------------------
	// -----------------------------------------------------------------------------------
	NAMESPACE_CRYPTOMATTE_API::metadata cryptomatte::metadata()
	{
		return m_Metadata;
	}

} // NAMESPACE_CRYPTOMATTE_API