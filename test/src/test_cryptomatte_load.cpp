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
TEST_CASE("cryptomatte::load invalid float16 bitdepth")
{
    // Cryptomattes are only supported for 32-bit, therefore loading it should raise an appropriate 
    // error.
    CHECK_THROWS_AS(cryptomatte::load("images/karma_cpu_solaris_one_crypto_16bit.exr", true), std::runtime_error);
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
    auto cmattes = cryptomatte::load("images/clarisse_two_crypto.exr", false);
    REQUIRE(cmattes.size() == 2);

 
    SUBCASE("cryptomatte_asset")
    {
        auto& crypto_object = cmattes[0];

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
        auto& crypto_object = cmattes[1];

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


namespace
{
    void test_karma_cryptomattes(cryptomatte& cmatte)
    {
        CHECK(cmatte.width() == 140);
        CHECK(cmatte.height() == 79);
        CHECK(cmatte.has_preview() == false);
        CHECK(cmatte.preview().empty());
        CHECK(cmatte.num_levels() == 6);

        auto& meta = cmatte.metadata();
        CHECK(meta.name() == "CryptoObject");
        CHECK(meta.key() == "3ae39a5");
        REQUIRE(meta.manifest() != std::nullopt);
        auto manifest = meta.manifest().value();
        CHECK(manifest.size() == 3);

        CHECK(manifest.contains("/grid1/mesh_0"));
        CHECK(manifest.hash<std::string>("/grid1/mesh_0") == "ff21a168");

        CHECK(manifest.contains("/sphere1"));
        CHECK(manifest.hash<std::string>("/sphere1") == "a8294cd8");

        CHECK(manifest.contains("/cube1"));
        CHECK(manifest.hash<std::string>("/cube1") == "5145171a");
    }
}


// -----------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------
TEST_CASE("cryptomatte::load karma_cpu 1 crypto")
{
    auto cmattes = cryptomatte::load("images/karma_cpu_solaris_one_crypto.exr", false);
    REQUIRE(cmattes.size() == 1);

    SUBCASE("CryptoObject")
    {
        auto& crypto_object = cmattes[0];
        test_karma_cryptomattes(crypto_object);       
    }
}


// -----------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------
TEST_CASE("cryptomatte::load karma_cpu 1 crypto sidecar manif")
{
    auto cmattes = cryptomatte::load("images/karma_cpu_solaris_one_crypto_sidecar_manif.exr", false);
    REQUIRE(cmattes.size() == 1);

    SUBCASE("CryptoObject")
    {
        auto& crypto_object = cmattes[0];
        test_karma_cryptomattes(crypto_object);
    }
}


// -----------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------
TEST_CASE("cryptomatte::load karma_xpu 1 crypto")
{
    auto cmattes = cryptomatte::load("images/karma_xpu_solaris_one_crypto.exr", false);
    REQUIRE(cmattes.size() == 1);

    SUBCASE("CryptoObject")
    {
        auto& crypto_object = cmattes[0];
        test_karma_cryptomattes(crypto_object);
    }
}


// -----------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------
TEST_CASE("cryptomatte::load karma_xpu 1 crypto sidecar manif")
{
    auto cmattes = cryptomatte::load("images/karma_xpu_solaris_one_crypto_sidecar_manif.exr", false);
    REQUIRE(cmattes.size() == 1);

    SUBCASE("CryptoObject")
    {
        auto& crypto_object = cmattes[0];
        test_karma_cryptomattes(crypto_object);
    }
}
