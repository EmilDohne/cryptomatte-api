#include "cryptomatte.h"

#include <cassert>
#include <ranges>
#include <algorithm>

#include "metadata.h"
#include "detail/channel_util.h"

#include <compressed/image.h>
#include <compressed/blosc2/lazyschunk.h>

namespace NAMESPACE_CRYPTOMATTE_API
{


	// -----------------------------------------------------------------------------------
	// -----------------------------------------------------------------------------------
	cryptomatte::cryptomatte(
		std::unordered_map<std::string, compressed::channel<float32_t>> channels, 
		const NAMESPACE_CRYPTOMATTE_API::metadata& metadata
	)
	{
		if (channels.empty())
		{
			throw std::invalid_argument("Unable to construct cryptomatte with empty channel list");
		}

		m_Metadata = metadata;

		std::vector<std::string> cryptomatte_channels;
		std::vector<std::string> legacy_channels;

		for (const auto& [name, _] : channels)
		{
			if (m_Metadata.is_valid_channel_name(name))
			{
				cryptomatte_channels.push_back(name);
			}
			else if (m_Metadata.is_valid_legacy_channel_name(name))
			{
				legacy_channels.push_back(name);
			}
		}

		// Make sure all of these are the same, this is required for further processing.
		size_t num_chunks = channels.begin()->second.num_chunks();
		size_t chunk_size = channels.begin()->second.chunk_size();
		size_t uncompressed_sz = channels.begin()->second.uncompressed_size();
		size_t width = channels.begin()->second.width();
		size_t height = channels.begin()->second.height();

		// Validate these and ensure they are ordered for later access.
		cryptomatte_channels = detail::sort_and_validate_channels(cryptomatte_channels);
		for (const auto& name : cryptomatte_channels)
		{
			const auto& channel = channels.at(name);
			if (channel.num_chunks() != num_chunks)
			{
				throw std::invalid_argument(
					std::format(
						"Invalid channel '{}' provided to cryptomatte constructor. All channes are required to have"
						" been encoded with the same number of chunks. This is expected to be {} but instead got {}",
						name, num_chunks, channels.at(name).num_chunks()
					)
				);
			}
			if (channel.chunk_size() != chunk_size)
			{
				throw std::invalid_argument(
					std::format(
						"Invalid channel '{}' provided to cryptomatte constructor. "
						"All channels must have the same chunk size: expected {}, got {}.",
						name, chunk_size, channel.chunk_size()
					)
				);
			}
			if (channel.uncompressed_size() != uncompressed_sz)
			{
				throw std::invalid_argument(
					std::format(
						"Invalid channel '{}' provided to cryptomatte constructor. "
						"All channels must have the same uncompressed size: expected {}, got {}.",
						name, uncompressed_sz, channel.uncompressed_size()
					)
				);
			}

			if (channel.width() != width || channel.height() != height)
			{
				throw std::invalid_argument(
					std::format(
						"Invalid channel '{}' provided to cryptomatte constructor. "
						"All channels must have the same resolution: expected {}x{}, got {}x{}.",
						name, width, height, channel.width(), channel.height()
					)
				);
			}


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
		if (channels.empty())
		{
			throw std::invalid_argument("Unable to construct cryptomatte with empty channel list");
		}

		m_Metadata = metadata;

		std::vector<std::string> cryptomatte_channels;
		std::vector<std::string> legacy_channels;

		for (const auto& [name, _] : channels)
		{
			if (m_Metadata.is_valid_channel_name(name))
			{
				cryptomatte_channels.push_back(name);
			}
			else if (m_Metadata.is_valid_legacy_channel_name(name))
			{
				legacy_channels.push_back(name);
			}
		}


		size_t vec_size = channels.begin()->second.size();

		// Validate these and ensure they are ordered for later access.
		cryptomatte_channels = detail::sort_and_validate_channels(cryptomatte_channels);
		for (const auto& name : cryptomatte_channels)
		{
			auto& channel_vec = channels.at(name);
			if (channel_vec.size() != vec_size)
			{
				throw std::invalid_argument(
					std::format(
						"Invalid channel '{}' provided to cryptomatte constructor. "
						"All channels must have the same chunk size: expected {}, got {}.",
						name, vec_size, channel_vec.size()
					)
				);
			}

			auto span = std::span<const float32_t>(channel_vec);
			// Will throw if the vec size is not that of width * height
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
		auto metadatas = NAMESPACE_CRYPTOMATTE_API::metadata::from_spec(input_ptr->spec(), file);
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

		return out;
	}

	// -----------------------------------------------------------------------------------
	// -----------------------------------------------------------------------------------
	size_t cryptomatte::width() const
	{
		if (!m_Channels.empty())
		{
			return m_Channels.begin()->second.width();
		}
		return {};
	}

	// -----------------------------------------------------------------------------------
	// -----------------------------------------------------------------------------------
	size_t cryptomatte::height() const
	{
		if (!m_Channels.empty())
		{
			return m_Channels.begin()->second.height();
		}
		return {};
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
	std::vector<float32_t> cryptomatte::mask(std::string name) const
	{
		if (!m_Metadata.manifest())
		{
			throw std::invalid_argument("Cannot use string overload of 'cryptomatte::mask' when there is no manifest present");
		}

		if (!m_Metadata.manifest().value().contains(name))
		{
			throw std::invalid_argument(
				std::format(
					"Unable to get mask '{}' using cryptomatte::mask as it does not exist on the manifest.",
					name
				)
			);
		}

		// Defer to the hash-based implementation.
		return this->mask(m_Metadata.manifest().value().hash(name));
	}

	// -----------------------------------------------------------------------------------
	// -----------------------------------------------------------------------------------
	std::vector<float32_t> cryptomatte::mask(uint32_t hash) const
	{
		std::vector<float32_t> out(this->width() * this->height());
		// Get the hash as float32_t, this way we don't have to do this in the hot loop.
		float32_t hash_val = std::bit_cast<float32_t>(hash);

		// Iterate rank and coverage channels together
		for (size_t i = 0; i + 1 < m_Channels.size(); i += 2)
		{
			// Our ctor performs validation that all of these are identical for purposes of iteration.
			const auto& rank_channel = m_Channels[i].second;
			const auto& covr_channel = m_Channels[i + 1].second;

			size_t chunk_size_elems = rank_channel.chunk_size() / sizeof(float32_t);

			std::vector<float32_t> rank_chunk(chunk_size_elems);
			std::vector<float32_t> covr_chunk(chunk_size_elems);

			// Iterate the chunks, decompressing on the fly
			for (size_t chunk_idx : std::views::iota(size_t{ 0 }, rank_channel.num_chunks()))
			{
				size_t base_idx = chunk_size_elems * chunk_idx;
				rank_channel.get_chunk(std::span<float32_t>(rank_chunk), chunk_idx);
				covr_channel.get_chunk(std::span<float32_t>(covr_chunk), chunk_idx);

				// Since the last chunk may hold less than `chunk_size` elements, we must account for this and ensure
				// we are only at most going to the end of the `out` vector.
				size_t num_elements = std::min<size_t>(out.size() - base_idx, chunk_size_elems);

				// Accumulate the output pixel from all of the coverage channels.
				auto pixel_iota = std::views::iota(size_t{ 0 }, num_elements);
				std::for_each(std::execution::par_unseq, pixel_iota.begin(), pixel_iota.end(), [&](size_t idx)
					{
						if (rank_chunk[idx] == hash_val)
						{
							out[base_idx + idx] += covr_chunk[idx];
						}
					});
			}
		}

		return out;
	}


	// -----------------------------------------------------------------------------------
	// -----------------------------------------------------------------------------------
	compressed::channel<float32_t> cryptomatte::mask_compressed(std::string name) const
	{
		if (!m_Metadata.manifest())
		{
			throw std::invalid_argument("Cannot use string overload of 'cryptomatte::mask' when there is no manifest present");
		}

		if (!m_Metadata.manifest().value().contains(name))
		{
			throw std::invalid_argument(
				std::format(
					"Unable to get mask '{}' using cryptomatte::mask as it does not exist on the manifest.",
					name
				)
			);
		}

		// Defer to the hash-based implementation.
		return this->mask_compressed(m_Metadata.manifest().value().hash(name));
	}

	// -----------------------------------------------------------------------------------
	// -----------------------------------------------------------------------------------
	compressed::channel<float32_t> cryptomatte::mask_compressed(uint32_t hash) const
	{
		const auto& first_channel = this->m_Channels.begin()->second;

		// Generate a lazy channel that we will use to fill, using a lazy channel allows us to 
		// not pay any memory allocation cost beyond a single chunk which will be reused. 
		// Ensure we use the same parameters as our channels as the ctor taking compressed::channel instances
		// and thus have non-standard block and chunk sizes.
		auto out = compressed::channel<float32_t>::zeros(
			this->width(), 
			this->height(),
			first_channel.compression(),
			static_cast<uint8_t>(first_channel.compression_level()),
			first_channel.block_size(),
			first_channel.chunk_size()
		);
		// Use a vector that doesn't default initialize, as compressed::channel::get_chunk will internally use
		// std::fill on the buffer.
		compressed::util::default_init_vector<float32_t> chunk_buffer(out.chunk_size());

		assert(out.chunk_size() == first_channel.chunk_size());
		assert(out.num_chunks() == first_channel.num_chunks());

		// Get the hash as float32_t, this way we don't have to do this in the hot loop.
		float32_t hash_val = std::bit_cast<float32_t>(hash);

		// Iterate rank and coverage channels together
		for (size_t i = 0; i + 1 < m_Channels.size(); i += 2)
		{
			const auto& rank_channel = m_Channels[i].second;
			const auto& covr_channel = m_Channels[i + 1].second;

			std::vector<float32_t> rank_chunk(rank_channel.chunk_size() / sizeof(float32_t));
			std::vector<float32_t> covr_chunk(covr_channel.chunk_size() / sizeof(float32_t));

			// Iterate the chunks, decompressing on the fly
			for (size_t chunk_idx : std::views::iota(size_t{ 0 }, rank_channel.num_chunks()))
			{
				// Will std::fill into the chunk_buffer (since its a lazy schunk).
				out.get_chunk(std::span<float32_t>(chunk_buffer), chunk_idx);

				rank_channel.get_chunk(std::span<float32_t>(rank_chunk), chunk_idx);
				covr_channel.get_chunk(std::span<float32_t>(covr_chunk), chunk_idx);

				// Accumulate the output pixel from all of the coverage channels.
				auto pixel_iota = std::views::iota(size_t{ 0 }, rank_chunk.size());
				std::for_each(std::execution::par_unseq, pixel_iota.begin(), pixel_iota.end(), [&](size_t idx)
					{
						if (rank_chunk[idx] == hash_val)
						{
							chunk_buffer[idx] += covr_chunk[idx];
						}
					});

				// Set the chunk again (will recompress).
				out.set_chunk(std::span<float32_t>(chunk_buffer), chunk_idx);
			}
		}

		return std::move(out);
	}

	// -----------------------------------------------------------------------------------
	// -----------------------------------------------------------------------------------
	size_t cryptomatte::num_levels() const noexcept
	{
		return m_Channels.size() / 2;
	}

	// -----------------------------------------------------------------------------------
	// -----------------------------------------------------------------------------------
	NAMESPACE_CRYPTOMATTE_API::metadata& cryptomatte::metadata()
	{
		return m_Metadata;
	}

	// -----------------------------------------------------------------------------------
	// -----------------------------------------------------------------------------------
	const NAMESPACE_CRYPTOMATTE_API::metadata& cryptomatte::metadata() const
	{
		return m_Metadata;
	}

} // NAMESPACE_CRYPTOMATTE_API