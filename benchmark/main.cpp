#include <vector>
#include <filesystem>
#include <execution>
#include <algorithm>
#include <span>

#include <benchmark/benchmark.h>

#include "cryptomatte/cryptomatte.h"
// Macros enabled via compile definitions
#include "cryptomatte/detail/scoped_timer.h"

#include "memory_sampling.h"

using namespace NAMESPACE_CRYPTOMATTE_API;


static const std::filesystem::path s_images_path = std::filesystem::current_path() / "images";


/// Iterate all the images in ./images/ and return them to us. Our benchmarks will instantiate for each of these
std::vector<std::filesystem::path> get_images()
{
	std::vector<std::filesystem::path> result{};
	for (const auto& entry : std::filesystem::directory_iterator(s_images_path))
	{
		if (std::filesystem::is_regular_file(entry) && entry.path().extension() == ".exr")
		{
			result.push_back(entry.path());
		}
	}
	return result;
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


void bench_cryptomatte_load(benchmark::State& state, const std::filesystem::path& image_path)
{
	bench_util::run_with_memory_sampling(state, [&]()
		{
			_CRYPTOMATTE_PROFILE_FUNCTION();
			auto image = ::cryptomatte::load(image_path, false);
			benchmark::DoNotOptimize(image);
			benchmark::ClobberMemory();
		});
}


void bench_cryptomatte_masks_compressed(benchmark::State& state, const std::filesystem::path& image_path)
{
	auto cmattes = cryptomatte::load(image_path, false);
	for (auto& matte : cmattes)
	{
		std::unordered_map<std::string, compressed::channel<float32_t>> all_masks;
		bench_util::run_with_memory_sampling(state, [&]()
			{
				_CRYPTOMATTE_PROFILE_FUNCTION();
				all_masks = matte.masks_compressed();
				benchmark::ClobberMemory();
			});

		//for (const auto& [name, mask] : all_masks)
		//{
		//	auto base_path = image_path.parent_path();

		//	std::string safe_name = name;
		//	std::replace(safe_name.begin(), safe_name.end(), '/', '_');
		//	std::replace(safe_name.begin(), safe_name.end(), '\\', '_');
		//	std::replace(safe_name.begin(), safe_name.end(), '\n', '_');
		//	std::replace(safe_name.begin(), safe_name.end(), ':', '_');

		//	// clear the extension temporarily
		//	auto base_name = image_path.filename().replace_extension("");
		//	std::filesystem::create_directory(base_path / base_name);
		//	base_name = std::format("{}/{}.exr", base_name.string(), safe_name);
		//	auto out_path = base_path / base_name;
		//	auto out_path_str = out_path.string();

		//	auto pixels = mask.get_decompressed();

		//	write_exr(out_path_str, pixels, mask.width(), mask.height());
		//}
	}
}


auto main(int argc, char** argv) -> int
{
	detail::Instrumentor::Get().BeginSession("BenchCryptomatte");

	for (const auto& image : get_images())
	{
		// Read benchmarks
		benchmark::RegisterBenchmark(
			std::format("cryptomatte::load: {}", image.filename().string()),
			&bench_cryptomatte_load, 
			image
		)->Unit(benchmark::kMillisecond)->Iterations(3);
		benchmark::RegisterBenchmark(
			std::format("cryptomatte::masks_compressed {}", image.filename().string()), 
			&bench_cryptomatte_masks_compressed, 
			image
		)->Unit(benchmark::kMillisecond)->Iterations(3);
}

	benchmark::Initialize(&argc, argv);
	benchmark::RunSpecifiedBenchmarks();

	detail::Instrumentor::Get().EndSession();
}