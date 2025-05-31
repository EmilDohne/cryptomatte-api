#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/stl_bind.h>
#include <pybind11/functional.h>
#include <thirdparty/pybind11_json.hpp>

#include <cryptomatte/manifest.h>

namespace py = pybind11;
using namespace NAMESPACE_CRYPTOMATTE_API;


void bind_manifest(py::module& m)
{
	py::class_<manifest>(m, "Manifest")
		.def(py::init<>())

		// Expose a constructor that takes regular json and converts it since pybind11_json does not expose ordered
		// json but only json. 
		.def_static("from_json", [](const nlohmann::json& json_obj) 
			{
				json_ordered ordered = json_obj;
				return manifest(ordered);
			})

		// Same for hash accessors and mapping
		.def("contains", &manifest::contains)
		.def("size", &manifest::size)

		.def("hash_uint32", &manifest::hash<uint32_t>)
		.def("hash_float", &manifest::hash<float32_t>)
		.def("hash_hex", &manifest::hash<std::string>)

		.def("mapping_uint32", &manifest::mapping<uint32_t>)
		.def("mapping_float", &manifest::mapping<float32_t>)
		.def("mapping_hex", &manifest::mapping<std::string>)
}