#include "file.h"

#include "util.h"

namespace NAMESPACE_CRYPTOMATTE_API
{

	// -----------------------------------------------------------------------------------
	// -----------------------------------------------------------------------------------
	file::file(
		std::unordered_map<std::string, compressed::channel<float32_t>> channels, 
		std::string name, 
		std::string key, 
		std::optional<cryptomatte::manifest> _manifest /*= std::nullopt */
	)
	{
		m_Channels = std::move(channels);
		m_Name = name;
		m_Key = key;
		m_Manifest = std::move(_manifest);

		
	}

	// -----------------------------------------------------------------------------------
	// -----------------------------------------------------------------------------------
	std::vector<file> file::load(std::filesystem::path file)
	{

	}

} // NAMESPACE_CRYPTOMATTE_API