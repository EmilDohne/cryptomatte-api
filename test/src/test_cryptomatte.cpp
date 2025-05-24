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
}