#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/stl_bind.h>
#include <pybind11/functional.h>
#include <pybind11/chrono.h>

#include <cryptomatte/cryptomatte.h>

namespace py = pybind11;
using namespace NAMESPACE_CRYPTOMATTE_API;


void bind_cryptomatte(py::module_& m) 
{
    py::class_<cryptomatte>(m, "Cryptomatte")
        // Default constructor and move operations
        .def(py::init<>())
        .def(py::init<
            std::unordered_map<std::string, compressed::channel<float32_t>>,
            const metadata&>(),
            py::arg("channels"), py::arg("metadata"))

        .def(py::init<
            std::unordered_map<std::string, std::vector<float32_t>>,
            size_t, size_t,
            const metadata&>(),
            py::arg("channels"), py::arg("width"), py::arg("height"), py::arg("metadata"))

        // Static method
        .def_static("load", &cryptomatte::load, py::arg("file"), py::arg("load_preview") = false)

        // Dimensions
        .def("width", &cryptomatte::width)
        .def("height", &cryptomatte::height)

        // Preview
        .def("has_preview", &cryptomatte::has_preview)
        .def("preview", &cryptomatte::preview)
        .def("extract_preview_compressed", &cryptomatte::extract_preview_compressed)

        // Masks (by name or hash)
        .def("mask", py::overload_cast<std::string>(&cryptomatte::mask, py::const_), py::arg("name"))
        .def("mask", py::overload_cast<uint32_t>(&cryptomatte::mask, py::const_), py::arg("hash"))
        .def("mask_compressed", py::overload_cast<std::string>(&cryptomatte::mask_compressed, py::const_), py::arg("name"))
        .def("mask_compressed", py::overload_cast<uint32_t>(&cryptomatte::mask_compressed, py::const_), py::arg("hash"))

        .def("masks", py::overload_cast<std::vector<std::string>>(&cryptomatte::masks), py::arg("names"))
        .def("masks", py::overload_cast<std::vector<uint32_t>>(&cryptomatte::masks), py::arg("hashes"))
        .def("masks", py::overload_cast<bool>(&cryptomatte::masks, py::const_), py::arg("use_manifest"))

        .def("masks_compressed", py::overload_cast<std::vector<std::string>>(&cryptomatte::masks_compressed), py::arg("names"))
        .def("masks_compressed", py::overload_cast<std::vector<uint32_t>>(&cryptomatte::masks_compressed), py::arg("hashes"))
        .def("masks_compressed", py::overload_cast<>(&cryptomatte::masks_compressed, py::const_))

        .def("num_levels", &cryptomatte::num_levels)

        // Metadata (returning reference)
        .def("metadata", py::overload_cast<>(&cryptomatte::metadata), py::return_value_policy::reference_internal)
        .def("metadata", py::overload_cast<>(&cryptomatte::metadata, py::const_), py::return_value_policy::reference_internal);
}