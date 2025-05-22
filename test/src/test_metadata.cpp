#include "doctest.h"

#include <vector>
#include <string>

#include "util.h"

#include "cryptomatte/metadata.h"

using namespace NAMESPACE_CRYPTOMATTE_API;


// -----------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------
TEST_CASE("metadata constructor and accessors")
{
    std::string name = "CryptoAsset";
    std::string key = "abc1234";
    std::string hash = "MurmurHash3_32";
    std::string conversion = "uint32_to_float32";

    metadata meta(name, key, hash, conversion);

    CHECK(meta.name() == name);
    CHECK(meta.key() == key);
    CHECK(meta.hash_method() == "MurmurHash3_32");
    CHECK(meta.conversion_method() == "uint32_to_float32");
    CHECK(!meta.manifest().has_value());  // Should be empty
}


// -----------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------
TEST_CASE("metadata constructor throws on invalid hash or conversion")
{
    std::string name = "CryptoAsset";
    std::string key = "abc1234";

    CHECK_THROWS_AS(metadata(name, key, "BadHash", "uint32_to_float32"), std::runtime_error);
    CHECK_THROWS_AS(metadata(name, key, "MurmurHash3_32", "BadConversion"), std::runtime_error);
}


// -----------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------
TEST_CASE("metadata channel name validation")
{
    metadata meta("CryptoAsset", "abc1234", "MurmurHash3_32", "uint32_to_float32");

    // Valid channel
    CHECK(meta.is_valid("CryptoAsset00.r"));
    CHECK(meta.is_valid("CryptoAsset01.g"));

    // Invalid channel
    CHECK_FALSE(meta.is_valid("CryptoAsset.r"));
    CHECK_FALSE(meta.is_valid("WrongName00.r"));

    // Valid legacy
    CHECK(meta.is_valid_legacy("CryptoAsset.r"));
    CHECK(meta.is_valid_legacy("CryptoAsset.g"));

    // Invalid legacy
    CHECK_FALSE(meta.is_valid_legacy("CryptoAsset00.r"));
}


// -----------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------
TEST_CASE("metadata filters valid channels")
{
    metadata meta("CryptoAsset", "abc1234", "MurmurHash3_32", "uint32_to_float32");

    std::vector<std::string> channels = {
        "CryptoAsset00.r", "CryptoAsset00.g", "CryptoAsset.r", "OtherAsset00.r"
    };

    auto actual = meta.channel_names(channels);
    auto legacy = meta.legacy_channel_names(channels);

    CHECK(actual == std::vector<std::string>{"CryptoAsset00.r", "CryptoAsset00.g"});
    CHECK(legacy == std::vector<std::string>{"CryptoAsset.r"});
}


// -----------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------
TEST_CASE("metadata attribute identifiers")
{
    CHECK(metadata::attrib_name_identifier() == "name");
    CHECK(metadata::attrib_hash_method_identifier() == "hash");
    CHECK(metadata::attrib_conversion_method_identifier() == "conversion");
    CHECK(metadata::attrib_manifest_identifier() == "manifest");
    CHECK(metadata::attrib_manif_file_identifier() == "manif_file");
}


// -----------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------
TEST_CASE("metadata::from_json parses valid cryptomatte metadata") 
{
    json_ordered json;

    json["cryptomatte/abc123/name"] = "CryptoAsset";
    json["cryptomatte/abc123/hash"] = "MurmurHash3_32";
    json["cryptomatte/abc123/conversion"] = "uint32_to_float32";

    std::filesystem::path dummy_path = "dummy.exr";
    auto result = metadata::from_json(json, dummy_path);

    REQUIRE(result.size() == 1);
    CHECK(result[0].name() == "CryptoAsset");
    CHECK(result[0].key() == "abc123");
    CHECK(result[0].hash_method() == "MurmurHash3_32");
    CHECK(result[0].conversion_method() == "uint32_to_float32");
}


// -----------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------
TEST_CASE("metadata::from_json throws on invalid key format") 
{
    json_ordered json;
    json["cryptomatte/invalidkey"] = "CryptoAsset"; // missing attribute

    std::filesystem::path dummy_path = "dummy.exr";

    CHECK_THROWS_AS(metadata::from_json(json, dummy_path), std::runtime_error);
}


// -----------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------
TEST_CASE("metadata::from_json throws on unknown attribute") 
{
    json_ordered json;
    json["cryptomatte/abc123/unknown_attr"] = "value";

    std::filesystem::path dummy_path = "dummy.exr";

    CHECK_THROWS_AS(metadata::from_json(json, dummy_path), std::runtime_error);
}


// -----------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------
TEST_CASE("metadata::from_json throws when name attribute is not a string")
{
    json_ordered json;
    json["cryptomatte/abc123/name"] = 123; // not a string
    json["cryptomatte/abc123/hash"] = "MurmurHash3_32";
    json["cryptomatte/abc123/conversion"] = "uint32_to_float32";

    std::filesystem::path dummy_path = "dummy.exr";

    CHECK_THROWS_AS(metadata::from_json(json, dummy_path), std::runtime_error);
}


// -----------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------
TEST_CASE("metadata::from_json throws on invalid hash method") {
    json_ordered json;
    json["cryptomatte/abc123/name"] = "CryptoAsset";
    json["cryptomatte/abc123/hash"] = "InvalidHash";
    json["cryptomatte/abc123/conversion"] = "uint32_to_float32";

    std::filesystem::path dummy_path = "dummy.exr";

    CHECK_THROWS_AS(metadata::from_json(json, dummy_path), std::runtime_error);
}


// -----------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------
TEST_CASE("metadata::from_json throws when required name attribute is missing") {
    json_ordered json;
    json["cryptomatte/abc123/hash"] = "MurmurHash3_32";
    json["cryptomatte/abc123/conversion"] = "uint32_to_float32";

    std::filesystem::path dummy_path = "dummy.exr";

    CHECK_THROWS_AS(metadata::from_json(json, dummy_path), std::runtime_error);
}


// -----------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------
TEST_CASE("metadata::from_json parses embedded manifest") 
{
    json_ordered json;
    json["cryptomatte/abc123/name"] = "CryptoAsset";
    json["cryptomatte/abc123/hash"] = "MurmurHash3_32";
    json["cryptomatte/abc123/conversion"] = "uint32_to_float32";
    json["cryptomatte/abc123/manifest"] = R"({"hero": "00000001", "villain": "00000002"})";

    std::filesystem::path dummy_path = "dummy.exr";
    auto result = metadata::from_json(json, dummy_path);

    REQUIRE(result.size() == 1);
    auto manif = result[0].manifest();
    REQUIRE(manif.has_value());
    CHECK(manif->contains("hero"));
    CHECK(manif->hash<std::string>("hero") == "00000001");
}