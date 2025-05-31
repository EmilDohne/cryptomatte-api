#pragma once

#include <filesystem>
#include <fstream>
#include <string>
#include <iostream>
#include <format>
#include <type_traits>
#include <optional>
#include <vector>
#include <bit>
#include <variant>

#include "detail/macros.h"
#include "detail/json_alias.h"

namespace NAMESPACE_CRYPTOMATTE_API
{

	struct manifest
	{
		/// \{
		/// \name creating a manifest

		manifest() = default;

		/// Load and decode a manifest from a json (this would be the cryptomatte/<hash>/manifest or 
		/// cryptomatte/<hash>/manif_file).
		/// 
		/// \param json The json to load from.
		explicit manifest(json_ordered json);

		/// Load and decode a manifest from a json string (this would be the cryptomatte/<hash>/manifest or 
		/// cryptomatte/<hash>/manif_file).
		/// 
		/// \param json The json string, must be a valid json.
		/// 
		/// \return The decoded manifest.
		static manifest from_str(std::string json);

		/// Load the manifest from the passed image metadata, optionally returning a decoded manifest.
		/// 
		/// This function scans the metadata for a manifest string or filename, attempts to read and decode
		/// the manifest data, and returns it if successful. If a sidecar file is used (very rare), the
		/// function will also attempt to load it from disk.
		/// 
		/// \param manif_key The metadata key for the manifest, will be used to determine whether its a sidecar or embedded.
		/// \param manif_value The value found on the cryptomattes' 'manifest' or 'manif_file' value, we willtake care of
		///					   parsing internally.
		/// \param image_path The path to the image that the cryptomatte was loaded from, required to successfully decode
		///					  sidecar files.
		/// 
		/// \return A decoded manifest if present and successfully parsed; otherwise, std::nullopt.
		static std::optional<manifest> load(std::string manif_key, std::string manif_value, std::filesystem::path image_path) noexcept;

		/// \}


		/// Check whether the manifest contains the passed name.
		/// 
		/// \param name The name to check for existence within the manifest.
		/// 
		/// \return True if the name exists in the manifest, false otherwise.
		bool contains(std::string_view name);

		/// \{
		/// \name retrieving the mapping

		std::vector<std::string> names() const noexcept;

		/// Retrieve the full name-to-hash mapping, cast to the specified type.
		/// 
		/// This function provides access to the name-hash mapping in the desired format:
		/// - `uint32_t`: The raw internal representation (default).
		/// - `float32_t`: A bit-cast form of the hash, this is what the hashes are stored as in-file.
		/// - `std::string`: A hexadecimal string representation of the hash.
		///
		/// \tparam T The type to return the hash values as, defaulting to `uint32_t`. Must be one of:
		///           `float32_t`, `std::string`, or `uint32_t`.
		/// 
		/// \return A vector of name-hash pairs in the specified format.
		template <typename T = uint32_t>
			requires std::is_same_v<T, float32_t> || std::is_same_v<T, std::string> || std::is_same_v<T, uint32_t>
		std::vector<std::pair<std::string, T>> mapping() const
		{
			// Same as we store it, return as is
			if constexpr (std::is_same_v<T, uint32_t>)
			{
				return m_Mapping;
			}

			// Bit-cast the uint32_t hash into a float32_t representation. While during encoding, one must take
			// care to avoid NaNs and inf by clamping the exponent we don't have to worry about it as we are
			// only decoding it.
			else if constexpr (std::is_same_v<T, float32_t>)
			{
				std::vector<std::pair<std::string, float32_t>> result;
				for (const auto& [key, val] : m_Mapping)
				{
					result.push_back(std::make_pair(key, std::bit_cast<float32_t>(val)));
				}
				return result;
			}

			// Convert it back into a hex 8-char string using fmt.
			else
			{
				std::vector<std::pair<std::string, std::string>> result;
				for (const auto& [key, val] : m_Mapping)
				{
					result.push_back(std::make_pair(key, std::format("{:08x}", val)));
				}

				return result;
			}
		}

		/// Get the hash associated with the given name
		/// 
		/// Returns it as the specified template parameter which may be `float32_t`, `std::string` or `uint32_t`
		/// 
		/// \throws std::invalid_argument if the given name does not exist in the manifest. Use `contains` to check
		///								  whether the hash exists
		/// \tparam T The type to retrieve the cryptomatte hash as, may be `float32_t`, `std::string` or `uint32_t` 
		///			  defaulting to `uint32_t`
		/// \return The hash at the given name.
		template <typename T = uint32_t>
			requires std::is_same_v<T, float32_t> || std::is_same_v<T, std::string> || std::is_same_v<T, uint32_t>
		T hash(std::string_view name) const
		{
			for (const auto& [key, value] : m_Mapping)
			{
				if (key == name)
				{
					if constexpr (std::is_same_v<T, uint32_t>)
					{
						return value;
					}
					else if constexpr (std::is_same_v<T, float32_t>)
					{
						return std::bit_cast<float32_t>(value);
					}
					else
					{
						return std::format("{:08x}", value);
					}
				}
			}

			throw std::invalid_argument(
				std::format(
					"Unable to get cryptomatte hash from key {} as it does not exist in the manifest", name
				)
			);
		}

		/// @}

		/// \brief Get the size of the manifest, i.e. how many items are in the mapping.
		size_t size() const noexcept;

	private:
		// The mapping of names into their respective hashes. On-disk these would be stored as e.g.
		// {"bunny":"13851a76", "default" : "42c9679f"}
		// We store these already decoded into uint32_t and provide the mapping() and hash() functions to 
		// allow us to convert it into what is needed at runtime.
		std::vector<std::pair<std::string, uint32_t>> m_Mapping;
	};

}