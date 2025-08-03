#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/stl_bind.h>
#include <pybind11/functional.h>
#include <pybind11/chrono.h>
#include <pybind11/stl/filesystem.h>

#include <py_img_util/image.h>

#include <cryptomatte/cryptomatte.h>

namespace py = pybind11;
using namespace NAMESPACE_CRYPTOMATTE_API;


void bind_cryptomatte(py::module_& m)
{
    py::class_<cryptomatte, std::shared_ptr<cryptomatte>> crypto_class(m, "Cryptomatte", R"doc(

A cryptomatte file loaded from disk or memory storing the channels as compressed buffer

)doc");

    //----------------------------------------------------------------------------//
    // Constructors                                                              //
    //----------------------------------------------------------------------------//

    crypto_class
        .def(py::init<>(), R"doc(
Construct an empty Cryptomatte object.
)doc");

    crypto_class
        .def(
            py::init(
                [](
                    std::unordered_map<std::string, py::array_t<float32_t>>& channels,
                    size_t width,
                    size_t height,
                    metadata& metadata
                )
                {
                    std::unordered_map<std::string, std::vector<float32_t>> converted_channels;
                    for (auto& [key, channel] : channels)
                    {
                        converted_channels[key] = py_img_util::from_py_array(
                            py_img_util::tag::vector{},
                            channel,
                            width,
                            height);
                    }
                    return std::make_unique<cryptomatte>(converted_channels, width, height, metadata);
                }
            ),
            py::arg("channels"),
            py::arg("width"),
            py::arg("height"),
            py::arg("metadata"),
            R"doc(
Construct a Cryptomatte from raw float32 image arrays.

:param channels: Mapping of channel names to float32 image arrays. Each array must be shaped (height, width).
:param width: Image width.
:param height: Image height.
:param metadata: Metadata used to validate and classify the provided channels.
:raises ValueError: If the channel map is empty or any array has mismatched shape.
)doc"
        );

    //----------------------------------------------------------------------------//
    // Static Methods                                                            //
    //----------------------------------------------------------------------------//

    crypto_class
        .def_static(
            "load",
            &cryptomatte::load,
            py::arg("file"),
            py::arg("load_preview") = false,
            R"doc(
Load cryptomatte(s) from an EXR file.

:param file: Path to an EXR file containing cryptomatte channels.
:param load_preview: Whether to load the legacy preview channels (.r/.g/.b).
:returns: List of loaded Cryptomatte instances.
)doc"
        );

    //----------------------------------------------------------------------------//
    // Dimension Accessors                                                        //
    //----------------------------------------------------------------------------//

    crypto_class
        .def(
            "width",
            &cryptomatte::width,
             R"doc(
Return the width of the cryptomatte in pixels.

:returns: Width of the cryptomatte.
)doc"
        );

    crypto_class
        .def(
            "height",
            &cryptomatte::height,
            R"doc(
Return the height of the cryptomatte in pixels.

:returns: Height of the cryptomatte.
)doc"
        );

    //----------------------------------------------------------------------------//
    // Preview Methods                                                            //
    //----------------------------------------------------------------------------//

    crypto_class
        .def(
            "has_preview",
            &cryptomatte::has_preview,
            R"doc(
Return whether preview (legacy .r/.g/.b) channels are available and loaded.

:returns: True if preview channels are available, False otherwise.
)doc"
        );

    crypto_class
        .def(
            "preview",
            [](cryptomatte& self)
            {
                auto result = self.preview();
                std::vector<py::array_t<float32_t>> out;
                for (auto& channel : result)
                {
                    out.push_back(
                        py_img_util::to_py_array(
                            std::move(channel),
                            self.width(),
                            self.height()
                        )
                    );
                }
                return out;
            },
            R"doc(
Return the preview (legacy) image channels as float32 numpy arrays. This list may be empty if the cryptomatte was loaded
without `load_preview` enabled.

:returns: List of numpy arrays representing preview channels, may be empty
)doc"
        );

    //----------------------------------------------------------------------------//
    // Mask Generation (Raw and Compressed)                                       //
    //----------------------------------------------------------------------------//

    // Single-mask by name (raw array)
    crypto_class
        .def(
            "mask",
            [](cryptomatte& self, std::variant<std::string, uint32_t> name_or_hash)
            {
                if (std::holds_alternative<std::string>(name_or_hash))
                {
                    auto result = self.mask(std::get<std::string>(name_or_hash));
                    return py_img_util::to_py_array(
                        std::move(result),
                        self.width(),
                        self.height()
                    );
                }
                auto result = self.mask(std::get<uint32_t>(name_or_hash));
                return py_img_util::to_py_array(
                    std::move(result),
                    self.width(),
                    self.height()
                );
            },
            py::arg("name_or_hash"),
            R"doc(
Compute and return a decoded mask for the given object name.

:param name: Name from the manifest.
:returns: np.float32 array mask.
)doc"
        );

    // Multi-mask (raw arrays) by names
    crypto_class
        .def(
            "masks",
            [](cryptomatte& self, std::optional<std::variant<std::vector<std::string>, std::vector<uint32_t>>> names_or_hashes)
            {
                // This function essentially wraps 3 unique functions in cpp into a single one for ease of use and a more
                // pythonic interface. We take an optional list of strings or integers and dispatch to the right function
                // accordingly
                std::unordered_map<std::string, std::vector<float32_t>> result_from_cpp;
                if (names_or_hashes)
                {
                    auto& names_or_hashes_val = names_or_hashes.value();
                    if (std::holds_alternative<std::vector<std::string>>(names_or_hashes_val))
                    {
                        result_from_cpp = self.masks(std::get<std::vector<std::string>>(names_or_hashes_val));
                    }
                    else
                    {
                        result_from_cpp = self.masks(std::get<std::vector<uint32_t>>(names_or_hashes_val));
                    }
                }
                else
                {
                    result_from_cpp = self.masks();
                }

                // Now finally, convert from the cpp types into python numpy arrays (mapped by string)
                std::unordered_map<std::string, py::array_t<float32_t>> out;
                for (auto& [key, value] : result_from_cpp)
                {
                    out[key] = py_img_util::to_py_array(
                        std::move(value),
                        self.width(),
                        self.height()
                    );
                }
                return out;
            },
            py::arg("names_or_hashes") = py::none()
        );

    //----------------------------------------------------------------------------//
    // Metadata and Levels                                                         //
    //----------------------------------------------------------------------------//

    crypto_class
        .def(
            "num_levels",
            &cryptomatte::num_levels,
            R"doc(
Return the number of encoded rank-coverage levels.

:returns: Number of levels.
)doc"
        );

    crypto_class
        .def(
            "metadata",
            py::overload_cast<>(&cryptomatte::metadata),
            py::return_value_policy::reference_internal,
            R"doc(
Return the metadata associated with this cryptomatte.

:returns: Metadata object.
)doc"
        );
}