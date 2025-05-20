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

#include "detail/macros.h"
#include "detail/json_alias.h"

#include "manifest.h"

namespace NAMESPACE_CRYPTOMATTE_API
{

	struct metadata
	{

		metadata() = default;

		/// Construct a cryptomattes' metadata from the given parameters extracted from the images' metadata.
		///
		/// \param name       The name of the cryptomattes, will be used to get the valid channel structure.
		/// \param key        The key or hash of this unique cryptomatte. As a file may store more than one cryptomatte
		///				      this uniquely identifies the cryptomatte within the file. This is not to be confused with the 
		///				      hash of an individual matte.
		/// \param hash       The hashing method used internally to generate the IDs, this must be 'MurmurHash3_32'
		/// \param conversion The conversion method of pixel values into hashes (only relevant for the rank channel 
		///					  of a cryptomatte). This must be 'uint32_to_float32'.
		/// 
		/// \throws std::runtime_error If the hash or conversion are not valid. See `hash_method` and `conversion_method`
		///							   for more information.
		metadata(
			std::string name,
			std::string key,
			std::string hash,
			std::string conversion,
			std::optional<manifest> manif = std::nullopt
		);

		/// Deserialize the cryptomattes' metadata from an imagespec.
		///
		/// Loads the cryptomatte related values into `metadata` structs. This may return none, one, or more than one
		/// items depending on how many cryptomattes are in the file. No metadata conversely also means that there 
		/// is no cryptomatte in the file. The individual definitions are separated by their `key` field.
		/// 
		/// This function is equivalent to calling `from_param_value_list(spec.extra_attribs)`.
		/// 
		/// \param spec The ImageSpec to deserialize the metadata from.
		/// 
		/// \throws std::runtime_error If the metadata is malformed or otherwise incorrect in its definition.
		///							   On failure of a single cryptomatte this function will abort and not parse the 
		///							   other, potentially valid cryptomattes.
		/// 
		/// \returns a vector of metadata definitions, there is no limit to how many there may be in a single file.
		static std::vector<metadata> from_spec(const OIIO::ImageSpec& spec);

		/// Deserialize the cryptomattes' metadata from a param value list.
		///
		/// Loads the cryptomatte related values into `metadata` structs. This may return none, one, or more than one
		/// items depending on how many cryptomattes are in the file. No metadata conversely also means that there 
		/// is no cryptomatte in the file. The individual definitions are separated by their `key` field.
		/// 
		/// \param list The ParamValueList to deserialize the metadata from.
		/// 
		/// \throws std::runtime_error If the metadata is malformed or otherwise incorrect in its definition.
		///							   On failure of a single cryptomatte this function will abort and not parse the 
		///							   other, potentially valid cryptomattes.
		/// 
		/// \returns a vector of metadata definitions, there is no limit to how many there may be in a single file.
		static std::vector<metadata> from_param_value_list(const OIIO::ParamValueList& list);

		/// Deserialize the cryptomattes' metadata from a json of the images' metadata.
		///
		/// Loads the cryptomatte related values into `metadata` structs. This may return none, one, or more than one
		/// items depending on how many cryptomattes are in the file. No metadata conversely also means that there 
		/// is no cryptomatte in the file. The individual definitions are separated by their `key` field.
		/// 
		/// \param json The ordered json to deserialize the metadata from.
		/// 
		/// \throws std::runtime_error If the metadata is malformed or otherwise incorrect in its definition.
		///							   On failure of a single cryptomatte this function will abort and not parse the 
		///							   other, potentially valid cryptomattes.
		/// 
		/// \returns a vector of metadata definitions, there is no limit to how many there may be in a single file.
		static std::vector<metadata> from_json(const json_ordered& json);

		/// Retrieve all the cryptomatte channel names for the given list of channelnames in the order they came in.
		/// Filters all of them according to `is_valid` and returns all the channel names matching this.
		std::vector<std::string> channel_names(const std::vector<std::string>& channelnames) const;

		/// Retrieve all the legacy cryptomatte channel names for the given list of channelnames in the order they came in.
		/// Filters all of them according to `is_valid_legacy` and returns all the channel names matching this.
		std::vector<std::string> legacy_channel_names(const std::vector<std::string>& channelnames) const;

		/// Check whether the passed channel name is a channel name of this metadata (excluding legacy channels).
		/// Channel names must follow the following convention
		/// 
		/// {typename}00.r
		/// {typename}00.g
		/// {typename}00.b
		/// {typename}00.a
		/// {typename}01.r
		/// ...
		/// 
		/// where typename is `name` 
		/// 
		/// \param channel_name The name to match against.
		/// \returns Whether the passed `channel_name` is a channel name.
		bool is_valid(std::string channel_name) const;

		/// Check whether the passed channel name is a legacy channel name of this metadata.
		/// Legacy channels in cryptomattes are one of the following:
		/// 
		/// {typename}.r
		/// {typename}.g
		/// {typename}.b
		/// 
		/// where typename is `name` 
		/// 
		/// \param channel_name The name to match against.
		/// \returns Whether the passed `channel_name` is a legacy channel name.
		bool is_valid_legacy(std::string channel_name) const;

		/// Retrieve the name, also referred to as 'typename' of the cryptomatte.
		///
		/// This determines the name the cryptomatte channels will have. For example, for the first rank channel
		/// the name of the channel would be '{typename}00.r'
		std::string name() const;

		/// Retrieve the key of the cryptomatte, this uniquely identifies the cryptomatte within the metadata.
		///
		/// This has no function outside of the metadata and consumers of the API do typically not have to deal
		/// with this. This will however tell you which part of the metadata refers to this specific cryptomatte.
		std::string key() const;

		/// Retrieve the hashing method used for encoding, always 'MurmurHash3_32'
		std::string_view hash_method() const;

		/// Retrieve the conversion method used for converting the rank-channel pixels into hashes. 
		/// Always 'uint32_to_float32'.
		std::string_view conversion_method() const;

		/// Retrieve the manifest (if present) from the metadata, this may be empty and should not be relied
		/// upon for decoding the cryptomatte masks.
		std::optional<NAMESPACE_CRYPTOMATTE_API::manifest> manifest() const;

		/// Constants for the identifiers of these attributes within the metadata.
		/// Cryptomattes store their metadata in the following format cryptomatte/<key>/<attribute>.
		/// These functions give you all the valid names for these attributes. These are 'name',
		/// 'hash', 'conversion', 'manifest' and 'manif_file'.
		static std::string attrib_name_identifier();
		static std::string attrib_hash_method_identifier();
		static std::string attrib_conversion_method_identifier();
		static std::string attrib_manifest_identifier();
		static std::string attrib_manif_file_identifier();

	private:
		/// The name of the cryptomatte type, for example, 'CryptoAsset'. This will be propagate to the channel names
		/// such that e.g. CryptoAsset01.r will be associated with the name.
		std::string m_Name;
		/// 7-char key that uniquely identifies the metadata associated with this cryptomatte. A single multi-part file
		/// can hold any number of cryptomattes and thus can hold multiple 'cryptomatte/<hash>/name' items. This 
		/// disambiguates them.
		std::string m_Key;
		/// As of the 1.2.0 cryptomatte spec these are constants with no alternatives available.
		/// Describes the hashing algorithm when encoding, for decoding this has no bearing. 
		static inline const std::string m_Hash = "MurmurHash3_32";
		/// As of the 1.2.0 cryptomatte spec these are constants with no alternatives available.
		/// Describes the mapping of hashes to pixel values, in this case mapping from a uint32_t
		/// hash to a float32_t pixel value.
		static inline const std::string m_Conversion = "uint32_to_float32";
		/// Cryptomatte manifest containing a mapping of human readable names to their uint32_t hashes. This manifest
		/// is not strictly required and therefore may not exist. It is implemented either as a json sidecar file or as
		/// an embedded json.
		std::optional<NAMESPACE_CRYPTOMATTE_API::manifest> m_Manifest;

		/// A list of all of the valid attribute names that cryptomatte metadata may contain. Used internally
		/// during parsing to validate.
		static inline const std::array<std::string, 5> s_ValidAttribs = {
			"name", "hash", "conversion", "manifest", "manif_file"
		};
		
	};

} // NAMESPACE_CRYPTOMATTE_API