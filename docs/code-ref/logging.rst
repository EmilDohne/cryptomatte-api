..
  Copyright Contributors to the cryptomatte-api project.


logging
-------

The cryptomatte-api uses ``spdlog`` as a logging library and adapter. We expose functions to either
register your own logger with it or get the logger used by the cryptomatte-api.

Internally we will call ``cmatte::get_logger`` to retrieve the logging object so this set up can be done 
at any time and any subsequent calls will use the new logger.

Below you can find an example of setting up your own logger to use with the cryptomatte-api:

.. code-block:: cpp

	#include <cryptomatte/cryptomatte.h>
	#include <cryptomatte/logger.h>


	auto main() -> int
	{
		std::shared_ptr<spdlog::logger> my_logger = ...;
		cmatte::set_logger(my_logger);

		// Now call any functions you wish and they will log to your logger instead.
		cmatte::cryptomatte::load(...);
	};


If you wish to instead modify the existing logger to change the verbosity etc. you can also do this 
via:

.. code-block:: cpp

	#include <cryptomatte/cryptomatte.h>
	#include <cryptomatte/logger.h>


	auto main() -> int
	{
		auto cmatte_logger = cmatte::get_logger();
		cmatte_logger->set_level(spdlog::level::debug);
	};



logging function reference 
****************************


.. doxygenvariable:: cmatte::s_default_logger_name
.. doxygenfunction:: cmatte::set_logger
.. doxygenfunction:: cmatte::get_logger
