#pragma once

#include <string>
#include <string_view>

#include "detail/macros.h"

#include "metadata.h"
#include "manifest.h"

#include <compressed/channel.h>
#include <OpenImageIO/imageio.h>
#include <json.hpp>

namespace NAMESPACE_CRYPTOMATTE_API
{
	// string_view literals for ""sv
	using namespace std::string_view_literals;

	/// @brief A cryptomatte file loaded from disk or memory storing the channels as compressed buffer
	/// 
	/// This is the main entry point for loading and interacting with cryptomatte files and their associated data.
	/// It fully supports loading any number of cryptomattes from a given file and performs validation on the metadata,
	/// channels and masks to ensure correctness.
	/// 
	/// Cryptomattes are a special image type that stores ID masks with support for transparency. To do this it stores
	/// alternating `rank` and `coverage` channels where the `rank` channel is the ID of the pixel and the `coverage` 
	/// channel is an additive value describing how much of that mask is stored in this rank-coverage pair.
	/// 
	/// This is to allow for transparent pixels by storing only one mask per pixel per rank-coverage pair and then storing
	/// further ids that lie on the same pixel in another rank-coverage pair. The cryptomatte channels are stored
	/// as follows:
	/// 
	/// {typename}.r    - deprecated -> may be skipped
	/// {typename}.g    - deprecated (may contain filtered preview colors) -> may be skipped
	/// {typename}.b    - deprecated (may contain filtered preview colors) -> may be skipped
	/// {typename}00.r  - id rank channel 0
	/// {typename}00.g  - coverage channel 0
	/// {typename}00.b  - id rank channel 1
	/// {typename}00.a  - coverage channel 1
	/// {typename}01.r  - id rank channel 2
	/// {typename}01.g  - coverage channel 2
	/// {typename}01.b  - id rank channel 3
	/// {typename}01.a  - coverage channel 3
	/// ...
	/// 
	/// Here typename can be anything, but usually it will be something along the lines of `Cryptomatte`, `CryptoAsset`,
	/// `CryptoMaterial` etc. Due to this cascading rank system you will typically find most data on the first rank-coverage
	/// pairs with the further pairs being used to resolve intersections on transparent pixels with diminishing returns. 
	/// 
	/// In DCCs you will often find a setting allowing you to control the numer of `levels`, this is the number of
	/// rank-coverage pairs and dictates how many objects may overlap on a single pixel.
	/// 
	/// The `cryptomatte::file` struct loads the above mentioned channels and performs mask computation on the fly. 
	/// It allows you to either target a specific mask, multiple masks or all masks using the `mask`, `masks` or 
	/// `mask_compressed` and `masks_compressed` functions (more on those later). This has the benefit of only loading
	/// the masks you need into memory, skipping over the rest. 
	/// 
	/// As a rule of thumb however, you should always try to extract as many masks in one go as you need, as for each
	/// mask query we need to iterate all of the above channels.
	/// 
	/// **Compression**
	/// 
	/// This Cryptomatte implementation uses in-memory compression for the channels, and optionally also compresses the
	/// masks as they are being extracted. This is primarily to greatly reduce memory usage as masks typically compress
	/// quite well, but also for performance reasons (less allocations -> greater speed). For specifics please check out
	/// the benchmarking section in the documentation.
	struct cryptomatte
	{

		cryptomatte() = default;
		cryptomatte(cryptomatte&&) = default;
		cryptomatte& operator=(cryptomatte&&) = default;
		// delete copy ctor and copy assignment operator as compressed::channel is not copyable.
		cryptomatte(const cryptomatte&) = delete;
		cryptomatte& operator=(const cryptomatte&) = delete;

		cryptomatte(
			std::unordered_map<std::string, compressed::channel<float32_t>> channels, 
			const NAMESPACE_CRYPTOMATTE_API::metadata metadata
		);

		cryptomatte(
			std::unordered_map<std::string, std::vector<float32_t>> channels, 
			const json_ordered& metadata
		);

		/// \brief Load a file containing cryptomattes into multiple cryptomattes.
		/// 
		/// \param file The file path to load the image from. This must be an exr file.
		/// \param load_preview Whether to load the legacy preview channels:
		///                     {typename}.r, {typename}.g, {typename}.b
		///						which may store a preview channel (but don't have to). If this is set to false
		///						we will never load these channels speeding up loading.
		/// 
		/// \returns The detected and loaded cryptomattes, there may be multiple or none per-file.
		static std::vector<file> load(std::filesystem::path file, bool load_preview);

		/// \brief Checks whether this cryptomatte contains the preview (legacy) channels
		/// 
		/// These are classified by the specification to be the {typename}.r, {typename}.g, {typename}.b
		/// (as opposed to {typename}00.r etc. for the actual cryptomatte channels). These are legacy but often 
		/// used to store a preview of all the mattes.
		/// 
		/// This function checks that these channels are present and loaded (the loading of them can be controlled
		/// via the `load_preview` flag of `load()`). To find out if a file contains them in the first place one can
		/// use the static versions `cryptomatte::has_preview`
		/// 
		/// \returns Whether the preview channels exist (and are loaded) 
		bool has_preview() const;

		/// \brief Returns a vector of the preview (legacy) channels
		/// 
		/// These may not always be available or loaded so the function can return either 0 or 3 channels. These channels
		/// are the {typename}.r, {typename}.g, {typename}.b channels and may store a filtered preview image of all the 
		/// mattes but these channels should not be used for computing masks. Please use the `masks` or `masks_compressed`
		/// functions for this instead.
		/// 
		/// \return The legacy/preview channels (if present)
		std::vector<std::vector<float32_t>> preview() const;

		/// \brief Extracts the legacy/preview channels from the `cryptomatte` instance. 
		/// 
		/// These may not always be available or loaded so the function can return either 0 or 3 channels. These channels
		/// are the {typename}.r, {typename}.g, {typename}.b channels and may store a filtered preview image of all the 
		/// mattes but these channels should not be used for computing masks. Please use the `masks` or `masks_compressed`
		/// functions for this instead.
		/// 
		/// This overloads returns the channels in their compressed format which is ideal if you wish to operate on the 
		/// compressed channels directly rather than paying the memory cost of decompressing them. After these have been
		/// extracted they are entirely in your control and extracting them again is not possible.
		/// 
		/// \return The legacy/preview channels (if present)
		std::unordered_map<std::string, compressed::channel<float32_t>> extract_compressed();

		/// \brief Extract the mask with the given name from the cryptomatte, computing the pixels as we go.
		/// 
		/// This function assumes that a valid cryptomatte manifest exists, if it doesn't/or the name is not known to us
		/// this function will throw a std::invalid_argument.
		/// 
		/// Note:
		///		For performance reasons it is often be desirable to extract as many masks as you need in one go rather
		///		than extracting them individually.
		/// 
		/// \param name The name as it is stored in the manifest, could e.g. be 'bunny1'
		/// 
		/// \returns The decoded cryptomatte mask
		std::vector<float32_t> mask(std::string name) const;

		/// \brief Extract the mask with the given hash from the cryptomatte, computing the pixels as we go.
		/// 
		/// The hash here is the pixel hash of the mask you wish to extract. If the hash could not be found this
		/// function will return an empty mask (black)
		/// 
		/// Note:
		///		For performance reasons it is often be desirable to extract as many masks as you need in one go rather
		///		than extracting them individually.
		/// 
		/// \param hash The hash of the mask
		/// 
		/// \returns The decoded cryptomatte mask
		std::vector<float32_t> mask(uint32_t hash) const;

		/// \brief Extract the mask with the given name from the cryptomatte as compressed channel, computing on the fly.
		/// 
		/// This function assumes that a valid cryptomatte manifest exists, if it doesn't/or the name is not known to us
		/// this function will throw a std::invalid_argument.
		/// 
		/// Note:
		///		For performance reasons it is often be desirable to extract as many masks as you need in one go rather
		///		than extracting them individually.
		/// 
		/// \param name The name as it is stored in the manifest, could e.g. be 'bunny1'
		/// 
		/// \returns The decoded cryptomatte mask
		compressed::channel<float32_t> mask_compressed(std::string name) const;

		/// \brief Extract the mask with the given hash from the cryptomatte as compressed channel, computing on the fly.
		/// 
		/// The hash here is the pixel hash of the mask you wish to extract. If the hash could not be found this
		/// function will return an empty mask (black)
		/// 
		/// Note:
		///		For performance reasons it is often be desirable to extract as many masks as you need in one go rather
		///		than extracting them individually.
		/// 
		/// \param hash The hash of the mask
		/// 
		/// \returns The decoded cryptomatte mask
		compressed::channel<float32_t> mask_compressed(uint32_t hash) const;

		/// \brief Extract the masks with the given names from the cryptomatte, computing on the fly.
		/// 
		/// This function assumes that a valid cryptomatte manifest exists, if it doesn't/or the names are not known to us
		/// this function will throw a std::invalid_argument.
		/// 
		/// Note:
		///		For performance reasons it is often be desirable to extract as many masks as you need in one go rather
		///		than extracting them individually.
		/// 
		/// \param names The names to extract, could e.g. be {'bunny1', 'car', ...}
		/// 
		/// \returns The decoded cryptomattes mapped by their name
		std::unordered_map<std::string, std::vector<float32_t>> masks(std::vector<std::string> names);

		/// \brief Extract the masks with the given names from the cryptomatte, computing on the fly.
		/// 
		/// The hashes here are the pixel hashes of the masks you wish to extract. If a hash could not be found that mask
		/// will be skipped.
		/// 
		/// Note:
		///		For performance reasons it is often be desirable to extract as many masks as you need in one go rather
		///		than extracting them individually.
		/// 
		/// \param names The hashes to extract, any non-valid hashes will be skipped and will also not appear in the output
		/// 
		/// \returns The decoded cryptomattes mapped by their name (if the manifest exists) or by their hashes in std::string
		///			 form.
		std::unordered_map<std::string, std::vector<float32_t>> masks(std::vector<uint32_t> hashes);

		/// \brief Extract all of the cryptomatte masks computing them on the fly
		/// 
		/// \param use_manifest Whether to use the manifest to preallocate the correct number of items before the hot loop.
		///						This allows for quite some optimizations so whenever possible this should be turned on.
		///						It may however throw a std::out_of_range if the manifest is faulty or incomplete.
		///						Therefore, often it is desirable to wrap this call in a try; catch falling back to the
		///						potentially slower, non-manifest based decoding. If no cryptomatte manifest exists,
		///						and this is true this function will run as if `use_manifest` was off.
		/// 
		/// \returns The decoded cryptomattes mapped by their name (if the manifest exists) or by their hashes in std::string
		///			 form.
		std::unordered_map<std::string, std::vector<float32_t>> masks(bool use_manifest) const;

		/// \brief Extract the masks with the given names from the cryptomatte into compressed buffers, computing on the fly.
		/// 
		/// This function assumes that a valid cryptomatte manifest exists, if it doesn't/or the names are not known to us
		/// this function will throw a std::invalid_argument.
		/// 
		/// Note:
		///		For performance reasons it is often be desirable to extract as many masks as you need in one go rather
		///		than extracting them individually.
		/// 
		/// \param names The names to extract, could e.g. be {'bunny1', 'car', ...}
		/// 
		/// \returns The decoded cryptomattes mapped by their name
		std::unordered_map<std::string, compressed::channel<float32_t>> masks_compressed(std::vector<std::string> names);

		/// \brief Extract the masks with the given hashes from the cryptomatte into compressed buffers, computing on the fly.
		/// 
		/// The hashes here are the pixel hashes of the masks you wish to extract. If a hash could not be found that mask
		/// will be skipped.
		/// 
		/// Note:
		///		For performance reasons it is often be desirable to extract as many masks as you need in one go rather
		///		than extracting them individually.
		/// 
		/// \param names The hashes to extract, any non-valid hashes will be skipped and will also not appear in the output
		/// 
		/// \returns The decoded cryptomattes mapped by their name (if the manifest exists) or by their hashes in std::string
		///			 form.
		std::unordered_map<std::string, compressed::channel<float32_t>> masks_compressed(std::vector<uint32_t> hashes);

		/// \brief Extract all of the cryptomatte masks into compressed buffers, computing them on the fly
		/// 
		/// \param use_manifest Whether to use the manifest to preallocate the correct number of items before the hot loop.
		///						This allows for quite some optimizations so whenever possible this should be turned on.
		///						It may however throw a std::out_of_range if the manifest is faulty or incomplete.
		///						Therefore, often it is desirable to wrap this call in a try; catch falling back to the
		///						potentially slower, non-manifest based decoding. If no cryptomatte manifest exists,
		///						and this is true this function will run as if `use_manifest` was off.
		/// 
		/// \returns The decoded cryptomattes mapped by their name (if the manifest exists) or by their hashes in std::string
		///			 form.
		std::unordered_map<std::string, compressed::channel<float32_t>> masks_compressed() const;

		/// Retrieve the number of levels (rank-coverage pairs) the cryptomatte was encoded with.
		size_t num_levels() const noexcept;

		/// Get the metadata associated with the cryptomatte file, this includes things such as the channel names,
		/// the unique key identifier and the cryptomatte manifest (a mapping of human-readable names to their hashes).
		NAMESPACE_CRYPTOMATTE_API::metadata metadata();
		
	private:
		/// The channels related to this cryptomatte mapped by their full names.
		/// this may look as follows:
		/// {
		///		CryptoAsset00.r : <channel>,
		///		CryptoAsset00.g : <channel>,
		///		CryptoAsset00.b : <channel>,
		///		CryptoAsset00.a : <channel>,
		///		CryptoAsset01.r : <channel>,
		///		...
		/// }
		std::unordered_map<std::string, compressed::channel<float32_t>> m_Channels;

		/// The legacy channels related to this cryptomatte, these sometimes contain a filtered preview image but
		/// have no effect on decoding.
		/// 
		/// this may look as follows:
		/// {
		///		CryptoAsset.r : <black>,
		///		CryptoAsset.g : <preview>,
		///		CryptoAsset.b : <preview>,
		///		...
		/// }
		std::unordered_map<std::string, compressed::channel<float32_t>> m_LegacyChannels;

		/// The cryptomattes' metadata, this contains information on 
		metadata m_Metadata;
	};

} // NAMESPACE_CRYPTOMATTE_API