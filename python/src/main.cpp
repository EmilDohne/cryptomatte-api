#include <pybind11/pybind11.h>

#include "bind_manifest.h"
#include "bind_metadata.h"

namespace py = pybind11;


PYBIND11_MODULE(cryptomatte_api, m)
{
	bind_metadata(m);
	bind_manifest(m);
}