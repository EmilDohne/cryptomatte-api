import pytest
import json
import struct
import pathlib
import os
import unittest
import tempfile

import cryptomatte_api


def bit_cast_to_float32(i):
    return struct.unpack("f", struct.pack("I", i))[0]


def create_test_json():
    return {
        "my_bunny_01": "00000001",
        "car_01": "00000002",
        "bunny_02": "00000003",
        "fire_truck": "00000004"
    }


class TestMetadata(unittest.TestCase):

    def test_create_manifest_from_json(self):
        manif = cryptomatte_api.Manifest.from_json(create_test_json())
        assert manif.contains("my_bunny_01")
        assert manif.contains("car_01")
        assert manif.contains("bunny_02")
        assert manif.contains("fire_truck")

        assert manif.hash_hex("my_bunny_01") == "00000001"
        assert manif.hash_hex("car_01") == "00000002"
        assert manif.hash_hex("bunny_02") == "00000003"
        assert manif.hash_hex("fire_truck") == "00000004"


    def test_create_manifest_from_string(self):
        json_str = json.dumps(create_test_json())
        manif = cryptomatte_api.Manifest.from_str(json_str)

        for k, v in create_test_json().items():
            assert manif.contains(k)
            assert manif.hash_hex(k) == v


    def test_manifest_contains(self):
        manif = cryptomatte_api.Manifest.from_json({"my_bunny_01": "00000001"})
        assert manif.contains("my_bunny_01")
        assert not manif.contains("car_01")


    def test_manifest_mapping_as_uint32(self):
        manif = cryptomatte_api.Manifest.from_json(create_test_json())
        mapping = manif.mapping_uint32()

        assert ("my_bunny_01", 1) in mapping
        assert ("car_01", 2) in mapping
        assert ("bunny_02", 3) in mapping
        assert ("fire_truck", 4) in mapping


    def test_manifest_mapping_as_float32(self):
        manif = cryptomatte_api.Manifest.from_json(create_test_json())
        mapping = manif.mapping_float()

        assert ("my_bunny_01", bit_cast_to_float32(1)) in mapping
        assert ("car_01", bit_cast_to_float32(2)) in mapping
        assert ("bunny_02", bit_cast_to_float32(3)) in mapping
        assert ("fire_truck", bit_cast_to_float32(4)) in mapping

    def test_manifest_hash_with_different_types(self):
        manif = cryptomatte_api.Manifest.from_json(create_test_json())

        assert manif.hash_hex("my_bunny_01") == "00000001"
        assert manif.hash_hex("car_01") == "00000002"
        assert manif.hash_hex("bunny_02") == "00000003"
        assert manif.hash_hex("fire_truck") == "00000004"

        assert manif.hash_uint32("my_bunny_01") == 1
        assert manif.hash_uint32("car_01") == 2
        assert manif.hash_uint32("bunny_02") == 3
        assert manif.hash_uint32("fire_truck") == 4

        assert manif.hash_float("my_bunny_01") == bit_cast_to_float32(1)
        assert manif.hash_float("car_01") == bit_cast_to_float32(2)
        assert manif.hash_float("bunny_02") == bit_cast_to_float32(3)
        assert manif.hash_float("fire_truck") == bit_cast_to_float32(4)


    def test_manifest_load_irrelevant_key(self):
        key = "cryptomatte/foo/otherkey"
        value = '{"a": "b"}'
        assert cryptomatte_api.Manifest.load(key, value, "") is None


    def test_manifest_load_valid_embedded_manifest(self):
        key = "cryptomatte/foo/manifest"
        value = '{"my_object": "00000001"}'
        res = cryptomatte_api.Manifest.load(key, value, "")
        assert res is not None
        assert res.contains("my_object")
        assert res.hash_hex("my_object") == "00000001"


    def test_manifest_load_malformed_embedded_manifest(self):
        key = "cryptomatte/foo/manifest"
        value = "{not_valid_json}"
        assert cryptomatte_api.Manifest.load(key, value, "") is None


    def test_manifest_load_valid_sidecar_file(self):
        tmp_path = tempfile.TemporaryDirectory()
        image_path = os.path.join(tmp_path.name, "image.exr")
        sidecar_path = os.path.join(tmp_path.name, "sidecar.json")
        sidecar_content = '{"sidecar_object": "00000042"}'
        with open(sidecar_path, "a") as f:
            f.write(sidecar_content)

        key = "cryptomatte/foo/manif_file"
        value = os.path.basename(sidecar_path)
        res = cryptomatte_api.Manifest.load(key, value, image_path)

        assert res is not None
        assert res.contains("sidecar_object")
        assert res.hash_hex("sidecar_object") == "00000042"


    def test_manifest_load_missing_sidecar_file(self):
        tmp_path = tempfile.TemporaryDirectory()
        image_path = os.path.join(tmp_path.name, "image.exr")
        key = "cryptomatte/foo/manif_file"
        value = "nonexistent.json"
        assert cryptomatte_api.Manifest.load(key, value, image_path) is None


    def test_manifest_load_malformed_sidecar_file(self):
        tmp_path = tempfile.TemporaryDirectory()
        image_path = os.path.join(tmp_path.name, "image.exr")
        sidecar_path = os.path.join(tmp_path.name, "sidecar.json")
        with open(sidecar_path, "a") as f:
            f.write("{bad_json}")

        key = "cryptomatte/foo/manif_file"
        value = os.path.basename(sidecar_path)
        assert cryptomatte_api.Manifest.load(key, value, image_path) is None


@pytest.mark.parametrize("bad_json", [
    "not_a_json",
    "{'almost_json'}"
])
def test_create_manifest_from_malformed_string(bad_json):
    with pytest.raises(RuntimeError):
        cryptomatte_api.Manifest.from_str(bad_json)