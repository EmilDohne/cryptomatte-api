#include "doctest.h"

#include <cryptomatte/cryptomatte.h>

using namespace NAMESPACE_CRYPTOMATTE_API;



// -----------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------
TEST_CASE("cryptomatte::load arnold 3 crypto no preview")
{
    auto cmattes = cryptomatte::load("images/arnold_three_crypto.exr", false);
    REQUIRE(cmattes.size() == 3);

    // These should come out ordered:
    SUBCASE("crypto_asset")
    {
        auto& crypto_asset = cmattes[0];

        CHECK(crypto_asset.width() == 320);
        CHECK(crypto_asset.height() == 180);
        CHECK(crypto_asset.has_preview() == false);
        CHECK(crypto_asset.preview().empty());
        CHECK(crypto_asset.num_levels() == 6);

        auto& meta = crypto_asset.metadata();
        CHECK(meta.name() == "crypto_asset");
        CHECK(meta.key() == "28322e9");
        REQUIRE(meta.manifest() != std::nullopt);
        auto manifest = meta.manifest().value();
        CHECK(manifest.size() == 0);
    }
    SUBCASE("crypto_material")
    {
        auto& crypto_material = cmattes[1];

        CHECK(crypto_material.width() == 320);
        CHECK(crypto_material.height() == 180);
        CHECK(crypto_material.has_preview() == false);
        CHECK(crypto_material.preview().empty());
        CHECK(crypto_material.num_levels() == 6);

        auto& meta = crypto_material.metadata();
        CHECK(meta.name() == "crypto_material");
        CHECK(meta.key() == "bda530a");
        REQUIRE(meta.manifest() != std::nullopt);
        auto manifest = meta.manifest().value();
        CHECK(manifest.size() == 1);
        CHECK(manifest.contains("Material__25"));
        CHECK(manifest.hash<std::string>("Material__25") == "ce242a4e");
    }
    SUBCASE("crypto_object")
    {
        auto& crypto_object = cmattes[2];

        CHECK(crypto_object.width() == 320);
        CHECK(crypto_object.height() == 180);
        CHECK(crypto_object.has_preview() == false);
        CHECK(crypto_object.preview().empty());
        CHECK(crypto_object.num_levels() == 6);

        auto& meta = crypto_object.metadata();
        CHECK(meta.name() == "crypto_object");
        CHECK(meta.key() == "f834d0a");
        REQUIRE(meta.manifest() != std::nullopt);
        auto manifest = meta.manifest().value();
        CHECK(manifest.size() == 3);

        CHECK(manifest.contains("Box001"));
        CHECK(manifest.hash<std::string>("Box001") == "6d15e631");

        CHECK(manifest.contains("Plane001"));
        CHECK(manifest.hash<std::string>("Plane001") == "ce9e0b32");

        CHECK(manifest.contains("Sphere001"));
        CHECK(manifest.hash<std::string>("Sphere001") == "7ab5de01");
    }
}


// -----------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------
TEST_CASE("cryptomatte::load arnold 3 crypto with preview")
{
    auto cmattes = cryptomatte::load("images/arnold_three_crypto.exr", true);
    REQUIRE(cmattes.size() == 3);
    
    CHECK(cmattes[0].has_preview() == true);
    CHECK(cmattes[0].preview().empty() == false);
    CHECK(cmattes[0].preview().size() == 3);

    CHECK(cmattes[1].has_preview() == true);
    CHECK(cmattes[1].preview().empty() == false);
    CHECK(cmattes[1].preview().size() == 3);

    CHECK(cmattes[2].has_preview() == true);
    CHECK(cmattes[2].preview().empty() == false);
    CHECK(cmattes[2].preview().size() == 3);

}


// -----------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------
TEST_CASE("cryptomatte::load arnold 1 crypto sidecar manif")
{
    auto cmattes = cryptomatte::load("images/arnold_one_crypto_sidecar_manif.exr", false);
    REQUIRE(cmattes.size() == 1);

    SUBCASE("crypto_object")
    {
        auto& crypto_object = cmattes[0];

        CHECK(crypto_object.width() == 320);
        CHECK(crypto_object.height() == 180);
        CHECK(crypto_object.has_preview() == false);
        CHECK(crypto_object.preview().empty());
        CHECK(crypto_object.num_levels() == 6);

        auto& meta = crypto_object.metadata();
        CHECK(meta.name() == "crypto_object");
        CHECK(meta.key() == "f834d0a");
        REQUIRE(meta.manifest() != std::nullopt);
        auto manifest = meta.manifest().value();
        CHECK(manifest.size() == 3);

        CHECK(manifest.contains("Box001"));
        CHECK(manifest.hash<std::string>("Box001") == "6d15e631");

        CHECK(manifest.contains("Plane001"));
        CHECK(manifest.hash<std::string>("Plane001") == "ce9e0b32");

        CHECK(manifest.contains("Sphere001"));
        CHECK(manifest.hash<std::string>("Sphere001") == "7ab5de01");
    }
}


// -----------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------
TEST_CASE("cryptomatte::load arnold 1 crypto three levels")
{
    auto cmattes = cryptomatte::load("images/arnold_one_crypto_three_levels.exr", false);
    REQUIRE(cmattes.size() == 1);

    SUBCASE("crypto_object")
    {
        auto& crypto_object = cmattes[0];

        CHECK(crypto_object.width() == 320);
        CHECK(crypto_object.height() == 180);
        CHECK(crypto_object.has_preview() == false);
        CHECK(crypto_object.preview().empty());
        // Despite having set up 3 levels, arnold will pad this up to 8 channels 
        // (crypto_object00 and crypto_object01).
        CHECK(crypto_object.num_levels() == 4);

        auto& meta = crypto_object.metadata();
        CHECK(meta.name() == "crypto_object");
        CHECK(meta.key() == "f834d0a");
        REQUIRE(meta.manifest() != std::nullopt);
        auto manifest = meta.manifest().value();
        CHECK(manifest.size() == 3);

        CHECK(manifest.contains("Box001"));
        CHECK(manifest.hash<std::string>("Box001") == "6d15e631");

        CHECK(manifest.contains("Plane001"));
        CHECK(manifest.hash<std::string>("Plane001") == "ce9e0b32");

        CHECK(manifest.contains("Sphere001"));
        CHECK(manifest.hash<std::string>("Sphere001") == "7ab5de01");
    }
}


// -----------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------
TEST_CASE("cryptomatte::load clarisse 2 crypto")
{
    auto cmattes = cryptomatte::load("images/clarisse_two_crypto_in_multilayer.exr", false);
    REQUIRE(cmattes.size() == 2);

 
    SUBCASE("cryptomatte_asset")
    {
        // Despite the channels being asset first, the metadata is asset second
        auto& crypto_object = cmattes[1];

        CHECK(crypto_object.width() == 1920);
        CHECK(crypto_object.height() == 1080);
        CHECK(crypto_object.has_preview() == false);
        CHECK(crypto_object.preview().empty());
        CHECK(crypto_object.num_levels() == 6);

        auto& meta = crypto_object.metadata();
        CHECK(meta.name() == "cryptomatte_asset");
        CHECK(meta.key() == "2");
        REQUIRE(meta.manifest() != std::nullopt);
        auto manifest = meta.manifest().value();
        CHECK(manifest.size() == 2);

        CHECK(manifest.contains("build://project/SHOT_NAME/GROUNDPLANE/GROUNDPLANE"));
        CHECK(manifest.hash<std::string>("build://project/SHOT_NAME/GROUNDPLANE/GROUNDPLANE") == "df860b4f");

        CHECK(manifest.contains("build://project/SHOT_NAME/DUMMY/Dragon_2_obj"));
        CHECK(manifest.hash<std::string>("build://project/SHOT_NAME/DUMMY/Dragon_2_obj") == "89c4b3cc");
    }
    SUBCASE("cryptomatte_material")
    {
        // Despite the channels being asset first, the metadata is material second
        auto& crypto_object = cmattes[0];

        CHECK(crypto_object.width() == 1920);
        CHECK(crypto_object.height() == 1080);
        CHECK(crypto_object.has_preview() == false);
        CHECK(crypto_object.preview().empty());
        CHECK(crypto_object.num_levels() == 6);

        auto& meta = crypto_object.metadata();
        CHECK(meta.name() == "cryptomatte_material");
        CHECK(meta.key() == "0");
        REQUIRE(meta.manifest() != std::nullopt);
        auto manifest = meta.manifest().value();
        CHECK(manifest.size() == 2);

        CHECK(manifest.contains("build://project/SHOT_NAME/GROUNDPLANE/mtl_GROUNDPLANE"));
        CHECK(manifest.hash<std::string>("build://project/instance/CAR/MATERIALS/__STD_glas_cover_rot/__STD_glas_cover_rot") == "1b3093df");

        CHECK(manifest.contains("build://project/SHOT_NAME/GROUNDPLANE/mtl_GROUNDPLANE"));
        CHECK(manifest.hash<std::string>("build://project/SHOT_NAME/GROUNDPLANE/mtl_GROUNDPLANE") == "f8f0ce5f");
    }
}