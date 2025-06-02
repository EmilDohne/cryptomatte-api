#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/stl_bind.h>
#include <pybind11/functional.h>
#include <pybind11/chrono.h>

#include <py_img_util/image.h>

#include <cryptomatte/cryptomatte.h>

namespace py = pybind11;
using namespace NAMESPACE_CRYPTOMATTE_API;


void bind_cryptomatte(py::module_& m)
{
    py::class_<cryptomatte> crypto_class(m, "Cryptomatte", R"doc(

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
        .def(py::init<
                std::unordered_map<std::string, compressed::channel<float32_t>>,
                const metadata&>(),
            py::arg("channels"),
            py::arg("metadata"),
            R"doc(
Construct a Cryptomatte from compressed float32 channels.

:param channels: Mapping of channel names to compressed float32 image channels.
:param metadata: Metadata used to validate and classify the provided channels.
:raises ValueError: If the channel map is empty or any cryptomatte channel has inconsistent encoding.
            )doc");

    crypto_class
        .def(
            py::init(
                [](
                    std::unordered_map<std::string, py::array_t<float32_t>>& channels,
                    size_t width,
                    size_t height,
                    const metadata& metadata
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
                    return std::make_unique<cryptomatte>(
                        converted_channels, width, height, metadata);
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
                return py_img_util::to_py_array(
                    std::move(result),
                    self.width(),
                    self.height()
                );
            },
            R"doc(
Return the preview (legacy) image channels as float32 numpy arrays.

:returns: List of numpy arrays representing preview channels.
)doc"
        );

    //----------------------------------------------------------------------------//
    // Mask Generation (Raw and Compressed)                                       //
    //----------------------------------------------------------------------------//

    // Single-mask by name (raw array)
    crypto_class
        .def(
            "mask",
            [](cryptomatte& self, const std::string& name)
            {
                auto result = self.mask(name);
                return py_img_util::to_py_array(
                    std::move(result),
                    self.width(),
                    self.height()
                );
            },
            py::arg("name"),
            R"doc(
Compute and return a decoded mask for the given object name.

:param name: Name from the manifest.
:returns: Float32 array mask.
)doc"
        );

    // Single-mask by hash (raw array)
    crypto_class
        .def(
            "mask",
            [](cryptomatte& self, uint32_t hash)
            {
                auto result = self.mask(hash);
                return py_img_util::to_py_array(
                    std::move(result),
                    self.width(),
                    self.height()
                );
            },
            py::arg("hash"),
            R"doc(
Compute and return a decoded mask for the given object hash.

:param hash: Pixel hash.
:returns: Float32 array mask.
)doc"
        );

    // Single-mask (compressed) by name
    crypto_class
        .def(
            "mask_compressed",
            py::overload_cast<std::string>(&cryptomatte::mask_compressed, py::const_),
            py::arg("name"),
            R"doc(
Compute and return a compressed mask for the given object name.

:param name: Object name.
:returns: Compressed mask channel.
)doc"
        );

    // Single-mask (compressed) by hash
    crypto_class
        .def(
            "mask_compressed",
            py::overload_cast<uint32_t>(&cryptomatte::mask_compressed, py::const_),
            py::arg("hash"),
            R"doc(
Compute and return a compressed mask for the given object hash.

:param hash: Object hash.
:returns: Compressed mask channel.
)doc"
        );

    // Multi-mask (raw arrays) by names
    crypto_class
        .def(
            "masks",
            [](cryptomatte& self, const std::vector<std::string>& names)
            {
                auto _result = self.masks(names);
                std::unordered_map<std::string, py::array_t<float32_t>> out;
                for (auto& [key, value] : _result)
                {
                    out[key] = py_img_util::to_py_array(
                        std::move(value),
                        self.width(),
                        self.height()
                    );
                }
                return out;
            },
            py::arg("names"),
            R"doc(
Compute multiple masks by name.

:param names: List of object names.
:returns: Dictionary of decoded masks.
)doc"
        );

    // Multi-mask (raw arrays) by hashes
    crypto_class
        .def(
            "masks",
            [](cryptomatte& self, const std::vector<uint32_t>& hashes)
            {
                auto _result = self.masks(hashes);
                std::unordered_map<std::string, py::array_t<float32_t>> out;
                for (auto& [key, value] : _result)
                {
                    out[key] = py_img_util::to_py_array(
                        std::move(value),
                        self.width(),
                        self.height()
                    );
                }
                return out;
            },
            py::arg("hashes"),
            R"doc(
Compute multiple masks by hash.

:param hashes: List of object hashes.
:returns: Dictionary of decoded masks.
)doc"
        );

    // Multi-mask (raw arrays) for all masks
    crypto_class
        .def(
            "masks",
            [](cryptomatte& self)
            {
                auto _result = self.masks();
                std::unordered_map<std::string, py::array_t<float32_t>> out;
                for (auto& [key, value] : _result)
                {
                    out[key] = py_img_util::to_py_array(
                        std::move(value),
                        self.width(),
                        self.height()
                    );
                }
                return out;
            },
            R"doc(
Compute and return all decoded masks.

:returns: Dictionary of decoded masks.
)doc"
        );

    // Multi-mask (compressed) by names
    crypto_class
        .def(
            "masks_compressed",
            py::overload_cast<std::vector<std::string>>(&cryptomatte::masks_compressed, py::const_),
            py::arg("names"),
            R"doc(
Compute multiple compressed masks by name.

:param names: List of object names.
:returns: Dictionary of compressed masks.
)doc"
        );

    // Multi-mask (compressed) by hashes
    crypto_class
        .def(
            "masks_compressed",
            py::overload_cast<std::vector<uint32_t>>(&cryptomatte::masks_compressed, py::const_),
            py::arg("hashes"),
            R"doc(
Compute multiple compressed masks by hash.

:param hashes: List of object hashes.
:returns: Dictionary of compressed masks.
)doc"
        );

    // Multi-mask (compressed) for all masks
    crypto_class
        .def(
            "masks_compressed",
            py::overload_cast<>(&cryptomatte::masks_compressed, py::const_),
            R"doc(
Compute and return all compressed masks.

:returns: Dictionary of compressed masks.
)doc"
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