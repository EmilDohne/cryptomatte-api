#include "doctest.h"

#include <vector>
#include <string>

#include "util.h"
#include "oiio_util.h"

#include "cryptomatte/cryptomatte.h"

using namespace NAMESPACE_CRYPTOMATTE_API;


namespace
{
    void iterate_manif_and_check_masks(const cryptomatte& crypto, std::string base_path)
    {
        const auto& metadata = crypto.metadata();
        const auto manifest = metadata.manifest().value();

        for (const auto& [name, _hash] : manifest.mapping())
        {
            std::string safe_name = name;
            std::replace(safe_name.begin(), safe_name.end(), '/', '_');
            std::replace(safe_name.begin(), safe_name.end(), '\\', '_');
            std::replace(safe_name.begin(), safe_name.end(), '\n', '_');
            std::replace(safe_name.begin(), safe_name.end(), ':', '_');

            std::string formatted_filename = std::format("{}_{}.exr", base_path, safe_name);
            auto mask = crypto.mask(name);

            // Uncomment this if you wish to write the images for debugging purposes or when adding other cryptomattes
            // to generate the reference output
            //test_util::oiio::write_exr(
            //    formatted_filename,
            //    mask,
            //    static_cast<int>(crypto.width()),
            //    static_cast<int>(crypto.height())
            //);

            auto read_mask = test_util::oiio::read<float32_t>(formatted_filename).at(0);
            test_util::oiio::compare_channels(mask, read_mask, formatted_filename, 0);
        }
    }
}


// -----------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------
TEST_CASE("cryptomatte::mask check arnold_one_crypto_sidecar_manif.exr")
{
    auto cmattes = cryptomatte::load("images/arnold_one_crypto_sidecar_manif.exr", false);
    REQUIRE(cmattes.size() == 1);

    SUBCASE("crypto_object")
    {
        auto& crypto_object = cmattes[0];
        iterate_manif_and_check_masks(crypto_object, "reference/arnold/crypto_object");
    }
}


// -----------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------
TEST_CASE("cryptomatte::mask check arnold_one_crypto_three_levels.exr")
{
    auto cmattes = cryptomatte::load("images/arnold_one_crypto_three_levels.exr", false);
    REQUIRE(cmattes.size() == 1);

    SUBCASE("crypto_object")
    {
        auto& crypto_object = cmattes[0];
        iterate_manif_and_check_masks(crypto_object, "reference/arnold/crypto_object");
    }
}


// -----------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------
TEST_CASE("cryptomatte::mask check arnold_three_crypto.exr")
{
    auto cmattes = cryptomatte::load("images/arnold_three_crypto.exr", false);
    REQUIRE(cmattes.size() == 3);

    SUBCASE("crypto_material")
    {
        auto& crypto_material = cmattes[1];
        iterate_manif_and_check_masks(crypto_material, "reference/arnold/crypto_material");
    }
    SUBCASE("crypto_object")
    {
        auto& crypto_object = cmattes[2];
        iterate_manif_and_check_masks(crypto_object, "reference/arnold/crypto_object");
    }
}


// -----------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------
TEST_CASE("cryptomatte::mask check clarisse_two_crypto.exr")
{
    auto cmattes = cryptomatte::load("images/clarisse_two_crypto.exr", false);
    REQUIRE(cmattes.size() == 2);

    SUBCASE("crypto_material")
    {
        auto& crypto_material = cmattes[0];
        iterate_manif_and_check_masks(crypto_material, "reference/clarisse/crypto_material");
    }
    SUBCASE("crypto_asset")
    {
        auto& crypto_object = cmattes[1];
        iterate_manif_and_check_masks(crypto_object, "reference/clarisse/crypto_object");
    }
}


// -----------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------
TEST_CASE("cryptomatte::mask check karma_cpu_solaris_one_crypto.exr")
{
    auto cmattes = cryptomatte::load("images/karma_cpu_solaris_one_crypto.exr", false);
    REQUIRE(cmattes.size() == 1);

    SUBCASE("crypto_object")
    {
        auto& crypto_object = cmattes[0];
        iterate_manif_and_check_masks(crypto_object, "reference/hou_karma_cpu/crypto_object");
    }
}


// -----------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------
TEST_CASE("cryptomatte::mask check karma_cpu_solaris_one_crypto_sidecar_manif.exr")
{
    auto cmattes = cryptomatte::load("images/karma_cpu_solaris_one_crypto_sidecar_manif.exr", false);
    REQUIRE(cmattes.size() == 1);

    SUBCASE("crypto_object")
    {
        auto& crypto_object = cmattes[0];
        iterate_manif_and_check_masks(crypto_object, "reference/hou_karma_cpu/crypto_object");
    }
}


// -----------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------
TEST_CASE("cryptomatte::mask check karma_xpu_solaris_one_crypto.exr")
{
    auto cmattes = cryptomatte::load("images/karma_xpu_solaris_one_crypto.exr", false);
    REQUIRE(cmattes.size() == 1);

    SUBCASE("crypto_object")
    {
        auto& crypto_object = cmattes[0];
        iterate_manif_and_check_masks(crypto_object, "reference/hou_karma_xpu/crypto_object");
    }
}


// -----------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------
TEST_CASE("cryptomatte::mask check karma_xpu_solaris_one_crypto_sidecar_manif.exr")
{
    auto cmattes = cryptomatte::load("images/karma_xpu_solaris_one_crypto_sidecar_manif.exr", false);
    REQUIRE(cmattes.size() == 1);

    SUBCASE("crypto_object")
    {
        auto& crypto_object = cmattes[0];
        iterate_manif_and_check_masks(crypto_object, "reference/hou_karma_xpu/crypto_object");
    }
}


// -----------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------
TEST_CASE("cryptomatte::mask check vray_single_crypto_in_multilayer.exr")
{
    auto cmattes = cryptomatte::load("images/vray_single_crypto_in_multilayer.exr", false);
    REQUIRE(cmattes.size() == 1);

    SUBCASE("cryptomatte")
    {
        auto& crypto_object = cmattes[0];
        iterate_manif_and_check_masks(crypto_object, "reference/vray_cpu/cryptomatte");
    }
}