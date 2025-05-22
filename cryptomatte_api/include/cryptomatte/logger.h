#include "detail/macros.h"

#include <spdlog/spdlog.h>
#include <memory>

namespace NAMESPACE_CRYPTOMATTE_API
{

	/// \brief The default logger name used internally if the consumer does not provide one.
	static inline std::string s_default_logger_name = "cryptomatte_api";

	/// \brief Set the logger instance used by the cryptomatte API.
	///
	/// This function allows consumers of the library to provide their own `spdlog::logger` instance.
	/// This can be useful to integrate the library’s logging output into an existing logging system,
	/// route messages to a file, or change verbosity dynamically.
	///
	/// \code
	/// auto my_logger = spdlog::basic_logger_mt("custom_logger", "log.txt");
	/// my_logger->set_level(spdlog::level::debug);
	/// NAMESPACE_CRYPTOMATTE_API::set_logger(my_logger);
	/// \endcode
	///
	/// If no logger is set, the library will lazily create a default one that logs to `stdout` at warning level.
	///
	/// \param logger The `spdlog::logger` instance to use for all library logging.
	void set_logger(std::shared_ptr<spdlog::logger> logger);

	/// \brief Retrieve the current logger instance used by the cryptomatte API.
	///
	/// If no logger has been previously set via `set_logger`, this function will initialize
	/// a default logger named `"cryptomatte_api"` that logs to standard output with color support,
	/// and at `spdlog::level::warn` verbosity.
	///
	/// \return A shared pointer to the currently active `spdlog::logger`.
	std::shared_ptr<spdlog::logger> get_logger();


} // NAMESPACE_CRYPTOMATTE_API