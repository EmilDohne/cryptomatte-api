/*
Functionally identical test suite to `test_cryptomatte_load.cpp` but instead of using our test data we instead use the 
psyop cryptomatte test images
*/

#include "doctest.h"

#include <cryptomatte/cryptomatte.h>

using namespace NAMESPACE_CRYPTOMATTE_API;


static const std::filesystem::path base_path = "images/cryptomatte_sample_images";


// -----------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------
TEST_CASE("cryptomatte::load psyop multichannel.exr debug")
{
    auto cmattes = cryptomatte::load(base_path / "debug_images/multichannel.exr", false);
    REQUIRE(cmattes.size() == 3);

    // These should come out ordered:
    SUBCASE("crypto_asset")
    {
        auto& crypto_asset = cmattes[0];

        CHECK(crypto_asset.width() == 128);
        CHECK(crypto_asset.height() == 128);
        CHECK(crypto_asset.has_preview() == false);
        CHECK(crypto_asset.preview().empty());
        CHECK(crypto_asset.num_levels() == 6);

        auto& meta = crypto_asset.metadata();
        CHECK(meta.name() == "crypto_asset");
        CHECK(meta.key() == "28322e9");
        REQUIRE(meta.manifest() != std::nullopt);
        auto manifest = meta.manifest().value();

        CHECK(manifest.size() == 2);
    }
    SUBCASE("crypto_material")
    {
        auto& crypto_material = cmattes[1];

        CHECK(crypto_material.width() == 128);
        CHECK(crypto_material.height() == 128);
        CHECK(crypto_material.has_preview() == false);
        CHECK(crypto_material.preview().empty());
        CHECK(crypto_material.num_levels() == 6);

        auto& meta = crypto_material.metadata();
        CHECK(meta.name() == "crypto_material");
        CHECK(meta.key() == "bda530a");
        REQUIRE(meta.manifest() != std::nullopt);
        auto manifest = meta.manifest().value();

        CHECK(manifest.size() == 4);
    }
    SUBCASE("crypto_object")
    {
        auto& crypto_object = cmattes[2];

        CHECK(crypto_object.width() == 128);
        CHECK(crypto_object.height() == 128);
        CHECK(crypto_object.has_preview() == false);
        CHECK(crypto_object.preview().empty());
        CHECK(crypto_object.num_levels() == 6);

        auto& meta = crypto_object.metadata();
        CHECK(meta.name() == "crypto_object");
        CHECK(meta.key() == "f834d0a");
        REQUIRE(meta.manifest() != std::nullopt);
        auto manifest = meta.manifest().value();

        CHECK(manifest.size() == 407);
    }
}


// -----------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------
TEST_CASE("cryptomatte::load psyop sidecar_manifest.exr debug")
{
    auto cmattes = cryptomatte::load(base_path / "debug_images/sidecar_manifest.exr", false);
    REQUIRE(cmattes.size() == 1);

    // These should come out ordered:
    SUBCASE("crypto_asset")
    {
        auto& crypto_asset = cmattes[0];

        CHECK(crypto_asset.width() == 64);
        CHECK(crypto_asset.height() == 64);
        CHECK(crypto_asset.has_preview() == false);
        CHECK(crypto_asset.preview().empty());
        CHECK(crypto_asset.num_levels() == 6);

        auto& meta = crypto_asset.metadata();
        CHECK(meta.name() == "crypto_asset");
        CHECK(meta.key() == "28322e9");
        REQUIRE(meta.manifest() != std::nullopt);
        auto manifest = meta.manifest().value();

        CHECK(manifest.size() == 8);

        CHECK(manifest.contains("default"));
        CHECK(manifest.hash<std::string>("default") == "42c9679f");

        CHECK(manifest.contains("default_111"));
        CHECK(manifest.hash<std::string>("default_111") == "ad58c73a");

        CHECK(manifest.contains("default_111"));
        CHECK(manifest.hash<std::string>("default_111") == "ad58c73a");

        CHECK(manifest.contains("default_222"));
        CHECK(manifest.hash<std::string>("default_222") == "24c0148d");

        CHECK(manifest.contains("default_333"));
        CHECK(manifest.hash<std::string>("default_333") == "64fdaf51");

        CHECK(manifest.contains("default_444"));
        CHECK(manifest.hash<std::string>("default_444") == "8ca57ad9");

        CHECK(manifest.contains("default_555"));
        CHECK(manifest.hash<std::string>("default_555") == "56c1f4ca");

        CHECK(manifest.contains("leftAsset"));
        CHECK(manifest.hash<std::string>("leftAsset") == "ce406a47");

        CHECK(manifest.contains("rightAsset"));
        CHECK(manifest.hash<std::string>("rightAsset") == "8f3ccbde");
    }
}


// -----------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------
TEST_CASE("cryptomatte::load psyop special_chars.exr debug")
{
    auto cmattes = cryptomatte::load(base_path / "debug_images/special_chars.exr", false);
    REQUIRE(cmattes.size() == 1);

    // These should come out ordered:
    SUBCASE("special_chars")
    {
        auto& crypto_asset = cmattes[0];

        CHECK(crypto_asset.width() == 64);
        CHECK(crypto_asset.height() == 64);
        CHECK(crypto_asset.has_preview() == false);
        CHECK(crypto_asset.preview().empty());
        CHECK(crypto_asset.num_levels() == 6);

        auto& meta = crypto_asset.metadata();
        CHECK(meta.name() == "special_chars");
        CHECK(meta.key() == "fbb35e3");
        REQUIRE(meta.manifest() != std::nullopt);
        auto manifest = meta.manifest().value();

        CHECK(manifest.size() == 4);

        CHECK(manifest.contains(R"(name containing a "quote"  )"));
        CHECK(manifest.hash<std::string>(R"(name containing a "quote"  )") == "af8bfa09");

        CHECK(manifest.contains("name_containing/slash"));
        CHECK(manifest.hash<std::string>("name_containing/slash") == "b0a04231");

        CHECK(manifest.contains("равнина"));
        CHECK(manifest.hash<std::string>("равнина") == "9623511e");

        CHECK(manifest.contains("name, with, comma"));
        CHECK(manifest.hash<std::string>("name, with, comma") == "62fa4db8");
    }
}


// -----------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------
TEST_CASE("cryptomatte::load psyop bunny sidecar manif")
{
    auto cmattes = cryptomatte::load(base_path / "sidecar_manifest/bunny_CryptoObject.exr", false);
    REQUIRE(cmattes.size() == 1);

    // These should come out ordered:
    SUBCASE("uCryptoObject")
    {
        auto& crypto_object = cmattes[0];

        CHECK(crypto_object.width() == 1280);
        CHECK(crypto_object.height() == 720);
        CHECK(crypto_object.has_preview() == false);
        CHECK(crypto_object.preview().empty());
        CHECK(crypto_object.num_levels() == 6);

        auto& meta = crypto_object.metadata();
        CHECK(meta.name() == "uCryptoObject");
        CHECK(meta.key() == "ae93ba3");
        REQUIRE(meta.manifest() != std::nullopt);
        auto manifest = meta.manifest().value();

        CHECK(manifest.size() == 32166);

    }
}

// -----------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------
TEST_CASE("cryptomatte::load psyop test grid")
{
    auto cmattes = cryptomatte::load(base_path / "testGrid_CryptoObject.exr", false);
    REQUIRE(cmattes.size() == 1);

    // These should come out ordered:
    SUBCASE("uCryptoObject")
    {
        auto& crypto_object = cmattes[0];

        CHECK(crypto_object.width() == 1920);
        CHECK(crypto_object.height() == 1080);
        CHECK(crypto_object.has_preview() == false);
        CHECK(crypto_object.preview().empty());
        CHECK(crypto_object.num_levels() == 6);

        auto& meta = crypto_object.metadata();
        CHECK(meta.name() == "uCryptoObject");
        CHECK(meta.key() == "ae93ba3");
        REQUIRE(meta.manifest() != std::nullopt);
        auto manifest = meta.manifest().value();

        CHECK(manifest.size() == 10212);

    }
}