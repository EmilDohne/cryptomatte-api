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
    py::class_<metadata> metadata_cls(m, "Metadata");

    // Constructors
    metadata_cls
        .def(py::init<>(), R"pbdoc(
Default constructor.

)pbdoc")
        .def(py::init<
            std::string,
            std::string,
            std::string,
            std::string,
            std::optional<NAMESPACE_CRYPTOMATTE_API::manifest>>(),
            py::arg("name"),
            py::arg("key"),
            py::arg("hash"),
            py::arg("conversion"),
            py::arg("manifest") = std::nullopt,
            R"pbdoc(
Construct a cryptomatte metadata object.

:param name: Cryptomatte type name.
:param key: Unique identifier within the file.
:param hash: Hashing method used. Must be 'MurmurHash3_32'.
:param conversion: Conversion method. Must be 'uint32_to_float32'.
:param manifest: Optional manifest for label-to-ID mapping.
)pbdoc");

    // Static methods
    metadata_cls
        .def_static("from_filepath",
            [](std::filesystem::path filepath) {
                auto input_ptr = OIIO::ImageInput::open(filepath);
                if (!input_ptr) {
                    throw std::runtime_error(
                        std::format("Failed to open file {}.", filepath.string())
                    );
                }
                auto& spec = input_ptr->spec();
                return metadata::from_spec(spec, filepath);
            },
            py::arg("filepath"),
            R"pbdoc(
Load and parse cryptomatte metadata from an image file.

:param filepath: Path to the image file.
:returns: List of Metadata objects parsed from the file.
)pbdoc")
        .def_static("from_json",
            &metadata::from_json,
            py::arg("json"),
            py::arg("image_path"),
            R"pbdoc(
Parse metadata from JSON representation.

:param json: Ordered JSON data representing the metadata.
:param image_path: Path to the source image (used to locate sidecar manifests).
:returns: List of Metadata objects parsed from the JSON.
)pbdoc");

    // Instance methods
    metadata_cls
        .def("channel_names",
            &metadata::channel_names,
            py::arg("channelnames"),
            R"pbdoc(
Retrieve all valid cryptomatte channel names.

:param channelnames: List of channel names to filter.
:returns: Valid cryptomatte channel names.
)pbdoc")
        .def("legacy_channel_names",
            &metadata::legacy_channel_names,
            py::arg("channelnames"),
            R"pbdoc(
Retrieve all valid legacy-style cryptomatte channel names.

:param channelnames: List of channel names to filter.
:returns: Valid legacy cryptomatte channel names.
)pbdoc")
        .def("is_valid_channel_name",
            &metadata::is_valid_channel_name,
            py::arg("channel_name"),
            R"pbdoc(
Check if a channel name is a valid cryptomatte channel name (non-legacy).

:param channel_name: The name to match against.
:returns: True if valid, False otherwise.
)pbdoc")
        .def("is_valid_legacy_channel_name",
            &metadata::is_valid_legacy_channel_name,
            py::arg("channel_name"),
            R"pbdoc(
Check if a channel name is a valid legacy cryptomatte channel name.

:param channel_name: The name to match against.
:returns: True if valid, False otherwise.
)pbdoc")
        .def("name",
            &metadata::name,
            R"pbdoc(
Get the name (type identifier) of the cryptomatte.

:returns: Cryptomatte name.
)pbdoc")
        .def("key",
            &metadata::key,
            R"pbdoc(
Get the unique key identifying this cryptomatte.

:returns: Unique key.
)pbdoc")
        .def("hash_method",
            &metadata::hash_method,
            R"pbdoc(
Get the hashing method used (always 'MurmurHash3_32').

:returns: Hash method.
)pbdoc")
        .def("conversion_method",
            &metadata::conversion_method,
            R"pbdoc(
Get the pixel value conversion method (always 'uint32_to_float32').

:returns: Conversion method.
)pbdoc")
        .def("manifest",
            &metadata::manifest,
            R"pbdoc(
Get the optional manifest mapping names to hash IDs.

:returns: The manifest or None.
)pbdoc");

    // Static attribute identifiers
    metadata_cls
        .def_static("attrib_name_identifier",
            &metadata::attrib_name_identifier,
            R"pbdoc(
Return the attribute name identifier ('name').

:returns: Identifier string for 'name' attribute.
)pbdoc")
        .def_static("attrib_hash_method_identifier",
            &metadata::attrib_hash_method_identifier,
            R"pbdoc(
Return the attribute hash method identifier ('hash').

:returns: Identifier string for 'hash' attribute.
)pbdoc")
        .def_static("attrib_conversion_method_identifier",
            &metadata::attrib_conversion_method_identifier,
            R"pbdoc(
Return the attribute conversion method identifier ('conversion').

:returns: Identifier string for 'conversion' attribute.
)pbdoc")
        .def_static("attrib_manifest_identifier",
            &metadata::attrib_manifest_identifier,
            R"pbdoc(
Return the attribute manifest identifier ('manifest').

:returns: Identifier string for 'manifest' attribute.
)pbdoc")
        .def_static("attrib_manif_file_identifier",
            &metadata::attrib_manif_file_identifier,
            R"pbdoc(
Return the attribute manifest file identifier ('manif_file').

:returns: Identifier string for 'manif_file' attribute.
)pbdoc");
}