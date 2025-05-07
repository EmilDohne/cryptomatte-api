#include "manifest.h"

#include "detail/detail.h"

namespace NAMESPACE_CRYPTOMATTE_API
{

	// -----------------------------------------------------------------------------------
	// -----------------------------------------------------------------------------------
	manifest::manifest(json_ordered json)
	{
		m_Mapping.reserve(json.size());
		for (const auto& [key, value] : json.items()) 
		{
			try
			{
				m_Mapping.emplace_back(key, detail::hex_str_to_uint32_t(value.get<std::string>()));
			}
			catch (const std::exception& except)
			{
				throw std::runtime_error(
					std::format(
						"Failed to decode hex string for manifest key {} due to the following reason: {}", key, except.what()
					)
				);
			}
		}
	}


	// -----------------------------------------------------------------------------------
	// -----------------------------------------------------------------------------------
	manifest manifest::from_str(std::string json)
	{
		return manifest(json_ordered::parse(json));
	}


	// -----------------------------------------------------------------------------------
	// -----------------------------------------------------------------------------------
	std::optional<manifest> manifest::load(
		const std::unordered_map<std::string, std::string>& metadata, 
		std::filesystem::path image_path
	)
	{
		for (const auto& [key, val] : metadata)
		{
			// This code checks for either an embedded manifest of a manifest file. The specification says that
			// these are exclusive, however our code does not do a check for this to also accept potentially incorrect
			// files which define both of these, preferring whichever it finds first. 

			// Embedded json manifest
			if (key.find("cryptomatte") != std::string::npos && key.find("manifest") != std::string::npos)
			{
				return manifest::from_str(val);
			}

			// Sidecar manifest file
			if (key.find("cryptomatte") != std::string::npos && key.find("manif_file") != std::string::npos)
			{
				// According to the specification the manifest file has to be relative to the image file, it may
				// also not start with './' or '../'. We do not verify over this second scenario however as this
				// library isn't meant to encode in a compliant matter but rather accept compliant cryptomattes.
				auto sidecar_path = image_path.parent_path() / val;

				// If the file does not exist, we skip over in the hopes of finding a regular manifest or
				// just returning std::nullopt
				if (!std::filesystem::exists(sidecar_path))
				{
					std::cout << std::format(
						"Unable to load cryptomatte manifest from sidecar file '{}' as it does not exist on disk",
						sidecar_path.string()
					) << std::endl;
					continue;
				}

				// The content of this file should be identical to that of the embedded manifest example.
				std::ifstream ifs(sidecar_path);
				json_ordered json_file = json_ordered::parse(ifs);
				return manifest(json_file);
			}
		}

		return std::nullopt;
	}


	// -----------------------------------------------------------------------------------
	// -----------------------------------------------------------------------------------
	bool manifest::contains(std::string_view name)
	{
		for (const auto& [key, val] : m_Mapping)
		{
			if (key == name)
			{
				return true;
			}
		}
		return false;
	}

} // NAMESPACE_CRYPTOMATTE_API