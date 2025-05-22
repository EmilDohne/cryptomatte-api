#include "logger.h"
#include <spdlog/sinks/stdout_color_sinks.h>


namespace NAMESPACE_CRYPTOMATTE_API
{

	static std::shared_ptr<spdlog::logger> s_logger = nullptr;

	// -----------------------------------------------------------------------------------
	// -----------------------------------------------------------------------------------
	void set_logger(std::shared_ptr<spdlog::logger> new_logger)
	{
		s_logger = std::move(new_logger);
	}

	// -----------------------------------------------------------------------------------
	// -----------------------------------------------------------------------------------
	std::shared_ptr<spdlog::logger> get_logger()
	{
		if (!s_logger)
		{
			// Lazy init with a sensible default
			s_logger = spdlog::stdout_color_mt(s_default_logger_name);
			s_logger->set_level(spdlog::level::warn);
		}
		return s_logger;
	}

}