#pragma once

#include <set>
#include <unordered_map>
#include <span>

#include "macros.h"
#include "detail.h"
#include "scoped_timer.h"
#include "cryptomatte/manifest.h"

#include <compressed/util.h>


namespace NAMESPACE_CRYPTOMATTE_API
{

	namespace detail
	{


		/// \brief Accumulate all the IDs stored in a given rank chunk in parallel over the passed thread count. 
		/// 
		/// \param rank_chunk   The rank chunk to iterate over (may be any size).
		/// \param thread_count The thread count to parallelize over.
		/// 
		/// \return A set containing all of the float32_t representations of the hashes that are found in that rank
		///			chunk.
		inline std::unordered_set<float32_t> accumulate_ids_in_rank_chunk(std::span<float32_t> rank_chunk, size_t thread_count)
		{
			std::vector<std::unordered_set<float32_t>> thread_local_sets(thread_count);

			// Divide the work into ranges per thread
			{
				_CRYPTOMATTE_PROFILE_SCOPE("get all ids from rank");
				const size_t _chunk_size = rank_chunk.size();
				const size_t _block_size = (_chunk_size + thread_count - 1) / thread_count;
				auto thread_iota = std::views::iota(size_t{ 0 }, thread_count);
				std::for_each(std::execution::par, thread_iota.begin(), thread_iota.end(), [&](size_t thread_idx)
					{
						const size_t start = thread_idx * _block_size;
						const size_t end = std::min(start + _block_size, _chunk_size);

						auto& local_set = thread_local_sets[thread_idx];
						for (size_t i = start; i < end; ++i)
						{
							float32_t elem = rank_chunk[i];
							if (elem != static_cast<float32_t>(0))
							{
								local_set.insert(elem);
							}
						}
					});
			}

			// Now merge the local sets into one large set.
			std::unordered_set<float32_t> ids_in_chunk;
			{
				for (const auto& local_set : thread_local_sets)
				{
					ids_in_chunk.insert(local_set.begin(), local_set.end());
				}
			}

			return ids_in_chunk;
		}


		/// \brief Remap the incoming map from float32_t to std::string using either the manifest or the string hash
		/// 
		/// \tparam storage_type The storage type of the map, not relevant for the remapping, items will be std::move'd
		/// \param in The input data to remap, taken as r-value reference.
		/// \param manif The manifest to use for mapping the names back to a string, if the manifest does not hold the 
		///				 hash we return the hash as a std::string form of the uint32_t.
		/// 
		/// \return The remapped resulting std::unordered_map
		template <typename storage_type>
		std::unordered_map<std::string, storage_type> map_to_string(
			std::unordered_map<float32_t, storage_type>&& in,
			const manifest& manif
		)
		{
			std::unordered_map<std::string, storage_type> out_as_str;
			{
				_CRYPTOMATTE_PROFILE_SCOPE("map by string");
				auto mapping = manif.mapping<float32_t>();
				for (auto& [key, value] : in)
				{
					// Either use the hash as hex or get the name from the mapping.
					std::string name = uint32_t_to_hex_str(std::bit_cast<uint32_t>(key));
					for (const auto [_name, hash] : mapping)
					{
						if (hash == key)
						{
							name = _name;
						}
					}

					out_as_str[name] = std::move(value);
				}
			}
			return out_as_str;
		}

		/// \brief Reallocates the passed mask_buffer to fit the requested number of ids.
		/// 
		/// This will reallocate using an exponential growth-policy and then return a mapping of the id hashes
		/// to a subspan of the mask_buffer big enough to hold one mask chunk.
		/// 
		/// \param mask_buffer The buffer to (potentially) reallocate
		/// \param ids The ids to allocate for and map a subregion to.
		/// \param chunk_num_elems The number of elements in the current chunk, will allocate accordingly
		/// \return A mapping of hashes to their subspan into the mask_buffer.
		inline std::unordered_map<float32_t, std::span<float32_t>> realloc_mask_buffer_if_necessary(
			compressed::util::default_init_vector<float32_t>& mask_buffer,
			const std::unordered_set<float32_t>& ids, 
			size_t chunk_num_elems
		)
		{
			auto _new_max_size = ids.size() * chunk_num_elems;
			if (mask_buffer.size() < _new_max_size)
			{
				_CRYPTOMATTE_PROFILE_SCOPE("realloc mask buffer");
				size_t new_capacity = std::max(mask_buffer.capacity() * 2, _new_max_size);
				mask_buffer.resize(new_capacity);
			}
			std::span<float32_t> _mask_buffer_span(mask_buffer.data(), _new_max_size);
			std::unordered_map<float32_t, std::span<float32_t>> mask_spans;

			// First set up the spans correctly
			size_t _count = 0;
			for (auto id : ids)
			{
				size_t offset = _count * chunk_num_elems;
				mask_spans[id] = std::span<float32_t>(_mask_buffer_span.data() + offset, chunk_num_elems);
				++_count;
			}

			return mask_spans;
		}

	} // detail

} // NAMESPACE_CRYPTOMATTE_API