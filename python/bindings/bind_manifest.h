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
    py::class_<manifest> manifest_cls(m, "Manifest");

    // Constructors
    manifest_cls
        .def(py::init<>(),
             R"doc(
Construct an empty Manifest.

)doc");

    manifest_cls
        .def_static("from_json",
            [](const nlohmann::json& json_obj)
            {
                json_ordered ordered = json_obj;
                return manifest(ordered);
            },
            py::arg("json_obj"),
            R"doc(
Load and decode a manifest from JSON.

:param json_obj: JSON object containing manifest data.
:returns: Decoded Manifest instance.
)doc");

    // Instance methods
    manifest_cls
        .def("contains",
            &manifest::contains,
            py::arg("name"),
            R"doc(
Check whether the manifest contains the given name.

:param name: Name to check for existence within the manifest.
:returns: True if the name exists, False otherwise.
)doc");

    manifest_cls
        .def("size",
            &manifest::size,
            R"doc(
Get the size of the manifest (number of items).

:returns: Number of items in the mapping.
)doc");

    manifest_cls
        .def("hash_uint32",
            &manifest::hash<uint32_t>,
            py::arg("name"),
            R"doc(
Get the hash associated with the given name as uint32.

:param name: Name whose hash to retrieve.
:returns: Hash value as uint32.
:raises ValueError: If the name does not exist in the manifest.
)doc");

    manifest_cls
        .def("hash_float",
            &manifest::hash<float32_t>,
            py::arg("name"),
            R"doc(
Get the hash associated with the given name as float32.

:param name: Name whose hash to retrieve.
:returns: Hash value as float32.
:raises ValueError: If the name does not exist in the manifest.
)doc");

    manifest_cls
        .def("hash_hex",
            &manifest::hash<std::string>,
            py::arg("name"),
            R"doc(
Get the hash associated with the given name as a hexadecimal string.

:param name: Name whose hash to retrieve.
:returns: Hash value as an 8-character hex string.
:raises ValueError: If the name does not exist in the manifest.
)doc");

    manifest_cls
        .def("hashes_uint32",
            &manifest::hashes<uint32_t>,
            R"doc(
Retrieve all hashes stored in the manifest as uint32 values.

:returns: List of hash values as uint32 in the same order as names().
)doc");

    manifest_cls
        .def("hashes_float",
            &manifest::hashes<float32_t>,
            R"doc(
Retrieve all hashes stored in the manifest as float32 values.

:returns: List of hash values as float32 in the same order as names().
)doc");

    manifest_cls
        .def("hashes_hex",
            &manifest::hashes<std::string>,
            R"doc(
Retrieve all hashes stored in the manifest as hexadecimal strings.

:returns: List of hash values as 8-character hex strings in the same order as names().
)doc");

    manifest_cls
        .def("names",
            &manifest::names,
            R"doc(
Retrieve all names stored in the manifest.

:returns: List of names in the manifest.
)doc");

    manifest_cls
        .def("mapping_uint32",
            &manifest::mapping<uint32_t>,
            R"doc(
Retrieve the full name-to-hash mapping as uint32 values.

:returns: List of (name, hash) pairs with hash as uint32.
)doc");

    manifest_cls
        .def("mapping_float",
            &manifest::mapping<float32_t>,
            R"doc(
Retrieve the full name-to-hash mapping as float32 values.

:returns: List of (name, hash) pairs with hash as float32.
)doc");

    manifest_cls
        .def("mapping_hex",
            &manifest::mapping<std::string>,
            R"doc(
Retrieve the full name-to-hash mapping as hexadecimal strings.

:returns: List of (name, hash) pairs with hash as 8-character hex strings.
)doc");
}