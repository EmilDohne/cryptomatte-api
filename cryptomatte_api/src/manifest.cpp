#include "manifest.h"

#include <format>

#include "detail/detail.h"
#include "logger.h"


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
	std::optional<manifest> manifest::load(std::string manif_key, std::string manif_value, std::filesystem::path image_path) noexcept
	{
		// This code checks for either an embedded manifest of a manifest file. The specification says that
		// these are exclusive, however our code does not do a check for this to also accept potentially incorrect
		// files which define both of these, preferring whichever it finds first. 

		// Embedded json manifest
		if (manif_key.find("cryptomatte") != std::string::npos && manif_key.find("manifest") != std::string::npos)
		{
			try
			{
				return manifest::from_str(manif_value);
			}
			catch (const std::exception& e)
			{
				get_logger()->warn(
					"Exception caught during the loading of the cryptomatte manifest '{}'."
					"The exception was: {}", manif_value, e.what()
				);
				return std::nullopt;
			}
		}

		// Sidecar manifest file
		if (manif_key.find("cryptomatte") != std::string::npos && manif_key.find("manif_file") != std::string::npos)
		{
			// According to the specification the manifest file has to be relative to the image file, it may
			// also not start with './' or '../'. We do not verify over this second scenario however as this
			// library isn't meant to encode in a compliant matter but rather accept compliant cryptomattes.
			auto sidecar_path = image_path.parent_path() / manif_value;

			// If the file does not exist, we alert the user of this and then return a nullopt
			if (!std::filesystem::exists(sidecar_path))
			{
				get_logger()->warn(
					"Unable to load cryptomatte manifest from sidecar file '{}' as it does not exist on disk",
					sidecar_path.string()
				);
				return std::nullopt;
			}

			// The content of this file should be identical to that of the embedded manifest example.
			try
			{
				std::ifstream ifs(sidecar_path);
				json_ordered json_file = json_ordered::parse(ifs);
				return manifest(json_file);
			}
			catch (const std::exception& e)
			{
				get_logger()->warn(
					"Exception caught during the loading of the cryptomatte manifest from the sidecar path '{}'."
					"The exception was: {}", sidecar_path.string(), e.what()
				);
				return std::nullopt;
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


	// -----------------------------------------------------------------------------------
	// -----------------------------------------------------------------------------------
	std::vector<std::string> manifest::names() const noexcept
	{
		std::vector<std::string> out;
		for (const auto& [name, _] : m_Mapping)
		{
			out.push_back(name);
		}
		return out;
	}

	// -----------------------------------------------------------------------------------
	// -----------------------------------------------------------------------------------
	size_t manifest::size() const noexcept
	{
		return m_Mapping.size();
	}

} // NAMESPACE_CRYPTOMATTE_API