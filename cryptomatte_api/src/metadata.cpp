#include "metadata.h"

#include "manifest.h"
#include "detail/detail.h"
#include "detail/string_util.h"

#include <compressed/detail/oiio_util.h>
#include <OpenImageIO/imageio.h>

#include <regex>

namespace NAMESPACE_CRYPTOMATTE_API
{

	// -----------------------------------------------------------------------------------
	// -----------------------------------------------------------------------------------
	metadata::metadata(
		std::string name,
		std::string key,
		std::string hash,
		std::string conversion,
		std::optional<NAMESPACE_CRYPTOMATTE_API::manifest> manif /* = std::nullopt */
	)
	{
		m_Name = std::move(name);
		m_Key = std::move(key);
		if (hash != m_Hash)
		{
			throw std::runtime_error(
				std::format
				(
					"Unable to validate metadata, invalid cryptomatte hashing method in metadata."
					" Expected to be 'MurmurHash3_32' but instead received {} while reading metadata"
					" for cryptomatte with key {}",
					hash, key
				)
			);
		}
		if (conversion != m_Conversion)
		{
			throw std::runtime_error(
				std::format
				(
					"Unable to validate metadata, invalid cryptomatte conversion method in metadata."
					" Expected to be 'uint32_to_float32' but instead received {} while reading metadata"
					" for cryptomatte with key {}",
					conversion, key
				)
			);
		}
		m_Manifest = std::move(manif);
	}


	// -----------------------------------------------------------------------------------
	// -----------------------------------------------------------------------------------
	std::vector<metadata> metadata::from_spec(const OIIO::ImageSpec& spec, std::filesystem::path image_path)
	{
		return metadata::from_param_value_list(spec.extra_attribs, image_path);
	}

	// -----------------------------------------------------------------------------------
	// -----------------------------------------------------------------------------------
	std::vector<metadata> metadata::from_param_value_list(const OIIO::ParamValueList& list, std::filesystem::path image_path)
	{
		auto json = compressed::detail::param_value::to_json(list);
		return from_json(json, image_path);
	}

	// -----------------------------------------------------------------------------------
	// -----------------------------------------------------------------------------------
	std::vector<metadata> metadata::from_json(const json_ordered& json, std::filesystem::path image_path)
	{
		std::unordered_map<std::string, metadata> result;

		// Start by iterating all the metadata and loading the cryptomatte related keys,
		// performs validation on the metadata values' type and format.
		for (const auto& [key, value] : json.items())
		{
			if (!key.starts_with("cryptomatte/"))
			{
				continue;
			}

			// Validate that the cryptomatte metadata key follows the convention laid out by the
			// specification. 
			auto split_key = str::split(key, "/");
			if (split_key.size() != 3)
			{
				throw std::runtime_error(
					std::format(
						"Unable to validate metadata, invalid metadata key encountered. All cryptomatte"
						" related keys must have the following format 'cryptomatte/<key>/<attribute>' with"
						" 3 sections divided by a single '/'. Received the following metadata key {}",
						key
					)
				);
			}

			auto& crypto_key = split_key[1];
			auto& crypto_attribute = split_key[2];

			// Check that the attribute part of the key is one of 'name', 'hash', 'conversion', 'manifest' or 
			// 'manif_file'. Any other input is malformed and we throw.
			if (std::find(s_ValidAttribs.begin(), s_ValidAttribs.end(), crypto_attribute) == s_ValidAttribs.end())
			{
				throw std::runtime_error(
					std::format(
						"Invalid cryptomatte metadata attribute encountered. These 5 attributes are known to the"
						" cryptomatte specification: {{ {}, {}, {}, {}, {} }}. However, we got {} which is not"
						" valid. Full metadata key: {}",
						s_ValidAttribs[0], s_ValidAttribs[1], s_ValidAttribs[2], s_ValidAttribs[3], s_ValidAttribs[4],
						crypto_attribute, key
					)
				);
			}

			// Get a reference to the partially populated (or default-initialized) metadata.
			metadata& ref = result[crypto_key];
			ref.m_Key = crypto_key;

			// Now do the actual decoding and storing of the values
			if (crypto_attribute == metadata::attrib_name_identifier())
			{
				if (!value.is_string())
				{
					throw std::runtime_error(
						std::format(
							"Invalid cryptomatte metadata attribute encountered. Attribute {} does not have a value"
							" of type string",
							key
						)
					);
				}
				ref.m_Name = value.template get<std::string>();
			}
			else if (crypto_attribute == metadata::attrib_hash_method_identifier())
			{
				if (!value.is_string())
				{
					throw std::runtime_error(
						std::format(
							"Invalid cryptomatte metadata attribute encountered. Attribute {} does not have a value"
							" of type string",
							key
						)
					);
				}
				// Only 'MurmurHash3_32' is supported by the specification at this time (v1.2.0)
				if (value.template get<std::string>() != "MurmurHash3_32")
				{
					throw std::runtime_error(
						fmt::format(
							"Unable to validate metadata, invalid cryptomatte hashing method in metadata."
							" Expected to be 'MurmurHash3_32' but instead received '{}' while reading metadata"
							" for cryptomatte with key '{}'",
							value.template get<std::string>(),
							crypto_key
						)
					);
				}

				// No need to store as its a constexpr variable
			}
			else if (crypto_attribute == metadata::attrib_conversion_method_identifier())
			{
				if (!value.is_string())
				{
					throw std::runtime_error(
						std::format(
							"Invalid cryptomatte metadata attribute encountered. Attribute {} does not have a value"
							" of type string",
							key
						)
					);
				}
				// Only 'uint32_to_float32' is supported by the specification at this time (v1.2.0)
				if (value.template get<std::string>() != "uint32_to_float32")
				{
					throw std::runtime_error(
						std::format(
							"Unable to validate metadata, invalid cryptomatte conversion method in metadata."
							" Expected to be 'uint32_to_float32' but instead received '{}' while reading metadata"
							" for cryptomatte with key '{}'",
							value.template get<std::string>(),
							crypto_key
						)
					);
				}

				// No need to store as its a constexpr variable
			}
			else if (
				crypto_attribute == metadata::attrib_manifest_identifier() 
				|| crypto_attribute == metadata::attrib_manif_file_identifier()
				)
			{
				// Our manifest struct already handles deserialization from either embedded manifest or
				// manifest file.
				if (!value.is_string() && !value.is_object())
				{
					throw std::runtime_error(
						std::format(
							"Invalid cryptomatte metadata attribute encountered. Attribute {} does not have a value"
							" of type string or dictionary",
							key
						)
					);
				}

				/// Depending on the way the attributes are parsed from e.g. OpenImageIO, they may 
				/// come out as either a json object or a string, we account for either allowing
				/// parsing from json or string.
				if (value.is_string())
				{
					ref.m_Manifest = NAMESPACE_CRYPTOMATTE_API::manifest::load(key, value.template get<std::string>(), image_path);
				}
				else
				{
					ref.m_Manifest = NAMESPACE_CRYPTOMATTE_API::manifest::load(key, value.dump(), image_path);
				}
			}
		}
		
		// Finally, check that the metadata contains the name field and convert the map into a 
		// vector. This is technically more permissive than it should be as the hash and conversion
		// field need to be filled but as these are constants we do not validate against this.
		std::vector<metadata> out;
		for (const auto& [key, meta] : result)
		{
			if (meta.m_Name.empty())
			{
				throw std::runtime_error(
					std::format(
						"Invalid cryptomatte metadata encountered for key '{}'. It does not contain"
						" a valid name field which is required by the specification.",
						key
					)
				);
			}

			out.push_back(std::move(meta));
		}

		return out;
	}


	// -----------------------------------------------------------------------------------
	// -----------------------------------------------------------------------------------
	std::vector<std::string> metadata::channel_names(const std::vector<std::string>& channelnames) const
	{
		std::vector<std::string> out;

		for (const auto& channelname : channelnames)
		{
			if (this->is_valid(channelname))
			{
				out.push_back(channelname);
			}
		}

		return out;
	}

	// -----------------------------------------------------------------------------------
	// -----------------------------------------------------------------------------------
	std::vector<std::string> metadata::legacy_channel_names(const std::vector<std::string>& channelnames) const
	{
		std::vector<std::string> out;

		for (const auto& channelname : channelnames)
		{
			if (this->is_valid_legacy(channelname))
			{
				out.push_back(channelname);
			}
		}

		return out;
	}

	// -----------------------------------------------------------------------------------
	// -----------------------------------------------------------------------------------
	bool metadata::is_valid(std::string channel_name) const
	{
		if (!channel_name.starts_with(this->name()))
		{
			return false;
		}

		auto res = str::lstrip(channel_name, this->name());
		if (res.size() < 2)
		{
			return false;
		}

		// Match any sequence starting with two digits
		std::regex re("^\\d\\d");
		std::smatch re_result;
		if (!std::regex_search(res, re_result, re))
		{
			return false;
		}

		// The specification mentions these channels always ending with r, g or b. However,
		// often times DCCs will write these names according to what they do with other channels.
		// Clarisse e.g. will always write out the full name such as 'red'.
		if (
			res.ends_with(".r") || res.ends_with(".g") || res.ends_with(".b") ||
			res.ends_with(".R") || res.ends_with(".G") || res.ends_with(".B") ||
			res.ends_with(".red") || res.ends_with(".green") || res.ends_with(".blue")
			)
		{
			return true;
		}

		return false;
	}

	// -----------------------------------------------------------------------------------
	// -----------------------------------------------------------------------------------
	bool metadata::is_valid_legacy(std::string channel_name) const
	{
		if (!channel_name.starts_with(this->name()))
		{
			return false;
		}

		auto res = str::lstrip(channel_name, this->name());

		// The specification mentions these channels always ending with r, g or b. However,
		// often times DCCs will write these names according to what they do with other channels.
		// Clarisse e.g. will always write out the full name such as 'red'.
		if (
			res == ".r" || res == ".g" || res == ".b" ||
			res == ".R" || res == ".G" || res == ".B" ||
			res == ".red" || res == ".green" || res == ".blue"
			)
		{
			return true;
		}

		return false;
	}

	// -----------------------------------------------------------------------------------
	// -----------------------------------------------------------------------------------
	std::string metadata::name() const
	{
		return m_Name;
	}

	// -----------------------------------------------------------------------------------
	// -----------------------------------------------------------------------------------
	std::string metadata::key() const
	{
		return m_Key;
	}

	// -----------------------------------------------------------------------------------
	// -----------------------------------------------------------------------------------
	std::string_view metadata::hash_method() const
	{
		return m_Hash;
	}

	// -----------------------------------------------------------------------------------
	// -----------------------------------------------------------------------------------
	std::string_view metadata::conversion_method() const
	{
		return m_Conversion;
	}

	// -----------------------------------------------------------------------------------
	// -----------------------------------------------------------------------------------
	std::optional<manifest> metadata::manifest() const
	{
		return m_Manifest;
	}

	// -----------------------------------------------------------------------------------
	// -----------------------------------------------------------------------------------
	std::string metadata::attrib_name_identifier()
	{
		return metadata::s_ValidAttribs[0];
	}

	// -----------------------------------------------------------------------------------
	// -----------------------------------------------------------------------------------
	std::string metadata::attrib_hash_method_identifier()
	{
		return metadata::s_ValidAttribs[1];
	}

	// -----------------------------------------------------------------------------------
	// -----------------------------------------------------------------------------------
	std::string metadata::attrib_conversion_method_identifier()
	{
		return metadata::s_ValidAttribs[2];
	}

	// -----------------------------------------------------------------------------------
	// -----------------------------------------------------------------------------------
	std::string metadata::attrib_manifest_identifier()
	{
		return metadata::s_ValidAttribs[3];
	}

	// -----------------------------------------------------------------------------------
	// -----------------------------------------------------------------------------------
	std::string metadata::attrib_manif_file_identifier()
	{
		return metadata::s_ValidAttribs[4];
	}

} // NAMESPACE_CRYPTOMATTE_API