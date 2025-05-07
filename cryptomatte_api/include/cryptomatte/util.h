#pragma once

#include "detail/macros.h"

#include <OpenImageIO/imageio.h>


namespace NAMESPACE_CRYPTOMATTE_API
{

	/// \brief Check whether the passed file contains the cryptomatte preview/legacy channels.
	/// 
	/// If you wish to check multiple things on the cryptomatte opening the spec yourself and using the 
	/// OIIO::ImageSpec overload of this method is likely faster as accessing the spec has a not insignificant
	/// overhead.
	/// 
	/// \param file The path to the file to inspect. Must be a .exr file
	bool has_preview(std::filesystem::path file);

	/// \brief Check whether the passed file contains the cryptomatte preview/legacy channels.
	/// 
	/// \param spec The OpenImageIO spec of the opened file.
	bool has_preview(const OIIO::ImageSpec& file);

	/// \brief Check whether the passed file contains a cryptomatte.
	/// 
	/// If you wish to check multiple things on the cryptomatte opening the spec yourself and using the 
	/// OIIO::ImageSpec overload of this method is likely faster as accessing the spec has a not insignificant
	/// overhead.
	/// 
	/// \param file The path to the file to inspect. Must be a .exr file
	bool has_cryptomatte(std::filesystem::path file);

	/// \brief Check whether the passed openimageio spec contains a cryptomatte.
	/// 
	/// \param spec The OpenImageIO spec of the opened file.
	bool has_cryptomatte(const OIIO::ImageSpec& file);

	/// \brief Get the number of cryptomattes stored on the given file
	/// 
	/// If you wish to check multiple things on the cryptomatte opening the spec yourself and using the 
	/// OIIO::ImageSpec overload of this method is likely faster as accessing the spec has a not insignificant
	/// overhead.
	/// 
	/// \param file The path to the file to inspect. Must be a .exr file
	size_t num_cryptomattes(std::filesystem::path file);

	/// \brief Get the number of cryptomattes stored on the given openimageio spec
	/// 
	/// \param spec The OpenImageIO spec of the opened file.
	size_t num_cryptomattes(const OIIO::ImageSpec& file);

	/// \brief Get all the cryptomatte channels of a given file
	/// 
	/// Inspects all the channels of the passed file and returns a nested vector containing the channel names
	/// for a given cryptomatte. In the case of multiple cryptomattes the return could therefore look as follows:
	/// {
	///		{ CryptoAsset.r, CryptoAsset.g, CryptoAsset.b, CryptoAsset00.r, CryptoAsset00.g, CryptoAsset00.b, CryptoAsset00.a }
	///		{ CryptoMat.r, CryptoMat.g, CryptoMat.b, CryptoMat.r, CryptoMat.g, CryptoMat.b, CryptoMat.a }
	/// }
	/// 
	/// This may also return an empty vector in which case no cryptomatte is present.
	/// 
	/// If you wish to check multiple things on the cryptomatte opening the spec yourself and using the 
	/// OIIO::ImageSpec overload of this method is likely faster as accessing the spec has a not insignificant
	/// overhead.
	/// 
	/// \param file The path to the file to inspect. Must be a .exr file
	std::vector<std::vector<std::string>> channel_names(std::filesystem::path file);

	/// \brief Get all the cryptomatte channels of a given file
	/// 
	/// Inspects all the channels of the passed file and returns a nested vector containing the channel names
	/// for a given cryptomatte. In the case of multiple cryptomattes the return could therefore look as follows:
	/// {
	///		{ CryptoAsset.r, CryptoAsset.g, CryptoAsset.b, CryptoAsset00.r, CryptoAsset00.g, CryptoAsset00.b, CryptoAsset00.a }
	///		{ CryptoMat.r, CryptoMat.g, CryptoMat.b, CryptoMat.r, CryptoMat.g, CryptoMat.b, CryptoMat.a }
	/// }
	/// 
	/// This may also return an empty vector in which case no cryptomatte is present.
	/// 
	/// \param spec The OpenImageIO spec of the opened file.
	std::vector<std::vector<std::string>> channel_names(const OIIO::ImageSpec& file);	

	/// \brief Inspects the passed cryptomatte file and validates its metadata.
	/// 
	/// This will check whether all the required metadata is present but does not 
	/// check whether the channels are correct.
	/// 
	/// If you wish to check multiple things on the cryptomatte opening the spec yourself and using the 
	/// OIIO::ImageSpec overload of this method is likely faster as accessing the spec has a not insignificant
	/// overhead.
	/// 
	/// \param file The path to the file to inspect. Must be a .exr file
	///
	/// \returns a tuple containing whether it is valid, as well as an optional error message.
	std::tuple<bool, std::string> validate_metadata(std::filesystem::path file);

	/// \brief Inspects the passed cryptomatte openimageio spec and validates its metadata.
	/// 
	/// This will check whether all the required metadata is present but does not 
	/// check whether the channels are correct.
	/// 
	/// \param spec The OpenImageIO spec of the opened file.
	///
	/// \returns a tuple containing whether it is valid, as well as an optional error message.
	std::tuple<bool, std::string> validate_metadata(const OIIO::ImageSpec& spec);

	/// \brief Inspects the passed cryptomatte file and validates its channel structure
	/// 
	/// This will check whether the channel names and number of channels are conformant with the cryptomatte
	/// specification.
	/// 
	/// If you wish to check multiple things on the cryptomatte opening the spec yourself and using the 
	/// OIIO::ImageSpec overload of this method is likely faster as accessing the spec has a not insignificant
	/// overhead.
	/// 
	/// \param file The path to the file to inspect. Must be a .exr file
	///
	/// \returns a tuple containing whether it is valid, as well as an optional error message.
	std::tuple<bool, std::string> validate_channel_structure(std::filesystem::path file);

	/// \brief Inspects the passed cryptomatte openimageio spec and validates its channel structure
	/// 
	/// This will check whether the channel names and number of channels are conformant with the cryptomatte
	/// specification.
	/// 
	/// \param spec The OpenImageIO spec of the opened file.
	/// 
	/// \returns a tuple containing whether it is valid, as well as an optional error message.
	std::tuple<bool, std::string> validate_channel_structure(const OIIO::ImageSpec& spec);

	/// \brief Inspects the passed cryptomatte file and validates the metadata and channel structure
	/// 
	/// Internally calls both `validate_channel_structure` and `validate_metadata`.
	/// 
	/// If you wish to check multiple things on the cryptomatte opening the spec yourself and using the 
	/// OIIO::ImageSpec overload of this method is likely faster as accessing the spec has a not insignificant
	/// overhead.
	/// 
	/// \param file The path to the file to inspect. Must be a .exr file
	/// \returns a tuple containing whether it is valid, as well as an optional error message.
	std::tuple<bool, std::string> validate(std::filesystem::path file);

	/// \brief Inspects the passed cryptomatte openimageio spec and validates the metadata and channel structure
	/// 
	/// Internally calls both `validate_channel_structure` and `validate_metadata`.
	/// 
	/// \param file The path to the file to inspect. Must be a .exr file
	/// \returns a tuple containing whether it is valid, as well as an optional error message.
	std::tuple<bool, std::string> validate(const OIIO::ImageSpec& file);

}