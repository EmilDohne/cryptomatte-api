#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/stl_bind.h>
#include <thirdparty/pybind11_json.hpp>

#include <cryptomatte/metadata.h>
#include <OpenImageIO/imageio.h>  // For OIIO::ImageSpec and ParamValueList

namespace py = pybind11;
using namespace NAMESPACE_CRYPTOMATTE_API;


void bind_metadata(py::module& m)
{
	py::class_<metadata>(m, "Metadata")
		.def(py::init<>())
		.def(py::init<
			std::string,
			std::string,
			std::string,
			std::string,
			std::optional<NAMESPACE_CRYPTOMATTE_API::manifest>
		>(),
			py::arg("name"),
			py::arg("key"),
			py::arg("hash"),
			py::arg("conversion"),
			py::arg("manifest") = std::nullopt)

		// Static methods
		.def_static("from_spec", &metadata::from_spec, py::arg("spec"), py::arg("image_path"))
		.def_static("from_json", &metadata::from_json, py::arg("json"), py::arg("image_path"))

		// Instance methods
		.def("channel_names", &metadata::channel_names, py::arg("channelnames"))
		.def("legacy_channel_names", &metadata::legacy_channel_names, py::arg("channelnames"))
		.def("is_valid_channel_name", &metadata::is_valid_channel_name, py::arg("channel_name"))
		.def("is_valid_legacy_channel_name", &metadata::is_valid_legacy_channel_name, py::arg("channel_name"))
		.def("name", &metadata::name)
		.def("key", &metadata::key)
		.def("hash_method", &metadata::hash_method)
		.def("conversion_method", &metadata::conversion_method)
		.def("manifest", &metadata::manifest)

		// Static constants
		.def_static("attrib_name_identifier", &metadata::attrib_name_identifier)
		.def_static("attrib_hash_method_identifier", &metadata::attrib_hash_method_identifier)
		.def_static("attrib_conversion_method_identifier", &metadata::attrib_conversion_method_identifier)
		.def_static("attrib_manifest_identifier", &metadata::attrib_manifest_identifier)
		.def_static("attrib_manif_file_identifier", &metadata::attrib_manif_file_identifier);

}