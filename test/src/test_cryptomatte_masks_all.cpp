#include "doctest.h"

#include <vector>
#include <string>

#include "util.h"
#include "oiio_util.h"

#include "cryptomatte/cryptomatte.h"

using namespace NAMESPACE_CRYPTOMATTE_API;


namespace
{

	void check_all_masks(
		const cryptomatte& crypto,
		std::unordered_map<std::string, std::vector<float32_t>>& all_masks, 
		std::string base_path
	)
	{
		for (const auto& [name, channel] : all_masks)
		{
			std::string safe_name = name;
			std::replace(safe_name.begin(), safe_name.end(), '/', '_');
			std::replace(safe_name.begin(), safe_name.end(), '\\', '_');
			std::replace(safe_name.begin(), safe_name.end(), '\n', '_');
			std::replace(safe_name.begin(), safe_name.end(), ':', '_');

			std::string formatted_filename = std::format("{}_{}.exr", base_path, safe_name);

			// Uncomment this if you wish to write the images for debugging purposes or when adding other cryptomattes
			// to generate the reference output
			//test_util::oiio::write_exr(
			//	formatted_filename,
			//	mask,
			//	static_cast<int>(crypto.width()),
			//	static_cast<int>(crypto.height())
			//);

			auto read_mask = test_util::oiio::read<float32_t>(formatted_filename).at(0);
			test_util::oiio::compare_channels(channel, read_mask, formatted_filename, 0);
		}
	}

	void check_all_masks_compressed(
		const cryptomatte& crypto, 
		std::unordered_map<std::string, compressed::channel<float32_t>>& all_masks,
		std::string base_path
	)
	{
		for (const auto& [name, channel] : all_masks)
		{
			std::string safe_name = name;
			std::replace(safe_name.begin(), safe_name.end(), '/', '_');
			std::replace(safe_name.begin(), safe_name.end(), '\\', '_');
			std::replace(safe_name.begin(), safe_name.end(), '\n', '_');
			std::replace(safe_name.begin(), safe_name.end(), ':', '_');

			std::string formatted_filename = std::format("{}_{}.exr", base_path, safe_name);
			auto mask = channel.get_decompressed();

			// Uncomment this if you wish to write the images for debugging purposes or when adding other cryptomattes
			// to generate the reference output
			//test_util::oiio::write_exr(
			//	formatted_filename,
			//	mask,
			//	static_cast<int>(crypto.width()),
			//	static_cast<int>(crypto.height())
			//);

			auto read_mask = test_util::oiio::read<float32_t>(formatted_filename).at(0);
			test_util::oiio::compare_channels(mask, read_mask, formatted_filename, 0);
		}
	}

} // anonymous namespace


// -----------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------
TEST_CASE("cryptomatte::masks compressed all check arnold_one_crypto_sidecar_manif.exr")
{
	auto cmattes = cryptomatte::load("images/arnold_one_crypto_sidecar_manif.exr", false);
	REQUIRE(cmattes.size() == 1);

	SUBCASE("crypto_object")
	{
		auto& crypto_object = cmattes[0];
		auto all_masks = crypto_object.masks_compressed();
		check_all_masks_compressed(crypto_object, all_masks, "reference/arnold/crypto_object");
	}
}

// -----------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------
TEST_CASE("cryptomatte::masks all check arnold_one_crypto_sidecar_manif.exr")
{
	auto cmattes = cryptomatte::load("images/arnold_one_crypto_sidecar_manif.exr", false);
	REQUIRE(cmattes.size() == 1);

	SUBCASE("crypto_object")
	{
		auto& crypto_object = cmattes[0];
		auto all_masks = crypto_object.masks();
		check_all_masks(crypto_object, all_masks, "reference/arnold/crypto_object");
	}
}


// -----------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------
TEST_CASE("cryptomatte::masks compressed manifest check arnold_one_crypto_sidecar_manif.exr")
{
	auto cmattes = cryptomatte::load("images/arnold_one_crypto_sidecar_manif.exr", false);
	REQUIRE(cmattes.size() == 1);

	SUBCASE("crypto_object")
	{
		auto& crypto_object = cmattes[0];
		auto manifest = crypto_object.metadata().manifest().value();
		auto all_masks = crypto_object.masks_compressed(manifest.names());
		check_all_masks_compressed(crypto_object, all_masks, "reference/arnold/crypto_object");
	}
}

// -----------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------
TEST_CASE("cryptomatte::masks manifest check arnold_one_crypto_sidecar_manif.exr")
{
	auto cmattes = cryptomatte::load("images/arnold_one_crypto_sidecar_manif.exr", false);
	REQUIRE(cmattes.size() == 1);

	SUBCASE("crypto_object")
	{
		auto& crypto_object = cmattes[0];
		auto manifest = crypto_object.metadata().manifest().value();
		auto all_masks = crypto_object.masks(manifest.names());
		check_all_masks(crypto_object, all_masks, "reference/arnold/crypto_object");
	}
}


// -----------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------
TEST_CASE("cryptomatte::masks compressed all check karma_cpu_solaris_one_crypto.exr")
{
	auto cmattes = cryptomatte::load("images/karma_cpu_solaris_one_crypto.exr", false);
	REQUIRE(cmattes.size() == 1);

	SUBCASE("crypto_object")
	{
		auto& crypto_object = cmattes[0];
		auto all_masks = crypto_object.masks_compressed();
		check_all_masks_compressed(crypto_object, all_masks, "reference/hou_karma_cpu/crypto_object");
	}
}

// -----------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------
TEST_CASE("cryptomatte::masks all check karma_cpu_solaris_one_crypto.exr")
{
	auto cmattes = cryptomatte::load("images/karma_cpu_solaris_one_crypto.exr", false);
	REQUIRE(cmattes.size() == 1);

	SUBCASE("crypto_object")
	{
		auto& crypto_object = cmattes[0];
		auto all_masks = crypto_object.masks();
		check_all_masks(crypto_object, all_masks, "reference/hou_karma_cpu/crypto_object");
	}
}


// -----------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------
TEST_CASE("cryptomatte::masks compressed manifest check karma_cpu_solaris_one_crypto.exr")
{
	auto cmattes = cryptomatte::load("images/karma_cpu_solaris_one_crypto.exr", false);
	REQUIRE(cmattes.size() == 1);

	SUBCASE("crypto_object")
	{
		auto& crypto_object = cmattes[0];
		auto manifest = crypto_object.metadata().manifest().value();
		auto all_masks = crypto_object.masks_compressed(manifest.names());
		check_all_masks_compressed(crypto_object, all_masks, "reference/hou_karma_cpu/crypto_object");
	}
}

// -----------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------
TEST_CASE("cryptomatte::masks manifest check karma_cpu_solaris_one_crypto.exr")
{
	auto cmattes = cryptomatte::load("images/karma_cpu_solaris_one_crypto.exr", false);
	REQUIRE(cmattes.size() == 1);

	SUBCASE("crypto_object")
	{
		auto& crypto_object = cmattes[0];
		auto manifest = crypto_object.metadata().manifest().value();
		auto all_masks = crypto_object.masks(manifest.names());
		check_all_masks(crypto_object, all_masks, "reference/hou_karma_cpu/crypto_object");
	}
}