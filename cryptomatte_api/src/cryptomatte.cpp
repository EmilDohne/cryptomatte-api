#include "cryptomatte.h"

#include <cassert>
#include <ranges>
#include <algorithm>
#include <unordered_map>
#include <unordered_set>

#include "metadata.h"
#include "detail/channel_util.h"
#include "detail/oiio_util.h"
#include "detail/decoding_impl.h"
#include "detail/detail.h"
#include "detail/scoped_timer.h"

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

		// Sort metadatas alphabetically, this is because OIIO appears to internally have different ordering of 
		// metadata depending on the OS/Compiler. 
		std::sort(metadatas.begin(), metadatas.end(), [](const auto& a, const auto& b) 
		{
			return a.name() < b.name();
		});

		std::vector<cryptomatte> out;
		out.reserve(metadatas.size());

		// Store the channelnames per metadata instance as well as all of the channels to load.
		std::vector<std::vector<std::string>> channel_names;
		std::vector<std::string> all_channel_names;

		// Get all of the channel names and store them
		for (const auto& meta : metadatas)
		{
			auto channelnames = meta.channel_names(input_ptr->spec().channelnames);

			// Check if the channels are not 32-bit (only the data channels, the preview channels can and do 
			// differ, often being encoded as 16-bit halfs)
			auto differing_channels = detail::find_mismatched_channels(
				input_ptr->spec(), channelnames, OIIO::TypeDesc::FLOAT
			);
			if (differing_channels.size() > 0)
			{
				throw std::runtime_error(
					std::format(
					"Cryptomatte specification requires all data channels to be 32-bit float.\n"
					"The following channels do not match this requirement:\n  {}",
					str::join(differing_channels, ", ")
					)
				);
			}

			if (load_preview)
			{
				auto preview_channelnames = meta.legacy_channel_names(input_ptr->spec().channelnames);
				channelnames.insert(channelnames.end(), preview_channelnames.begin(), preview_channelnames.end());
			}

			channel_names.push_back(channelnames);
			all_channel_names.insert(all_channel_names.end(), channelnames.begin(), channelnames.end());
		}

		// Load all the channels in one go, we split these up later. Lower the chunk size so we don't over-allocate
		// for small images.
		const auto& spec = input_ptr->spec();
		auto chunk_size = std::min(compressed::s_default_chunksize, static_cast<size_t>(spec.width) * spec.height * sizeof(float32_t));
		auto block_size = std::min(chunk_size / 128, compressed::s_default_blocksize);
		auto image = compressed::image<float32_t>::read(
			std::move(input_ptr), 
			all_channel_names, 
			0, 
			compressed::enums::codec::lz4, 
			9, 
			block_size, 
			chunk_size
		);

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
	std::unordered_map<std::string, compressed::channel<float32_t>> cryptomatte::extract_preview_compressed()
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
				size_t chunk_num_elems = rank_channel.chunk_size(chunk_idx) / sizeof(float32_t);

				// Will std::fill into the chunk_buffer (since its a lazy schunk).
				out.get_chunk(std::span<float32_t>(chunk_buffer.data(), chunk_num_elems), chunk_idx);

				rank_channel.get_chunk(std::span<float32_t>(rank_chunk.data(), chunk_num_elems), chunk_idx);
				covr_channel.get_chunk(std::span<float32_t>(covr_chunk.data(), chunk_num_elems), chunk_idx);

				// Accumulate the output pixel from all of the coverage channels.
				auto pixel_iota = std::views::iota(size_t{ 0 }, chunk_num_elems);
				std::for_each(std::execution::par_unseq, pixel_iota.begin(), pixel_iota.end(), [&](size_t idx)
					{
						if (rank_chunk[idx] == hash_val)
						{
							chunk_buffer[idx] += covr_chunk[idx];
						}
					});

				// Set the chunk again (will recompress).
				out.set_chunk(std::span<float32_t>(chunk_buffer.data(), chunk_num_elems), chunk_idx);
			}
		}

		return out;
	}


	// -----------------------------------------------------------------------------------
	// -----------------------------------------------------------------------------------
	std::unordered_map<std::string, std::vector<float32_t>> cryptomatte::masks(std::vector<std::string> names) const
	{
		if (!m_Metadata.manifest().has_value())
		{
			throw std::invalid_argument(
				"Unable to extract the masks by their names if there is no manifest present on the cryptomatte."
			);
		}

		auto manif = m_Metadata.manifest().value();
		std::vector<uint32_t> hashes;
		for (const auto& name : names)
		{
			// This will throw std::invalid_argument on failure to find the name.
			hashes.push_back(manif.hash(name));
		}
		return masks(std::move(hashes));
	}

	// -----------------------------------------------------------------------------------
	// -----------------------------------------------------------------------------------
	std::unordered_map<std::string, std::vector<float32_t>> cryptomatte::masks(std::vector<uint32_t> hashes) const
	{
		// Set up the output map mapped by float32_t at first since that is the storage type, we remap later outside
		// of the hot loop.
		std::unordered_map<float32_t, std::vector<float32_t>> out;
		std::unordered_set<float32_t> requested_hashes;
		const auto& first_channel = this->m_Channels.begin()->second;
		const auto _first_chunk_num_elems = first_channel.chunk_size() / sizeof(float32_t);
		const auto _width = this->width();
		const auto _height = this->height();
		for (const auto& hash : hashes)
		{
			requested_hashes.insert(std::bit_cast<float32_t>(hash));
			out[std::bit_cast<float32_t>(hash)] = {};
		}
		std::for_each(std::execution::par_unseq, out.begin(), out.end(), [&](auto& pair)
			{
				pair.second = std::vector<float32_t>(_width * _height);
			});

		compressed::util::default_init_vector<float32_t> rank_chunk(first_channel.chunk_size() / sizeof(float32_t));
		compressed::util::default_init_vector<float32_t> covr_chunk(first_channel.chunk_size() / sizeof(float32_t));

		// Iterate rank and coverage channels together
		for (size_t i = 0; i + 1 < m_Channels.size(); i += 2)
		{
			_CRYPTOMATTE_PROFILE_SCOPE("iter rank-coverage pairs");
			const auto& rank_channel = m_Channels[i].second;
			const auto& covr_channel = m_Channels[i + 1].second;

			std::unordered_set<float32_t> hashes_in_rank_channel;

			// Iterate the chunks, decompressing on the fly
			for (size_t chunk_idx : std::views::iota(size_t{ 0 }, rank_channel.num_chunks()))
			{
				_CRYPTOMATTE_PROFILE_SCOPE("iter chunks");

				size_t chunk_num_elems = rank_channel.chunk_size(chunk_idx) / sizeof(float32_t);
				{
					_CRYPTOMATTE_PROFILE_SCOPE("decompress rank chunk");
					rank_channel.get_chunk(std::span<float32_t>(rank_chunk.data(), chunk_num_elems), chunk_idx);
				}

				const size_t thread_count = std::thread::hardware_concurrency();
				auto _ids_in_chunk = detail::accumulate_ids_in_rank_chunk(std::span<float32_t>(rank_chunk), thread_count);
				hashes_in_rank_channel.insert(_ids_in_chunk.begin(), _ids_in_chunk.end());

				// Since we only care about the hashes we were asked about, we take the intersection of the ids requested
				// and the ids in the chunk.
				std::unordered_set<float32_t> ids_in_chunk_isection;
				for (const auto& id : _ids_in_chunk) 
				{
					if (requested_hashes.count(id)) 
					{
						ids_in_chunk_isection.insert(id);
					}
				}


				// No ids in chunk -> skip
				if (ids_in_chunk_isection.empty())
				{
					continue;
				}

				// Only now decompress the coverage channel, once we know there's data to get.
				{
					_CRYPTOMATTE_PROFILE_SCOPE("decompress coverage chunk");
					covr_channel.get_chunk(std::span<float32_t>(covr_chunk.data(), chunk_num_elems), chunk_idx);
				}

				// First set up the spans correctly
				std::unordered_map<float32_t, std::span<float32_t>> mask_spans;
				for (auto id : ids_in_chunk_isection)
				{
					size_t base_offset = chunk_idx * _first_chunk_num_elems;
					mask_spans[id] = std::span<float32_t>(out.at(id).data() + base_offset, chunk_num_elems);
				}


				// Accumulate the output pixel from all of the coverage channels.
				{
					_CRYPTOMATTE_PROFILE_SCOPE("accumulate masks");
					auto pixel_iota = std::views::iota(size_t{ 0 }, chunk_num_elems);
					std::for_each(std::execution::par_unseq, pixel_iota.begin(), pixel_iota.end(), [&](size_t idx)
						{
							// Skip any zero rank-channels, accumulate the rest, 
							if (rank_chunk[idx] != static_cast<float32_t>(0))
							{
								if (ids_in_chunk_isection.contains(rank_chunk[idx]))
								{
									auto& it = mask_spans.at(rank_chunk[idx]);
									it[idx] += covr_chunk[idx];
								}
							}
						});
				}

				// Once we have zero hashes in the whole rank channel, we can be pretty confident 
				// that there will be no more hashes in subsequent rank-coverage pairs due to the 
				// cascading nature of hashes, therefore we can safely exit out of the loop saving
				// us iteration and decompression/compression costs.
				if (hashes_in_rank_channel.empty())
				{
					break;
				}
			}
		}

		// Now convert the floating point values into strings for the output mapping.
		return detail::map_to_string(
			std::move(out),
			m_Metadata.manifest().value_or(NAMESPACE_CRYPTOMATTE_API::manifest())
		);
	}

	// -----------------------------------------------------------------------------------
	// -----------------------------------------------------------------------------------
	std::unordered_map<std::string, std::vector<float32_t>> cryptomatte::masks() const
	{
		// Set up the output map mapped by float32_t at first since that is the storage type, we remap later outside
		// of the hot loop.
		std::unordered_map<float32_t, std::vector<float32_t>> out;
		const auto& first_channel = this->m_Channels.begin()->second;
		const auto _first_chunk_num_elems = first_channel.chunk_size() / sizeof(float32_t);
		const auto _width = this->width();
		const auto _height = this->height();

		compressed::util::default_init_vector<float32_t> rank_chunk(first_channel.chunk_size() / sizeof(float32_t));
		compressed::util::default_init_vector<float32_t> covr_chunk(first_channel.chunk_size() / sizeof(float32_t));

		// Iterate rank and coverage channels together
		for (size_t i = 0; i + 1 < m_Channels.size(); i += 2)
		{
			_CRYPTOMATTE_PROFILE_SCOPE("iter rank-coverage pairs");
			const auto& rank_channel = m_Channels[i].second;
			const auto& covr_channel = m_Channels[i + 1].second;

			std::unordered_set<float32_t> hashes_in_rank_channel;

			// Iterate the chunks, decompressing on the fly
			for (size_t chunk_idx : std::views::iota(size_t{ 0 }, rank_channel.num_chunks()))
			{
				_CRYPTOMATTE_PROFILE_SCOPE("iter chunks");

				size_t chunk_num_elems = rank_channel.chunk_size(chunk_idx) / sizeof(float32_t);
				{
					_CRYPTOMATTE_PROFILE_SCOPE("decompress rank chunk");
					rank_channel.get_chunk(std::span<float32_t>(rank_chunk.data(), chunk_num_elems), chunk_idx);
				}

				const size_t thread_count = std::thread::hardware_concurrency();
				auto _ids_in_chunk = detail::accumulate_ids_in_rank_chunk(std::span<float32_t>(rank_chunk), thread_count);
				hashes_in_rank_channel.insert(_ids_in_chunk.begin(), _ids_in_chunk.end());

				// No ids in chunk -> skip
				if (_ids_in_chunk.empty())
				{
					continue;
				}

				// Allocate all the memory for any masks that are not yet initialized.
				for (const auto& hash : _ids_in_chunk)
				{
					if (!out.contains(hash))
					{
						out[std::bit_cast<float32_t>(hash)] = std::vector<float32_t>(_width * _height);
					}
				}

				// Only now decompress the coverage channel, once we know there's data to get.
				{
					_CRYPTOMATTE_PROFILE_SCOPE("decompress coverage chunk");
					covr_channel.get_chunk(std::span<float32_t>(covr_chunk.data(), chunk_num_elems), chunk_idx);
				}

				// First set up the spans correctly
				std::unordered_map<float32_t, std::span<float32_t>> mask_spans;
				for (auto id : _ids_in_chunk)
				{
					size_t base_offset = chunk_idx * _first_chunk_num_elems;
					mask_spans[id] = std::span<float32_t>(out.at(id).data() + base_offset, chunk_num_elems);
				}


				// Accumulate the output pixel from all of the coverage channels.
				{
					_CRYPTOMATTE_PROFILE_SCOPE("accumulate masks");
					auto pixel_iota = std::views::iota(size_t{ 0 }, chunk_num_elems);
					std::for_each(std::execution::par_unseq, pixel_iota.begin(), pixel_iota.end(), [&](size_t idx)
						{
							// Skip any zero rank-channels, accumulate the rest, 
							if (rank_chunk[idx] != static_cast<float32_t>(0))
							{
								auto& it = mask_spans.at(rank_chunk[idx]);
								it[idx] += covr_chunk[idx];
							}
						});
				}

				// Once we have zero hashes in the whole rank channel, we can be pretty confident 
				// that there will be no more hashes in subsequent rank-coverage pairs due to the 
				// cascading nature of hashes, therefore we can safely exit out of the loop saving
				// us iteration and decompression/compression costs.
				if (hashes_in_rank_channel.empty())
				{
					break;
				}
			}
		}

		// Now convert the floating point values into strings for the output mapping.
		return detail::map_to_string(
			std::move(out),
			m_Metadata.manifest().value_or(NAMESPACE_CRYPTOMATTE_API::manifest())
		);
	}

	// -----------------------------------------------------------------------------------
	// -----------------------------------------------------------------------------------
	std::unordered_map<std::string, compressed::channel<float32_t>> cryptomatte::masks_compressed(std::vector<std::string> names) const
	{
		if (!m_Metadata.manifest().has_value())
		{
			throw std::invalid_argument(
				"Unable to extract the masks by their names if there is no manifest present on the cryptomatte."
			);
		}

		auto manif = m_Metadata.manifest().value();
		std::vector<uint32_t> hashes;
		for (const auto& name : names)
		{
			// This will throw std::invalid_argument on failure to find the name.
			hashes.push_back(manif.hash(name));
		}
		return masks_compressed(std::move(hashes));
	}

	// -----------------------------------------------------------------------------------
	// -----------------------------------------------------------------------------------
	std::unordered_map<std::string, compressed::channel<float32_t>> cryptomatte::masks_compressed(std::vector<uint32_t> hashes) const
	{
		_CRYPTOMATTE_PROFILE_FUNCTION();

		// Set up the output map mapped by float32_t at first since that is the storage type, we remap later outside
		// of the hot loop.
		std::unordered_map<float32_t, compressed::channel<float32_t>> out;
		std::unordered_set<float32_t> requested_hashes;
		const auto& first_channel = this->m_Channels.begin()->second;
		const auto _width = this->width();
		const auto _height = this->height();
		for (const auto& hash : hashes)
		{
			// Generate a lazy channel that we will use to fill, using a lazy channel allows us to 
			// not pay any memory allocation cost beyond a single chunk which will be reused. 
			// Ensure we use the same parameters as our channels as the ctor taking compressed::channel instances
			// and thus have non-standard block and chunk sizes.
			auto channel = compressed::channel<float32_t>::zeros(
				_width,
				_height,
				first_channel.compression(),
				static_cast<uint8_t>(first_channel.compression_level()),
				first_channel.block_size(),
				first_channel.chunk_size()
			);
			channel.update_nthreads(1, first_channel.block_size());
			out[std::bit_cast<float32_t>(hash)] = std::move(channel);
			requested_hashes.insert(std::bit_cast<float32_t>(hash));
		}

		// Allocate a contiguous memory chunk and generate an index into it based off the ids request. 
		// This allows us to fill the memory to zeros in parallel (which is usually faster)
		// while also being a contiguous chunk for better data locality.
		compressed::util::default_init_vector<float32_t> _mask_buffer;

		compressed::util::default_init_vector<float32_t> rank_chunk(first_channel.chunk_size() / sizeof(float32_t));
		compressed::util::default_init_vector<float32_t> covr_chunk(first_channel.chunk_size() / sizeof(float32_t));

		// Iterate rank and coverage channels together
		for (size_t i = 0; i + 1 < m_Channels.size(); i += 2)
		{
			_CRYPTOMATTE_PROFILE_SCOPE("iter rank-coverage pairs");
			const auto& rank_channel = m_Channels[i].second;
			const auto& covr_channel = m_Channels[i + 1].second;

			std::unordered_set<float32_t> hashes_in_rank_channel;

			// Iterate the chunks, decompressing on the fly
			for (size_t chunk_idx : std::views::iota(size_t{ 0 }, rank_channel.num_chunks()))
			{
				_CRYPTOMATTE_PROFILE_SCOPE("iter chunks");

				size_t chunk_num_elems = rank_channel.chunk_size(chunk_idx) / sizeof(float32_t);
				{
					_CRYPTOMATTE_PROFILE_SCOPE("decompress rank chunk");
					rank_channel.get_chunk(std::span<float32_t>(rank_chunk.data(), chunk_num_elems), chunk_idx);
				}

				const size_t thread_count = std::thread::hardware_concurrency();
				auto _ids_in_chunk = detail::accumulate_ids_in_rank_chunk(std::span<float32_t>(rank_chunk), thread_count);
				hashes_in_rank_channel.insert(_ids_in_chunk.begin(), _ids_in_chunk.end());

				// Since we only care about the hashes we were asked about, we take the intersection of the ids requested
				// and the ids in the chunk.
				std::unordered_set<float32_t> ids_in_chunk_isection;
				for (const auto& id : _ids_in_chunk) 
				{
					if (requested_hashes.count(id)) 
					{
						ids_in_chunk_isection.insert(id);
					}
				}


				// No ids in chunk -> skip
				if (ids_in_chunk_isection.empty())
				{
					continue;
				}

				// Only now decompress the coverage channel, once we know there's data to get.
				{
					_CRYPTOMATTE_PROFILE_SCOPE("decompress coverage chunk");
					covr_channel.get_chunk(std::span<float32_t>(covr_chunk.data(), chunk_num_elems), chunk_idx);
				}

				// Resize the vector only if we need a larger size, avoids having to realloc this data every iteration.
				// gives us back a mapping of ids to spans to use for mask decoding.
				auto mask_spans = detail::realloc_mask_buffer_if_necessary(_mask_buffer, ids_in_chunk_isection, chunk_num_elems);

				// Now fill them in parallel, note that this doesn't std::fill 0 into the vector but instead
				// uses get_chunk in case any of the previous rank-coverage pairs already included this mask.
				{
					_CRYPTOMATTE_PROFILE_SCOPE("decompress mask chunks");
					std::for_each(std::execution::par_unseq, mask_spans.begin(), mask_spans.end(), [&](auto pair)
						{
							auto key = pair.first;
							out.at(key).get_chunk(pair.second, chunk_idx);
						});
				}

				// Accumulate the output pixel from all of the coverage channels.
				{
					_CRYPTOMATTE_PROFILE_SCOPE("accumulate masks");
					auto pixel_iota = std::views::iota(size_t{ 0 }, chunk_num_elems);
					std::for_each(std::execution::par_unseq, pixel_iota.begin(), pixel_iota.end(), [&](size_t idx)
						{
							// Skip any zero rank-channels, accumulate the rest, 
							if (rank_chunk[idx] != static_cast<float32_t>(0))
							{
								if (ids_in_chunk_isection.contains(rank_chunk[idx]))
								{
									auto& it = mask_spans.at(rank_chunk[idx]);
									it[idx] += covr_chunk[idx];
								}
							}
						});
				}

				// Set the data again, will recompress
				{
					_CRYPTOMATTE_PROFILE_SCOPE("recompress mask chunks");
					std::for_each(std::execution::par_unseq, mask_spans.begin(), mask_spans.end(), [&](auto pair)
						{
							auto key = pair.first;
							out.at(key).set_chunk(pair.second, chunk_idx);
						});
				}
			}

			// Once we have zero hashes in the whole rank channel, we can be pretty confident 
			// that there will be no more hashes in subsequent rank-coverage pairs due to the 
			// cascading nature of hashes, therefore we can safely exit out of the loop saving
			// us iteration and decompression/compression costs.
			if (hashes_in_rank_channel.empty())
			{
				break;
			}
		}

		// Now convert the floating point values into strings for the output mapping.
		return detail::map_to_string(
			std::move(out),
			m_Metadata.manifest().value_or(NAMESPACE_CRYPTOMATTE_API::manifest())
		);
	}

	// -----------------------------------------------------------------------------------
	// -----------------------------------------------------------------------------------
	std::unordered_map<std::string, compressed::channel<float32_t>> cryptomatte::masks_compressed() const
	{
		_CRYPTOMATTE_PROFILE_FUNCTION();
		// Generate this as a float32_t and then remap later since the pixels store it as float32_t so we don't have
		// to do this in the hot loop
		std::unordered_map<float32_t, compressed::channel<float32_t>> out;
		if (m_Metadata.manifest())
		{
			out.reserve(m_Metadata.manifest().value().size());
		}

		// Lambda for generating a new lazy channel on demand. This way we only
		// create the channels when we encounter them.
		const auto& first_channel = this->m_Channels.begin()->second;
		const auto _width = this->width();
		const auto _height = this->height();
		auto generate_lazy_channel = [&]()
			{
				_CRYPTOMATTE_PROFILE_FUNCTION();
				// Generate a lazy channel that we will use to fill, using a lazy channel allows us to 
				// not pay any memory allocation cost beyond a single chunk which will be reused. 
				// Ensure we use the same parameters as our channels as the ctor taking compressed::channel instances
				// and thus have non-standard block and chunk sizes.
				auto channel = compressed::channel<float32_t>::zeros(
					_width,
					_height,
					first_channel.compression(),
					static_cast<uint8_t>(first_channel.compression_level()),
					first_channel.block_size(),
					first_channel.chunk_size()
				);
				channel.update_nthreads(1, first_channel.block_size());
				return channel;
			};


		// Allocate a contiguous memory chunk and generate an index into it based off all the ids in the 
		// current chunk. This allows us to fill the memory to zeros in parallel (which is usually faster)
		// while also being a contiguous chunk for better data locality.
		compressed::util::default_init_vector<float32_t> _mask_buffer;

		compressed::util::default_init_vector<float32_t> rank_chunk(first_channel.chunk_size() / sizeof(float32_t));
		compressed::util::default_init_vector<float32_t> covr_chunk(first_channel.chunk_size() / sizeof(float32_t));

		// Iterate rank and coverage channels together
		for (size_t i = 0; i + 1 < m_Channels.size(); i += 2)
		{
			_CRYPTOMATTE_PROFILE_SCOPE("iter rank-coverage pairs");
			const auto& rank_channel = m_Channels[i].second;
			const auto& covr_channel = m_Channels[i + 1].second;

			std::unordered_set<float32_t> hashes_in_rank_channel;

			// Iterate the chunks, decompressing on the fly
			for (size_t chunk_idx : std::views::iota(size_t{ 0 }, rank_channel.num_chunks()))
			{
				_CRYPTOMATTE_PROFILE_SCOPE("iter chunks");

				size_t chunk_num_elems = rank_channel.chunk_size(chunk_idx) / sizeof(float32_t);
				{
					_CRYPTOMATTE_PROFILE_SCOPE("decompress rank chunk");
					rank_channel.get_chunk(std::span<float32_t>(rank_chunk.data(), chunk_num_elems), chunk_idx);
				}

				const size_t thread_count = std::thread::hardware_concurrency();
				auto ids_in_chunk = detail::accumulate_ids_in_rank_chunk(std::span<float32_t>(rank_chunk), thread_count);
				hashes_in_rank_channel.insert(ids_in_chunk.begin(), ids_in_chunk.end());
				{
					_CRYPTOMATTE_PROFILE_SCOPE("generate_lazy_channel");
					for (float32_t id : ids_in_chunk)
					{
						if (!out.contains(id))
						{
							out[id] = generate_lazy_channel();
						}
					}
				}

				// No ids in chunk -> skip
				if (ids_in_chunk.empty())
				{
					continue;
				}

				// Only now decompress the coverage channel, once we know there's data to get.
				{
					_CRYPTOMATTE_PROFILE_SCOPE("decompress coverage chunk");
					covr_channel.get_chunk(std::span<float32_t>(covr_chunk.data(), chunk_num_elems), chunk_idx);
				}

				// Resize the vector only if we need a larger size, avoids having to realloc this data every iteration.
				auto mask_spans = detail::realloc_mask_buffer_if_necessary(_mask_buffer, ids_in_chunk, chunk_num_elems);

				{
					_CRYPTOMATTE_PROFILE_SCOPE("decompress mask chunks");
					// Now fill them in parallel, note that this doesn't std::fill 0 into the vector but instead
					// uses get_chunk in case any of the previous rank-coverage pairs already included this mask.
					std::for_each(std::execution::par_unseq, mask_spans.begin(), mask_spans.end(), [&](auto pair)
						{
							auto key = pair.first;
							out.at(key).get_chunk(pair.second, chunk_idx);
						});
				}

				{
					_CRYPTOMATTE_PROFILE_SCOPE("accumulate masks");
					// Accumulate the output pixel from all of the coverage channels.
					auto pixel_iota = std::views::iota(size_t{ 0 }, chunk_num_elems);
					std::for_each(std::execution::par_unseq, pixel_iota.begin(), pixel_iota.end(), [&](size_t idx)
						{
							// Skip any zero rank-channels, accumulate the rest, 
							if (rank_chunk[idx] != static_cast<float32_t>(0))
							{
								auto& it = mask_spans.at(rank_chunk[idx]);
								it[idx] += covr_chunk[idx];
							}
						});
				}

				{
					_CRYPTOMATTE_PROFILE_SCOPE("recompress mask chunks");
					// Set the data again, will recompress
					std::for_each(std::execution::par_unseq, mask_spans.begin(), mask_spans.end(), [&](auto pair)
						{
							auto key = pair.first;
							out.at(key).set_chunk(pair.second, chunk_idx);
						});
				}
			}
		
			// Once we have zero hashes in the whole rank channel, we can be pretty confident 
			// that there will be no more hashes in subsequent rank-coverage pairs due to the 
			// cascading nature of hashes, therefore we can safely exit out of the loop saving
			// us iteration and decompression/compression costs.
			if (hashes_in_rank_channel.empty())
			{
				break;
			}
		}

		// Now convert the floating point values into strings for the output mapping.
		return detail::map_to_string(
			std::move(out), 
			m_Metadata.manifest().value_or(NAMESPACE_CRYPTOMATTE_API::manifest())
		);
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