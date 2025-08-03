import os

import cryptomatte_api as cryptomatte

# The base dir to the cpp test images, so we don't have to copy them over to our python test suite.
_BASE_IMAGE_PATH_ABS = os.path.join(os.path.dirname(__file__), "../../test/images")


# These tests are primarily to confirm our bindings are working as expected but won't cover all edge cases as the cpp
# test suite will cover that.

def test_load_cryptomatte_file():
    print(os.path.abspath(_BASE_IMAGE_PATH_ABS))
    cmattes = cryptomatte.Cryptomatte.load(os.path.join(_BASE_IMAGE_PATH_ABS, "arnold_one_crypto_sidecar_manif.exr"), False)
    assert len(cmattes) == 1

    crypto_object = cmattes[0]

    assert crypto_object.width() == 320
    assert crypto_object.height() == 180
    assert crypto_object.has_preview() == False
    assert len(crypto_object.preview()) == 0
    assert crypto_object.num_levels() == 6

    meta = crypto_object.metadata()
    assert meta.name() == "crypto_object"
    assert meta.key() == "f834d0a"
    assert meta.manifest() is not None
    manifest = meta.manifest()
    assert manifest.size() == 3

    assert manifest.contains("Box001")
    assert manifest.hash_hex("Box001") == "6d15e631"

    assert manifest.contains("Plane001")
    assert manifest.hash_hex("Plane001") == "ce9e0b32"

    assert manifest.contains("Sphere001")
    assert manifest.hash_hex("Sphere001") == "7ab5de01"

    all_masks = crypto_object.masks()
    assert "Box001" in all_masks
    assert "Plane001" in all_masks
    assert "Sphere001" in all_masks

    assert crypto_object.mask("Box001").shape == (180, 320)
    assert crypto_object.mask("Plane001").shape == (180, 320)
    assert crypto_object.mask("Sphere001").shape == (180, 320)