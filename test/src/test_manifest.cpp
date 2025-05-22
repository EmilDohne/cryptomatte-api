#include "doctest.h"

#include <vector>
#include <string>

#include "util.h"

#include "cryptomatte/manifest.h"

using namespace NAMESPACE_CRYPTOMATTE_API;


// -----------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------
TEST_CASE("create manifest from json")
{
	json_ordered json;
	json["my_bunny_01"] = "00000001";
	json["car_01"] = "00000002";
	json["bunny_02"] = "00000003";
	json["fire_truck"] = "00000004";

	auto manif = manifest(json);

	CHECK(manif.contains("my_bunny_01"));
	CHECK(manif.contains("car_01"));
	CHECK(manif.contains("bunny_02"));
	CHECK(manif.contains("fire_truck"));

	CHECK(manif.hash<std::string>("my_bunny_01") == "00000001");
	CHECK(manif.hash<std::string>("car_01") == "00000002");
	CHECK(manif.hash<std::string>("bunny_02") == "00000003");
	CHECK(manif.hash<std::string>("fire_truck") == "00000004");
}


// -----------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------
TEST_CASE("create manifest from string")
{
	json_ordered json;
	json["my_bunny_01"] = "00000001";
	json["car_01"] = "00000002";
	json["bunny_02"] = "00000003";
	json["fire_truck"] = "00000004";

	auto manif = manifest::from_str(json.dump());

	CHECK(manif.contains("my_bunny_01"));
	CHECK(manif.contains("car_01"));
	CHECK(manif.contains("bunny_02"));
	CHECK(manif.contains("fire_truck"));

	CHECK(manif.hash<std::string>("my_bunny_01") == "00000001");
	CHECK(manif.hash<std::string>("car_01") == "00000002");
	CHECK(manif.hash<std::string>("bunny_02") == "00000003");
	CHECK(manif.hash<std::string>("fire_truck") == "00000004");
}


// -----------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------
TEST_CASE("create manifest from malformed string")
{
	CHECK_THROWS_AS(manifest::from_str("not_a_json"), nlohmann::json::parse_error);
	CHECK_THROWS_AS(manifest::from_str("{'almost_json'}"), nlohmann::json::parse_error);
}


// -----------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------
TEST_CASE("manifest contains")
{
	json_ordered json;
	json["my_bunny_01"] = "00000001";

	auto manif = manifest(json);

	CHECK(manif.contains("my_bunny_01"));
	CHECK(!manif.contains("car_01"));
}


// -----------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------
TEST_CASE("manifest contains")
{
	json_ordered json;
	json["my_bunny_01"] = "00000001";

	auto manif = manifest(json);

	CHECK(manif.contains("my_bunny_01"));
	CHECK(!manif.contains("car_01"));
}


// -----------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------
TEST_CASE("manifest mapping as uint32_t")
{
	json_ordered json;
	json["my_bunny_01"] = "00000001";
	json["car_01"] = "00000002";
	json["bunny_02"] = "00000003";
	json["fire_truck"] = "00000004";

	auto manif = manifest(json);
	auto mapping = manif.mapping<uint32_t>();

	CHECK(mapping[0].first == "my_bunny_01");
	CHECK(mapping[0].second == 1);
	CHECK(mapping[1].first == "car_01");
	CHECK(mapping[1].second == 2);
	CHECK(mapping[2].first == "bunny_02");
	CHECK(mapping[2].second == 3);
	CHECK(mapping[3].first == "fire_truck");
	CHECK(mapping[3].second == 4);
}


// -----------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------
TEST_CASE("manifest mapping as float32_t")
{
	json_ordered json;
	json["my_bunny_01"] = "00000001";
	json["car_01"] = "00000002";
	json["bunny_02"] = "00000003";
	json["fire_truck"] = "00000004";

	auto manif = manifest(json);
	auto mapping = manif.mapping<float32_t>();

	CHECK(mapping[0].first == "my_bunny_01");
	CHECK(mapping[0].second == std::bit_cast<float32_t>(uint32_t{ 1 }));
	CHECK(mapping[1].first == "car_01");
	CHECK(mapping[1].second == std::bit_cast<float32_t>(uint32_t{ 2 }));
	CHECK(mapping[2].first == "bunny_02");
	CHECK(mapping[2].second == std::bit_cast<float32_t>(uint32_t{ 3 }));
	CHECK(mapping[3].first == "fire_truck");
	CHECK(mapping[3].second == std::bit_cast<float32_t>(uint32_t{ 4 }));
}


// -----------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------
TEST_CASE("manifest mapping as std::string")
{
	json_ordered json;
	json["my_bunny_01"] = "00000001";
	json["car_01"] = "00000002";
	json["bunny_02"] = "00000003";
	json["fire_truck"] = "00000004";

	auto manif = manifest(json);
	auto mapping = manif.mapping<std::string>();

	CHECK(mapping[0].first == "my_bunny_01");
	CHECK(mapping[0].second == "00000001");
	CHECK(mapping[1].first == "car_01");
	CHECK(mapping[1].second == "00000002");
	CHECK(mapping[2].first == "bunny_02");
	CHECK(mapping[2].second == "00000003");
	CHECK(mapping[3].first == "fire_truck");
	CHECK(mapping[3].second == "00000004");
}


// -----------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------
TEST_CASE("manifest hash with different types")
{
	json_ordered json;
	json["my_bunny_01"] = "00000001";
	json["car_01"] = "00000002";
	json["bunny_02"] = "00000003";
	json["fire_truck"] = "00000004";

	auto manif = manifest(json);

	CHECK(manif.contains("my_bunny_01"));
	CHECK(manif.contains("car_01"));
	CHECK(manif.contains("bunny_02"));
	CHECK(manif.contains("fire_truck"));

	CHECK(manif.hash<std::string>("my_bunny_01") == "00000001");
	CHECK(manif.hash<std::string>("car_01") == "00000002");
	CHECK(manif.hash<std::string>("bunny_02") == "00000003");
	CHECK(manif.hash<std::string>("fire_truck") == "00000004");

	CHECK(manif.hash<uint32_t>("my_bunny_01") == 1);
	CHECK(manif.hash<uint32_t>("car_01") == 2);
	CHECK(manif.hash<uint32_t>("bunny_02") == 3);
	CHECK(manif.hash<uint32_t>("fire_truck") == 4);

	CHECK(manif.hash<float32_t>("my_bunny_01") == std::bit_cast<float32_t>(uint32_t{ 1 }));
	CHECK(manif.hash<float32_t>("car_01") == std::bit_cast<float32_t>(uint32_t{ 2 }));
	CHECK(manif.hash<float32_t>("bunny_02") == std::bit_cast<float32_t>(uint32_t{ 3 }));
	CHECK(manif.hash<float32_t>("fire_truck") == std::bit_cast<float32_t>(uint32_t{ 4 }));
}


// -----------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------
TEST_CASE("manifest::load irrelevant key")
{
	std::string key = "cryptomatte/foo/otherkey";
	std::string value = R"({"a": "b"})";
	auto res = manifest::load(key, value, "");
	CHECK(res == std::nullopt);
}


// -----------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------
TEST_CASE("manifest::load valid embedded manifest")
{
	std::string key = "cryptomatte/foo/manifest";
	std::string value = R"({"my_object": "00000001"})";
	auto res = manifest::load(key, value, "");
	REQUIRE(res.has_value());
	CHECK(res->contains("my_object"));
	CHECK(res->hash<std::string>("my_object") == "00000001");
}


// -----------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------
TEST_CASE("manifest::load malformed embedded manifest")
{
	std::string key = "cryptomatte/foo/manifest";
	std::string value = "{not_valid_json}";  // malformed JSON
	auto res = manifest::load(key, value, "");
	CHECK(res == std::nullopt);
}


// -----------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------
TEST_CASE("manifest::load valid sidecar manifest file")
{
	std::filesystem::path image_path = "test_data/image.exr";
	std::filesystem::create_directories(image_path.parent_path());
	std::string sidecar_content = R"({"sidecar_object": "00000042"})";
	std::ofstream("test_data/sidecar.json") << sidecar_content;

	std::string key = "cryptomatte/foo/manif_file";
	std::string value = "sidecar.json";
	auto res = manifest::load(key, value, image_path);

	REQUIRE(res.has_value());
	CHECK(res->contains("sidecar_object"));
	CHECK(res->hash<std::string>("sidecar_object") == "00000042");

	std::filesystem::remove_all("test_data");
}


// -----------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------
TEST_CASE("manifest::load missing sidecar manifest file")
{
	std::filesystem::path image_path = "some/path/image.exr";
	std::string key = "cryptomatte/foo/manif_file";
	std::string value = "nonexistent.json";
	auto res = manifest::load(key, value, image_path);
	CHECK(res == std::nullopt);
}


// -----------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------
TEST_CASE("manifest::load malformed sidecar manifest file")
{
	std::filesystem::path image_path = "test_data/image.exr";
	std::filesystem::create_directories(image_path.parent_path());
	std::ofstream("test_data/sidecar.json") << "{bad_json}";

	std::string key = "cryptomatte/foo/manif_file";
	std::string value = "sidecar.json";
	auto res = manifest::load(key, value, image_path);

	CHECK(res == std::nullopt);

	std::filesystem::remove_all("test_data");
}