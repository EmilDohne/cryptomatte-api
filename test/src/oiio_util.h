#pragma once

#include <vector>
#include <format>
#include <filesystem>

#include <OpenImageIO/imageio.h>

#include <compressed/image_algo.h>
#include <compressed/enums.h>


namespace test_util
{

	namespace oiio
	{

		/// Read the image using OpenImageIO (OIIO) and deinterleave all the channels into discrete buffers.
		/// 
		/// This function opens an image file using OIIO, reads its pixel data into a single buffer, 
		/// and then separates the interleaved channel data into individual channel buffers.
		/// 
		/// \tparam T The pixel data type (e.g., uint8_t, float).
		/// \param filepath The file path to the image.
		/// \return A vector of vectors, where each inner vector represents a deinterleaved channel.
		/// \throws std::runtime_error if the image fails to open or read.
		template <typename T>
		std::vector<std::vector<T>> read(std::filesystem::path filepath, int subimage = 0)
		{
			auto input_ptr = OIIO::ImageInput::open(filepath.string());
			if (!input_ptr)
			{
				throw std::runtime_error(std::format("Failed to open image {}", filepath.string()));
			}
			auto res = input_ptr->seek_subimage(subimage, 0);
			if (!res)
			{
				throw std::runtime_error(std::format("Image {} does not contain subimage {}", filepath.string(), subimage));
			}
			const OIIO::ImageSpec& spec = input_ptr->spec();
			std::vector<T> pixels(static_cast<size_t>(spec.width) * spec.height * spec.nchannels);
			std::vector<std::vector<T>> channels;
			for ([[maybe_unused]] auto _ : std::views::iota(0, spec.nchannels))
			{
				channels.push_back(std::vector<T>(static_cast<size_t>(spec.width) * spec.height));
			}

			auto typedesc = compressed::enums::get_type_desc<T>();
			auto ok = input_ptr->read_image(subimage, 0, 0, spec.nchannels, typedesc, static_cast<void*>(pixels.data()));
			if (!ok)
			{
				throw std::runtime_error(std::format("Image {} failed to read because: {}", filepath.string(), input_ptr->geterror()));
			}
			compressed::image_algo::deinterleave(std::span<const T>(pixels), channels);
			return channels;
		}

		/// Compare two vectors (representing two channels), ensuring their contents are equal.
		/// 
		/// This function checks if two channels (stored as `std::vector<T>`) contain the same number of elements, 
		/// and that all pixel values match. If any discrepancy is found, a detailed exception is thrown.
		/// 
		/// \tparam T The pixel data type (e.g., uint8_t, float).
		/// \param a The first channel to compare.
		/// \param b The second channel to compare.
		/// \param name A label for the images, used in error messages.
		/// \throws std::runtime_error if the channels differ in structure or content.
		template <typename T>
		void compare_channels(std::vector<T>& a, std::vector<T> b, std::string name, size_t channel_idx)
		{
			if (a.size() != b.size())
			{
				throw std::runtime_error(std::format("{}: Error while comparing images, mismatch in number of channels {} : {}", name, a.size(), b.size()));
			}

			for (auto i : std::views::iota(static_cast<size_t>(0), a.size()))
			{
				if (a[i] != b[i])
				{
					throw std::runtime_error(
						std::format(
							"{}: Error while comparing images, mismatch at element {} in channel {}. a: {}, b: {}",
							name,
							i,
							channel_idx,
							a[i],
							b[i]
						)
					);
				}
			}
		}

		/// Compare two nested vectors (representing two multi-channel images), ensuring their contents are equal.
		/// 
		/// This function checks if two images (stored as `std::vector<std::vector<T>>`) have the same number of channels, 
		/// that each channel contains the same number of elements, and that all pixel values match. 
		/// If any discrepancy is found, a detailed exception is thrown.
		/// 
		/// \tparam T The pixel data type (e.g., uint8_t, float).
		/// \param a The first image to compare.
		/// \param b The second image to compare.
		/// \param name A label for the images, used in error messages.
		/// \throws std::runtime_error if the images differ in structure or content.
		template <typename T>
		void compare_images(std::vector<std::vector<T>> a, std::vector<std::vector<T>> b, std::string name)
		{
			if (a.size() != b.size())
			{
				throw std::runtime_error(std::format("{}: Error while comparing images, mismatch in number of channels {} : {}", name, a.size(), b.size()));
			}

			for (auto channel_idx : std::views::iota(static_cast<size_t>(0), a.size()))
			{
				compare_channels(a[channel_idx], b[channel_idx], name, channel_idx);
			}
		}


        /// \brief Write a single channel into an exr image.
        /// \param filename the output filename
        /// \param pixels The pixel vector, must be float
        /// \param width The width to write, pixels must be width * height
        /// \param height The width to write, pixels must be width * height
        void write_exr(const std::string& filename, const std::vector<float>& pixels, int width, int height) 
        {
            if (pixels.size() != static_cast<size_t>(width * height)) 
            {
                throw std::invalid_argument(
                    std::format(
                        "Pixel data size ({}) does not match width × height ({} × {}) = {}",
                        pixels.size(), width, height, width * height
                    )
                );
            }

            auto out = OIIO::ImageOutput::create(filename);
            if (!out) 
            {
                throw std::runtime_error(std::format("Could not create image output for '{}'", filename));
            }

            OIIO::ImageSpec spec(width, height, 1, OIIO::TypeDesc::FLOAT); // 1 channel, float

            if (!out->open(filename, spec)) 
            {
                throw std::runtime_error(std::format("Failed to open file '{}': {}", filename, out->geterror()));
            }

            if (!out->write_image(OIIO::TypeDesc::FLOAT, pixels.data())) 
            {
                throw std::runtime_error(std::format("Failed to write image '{}': {}", filename, out->geterror()));
            }
        }

	} // oiio


} // test_util