import pytest
import cryptomatte_api as cryptomatte


def test_metadata_ctor_and_accessors():
    metadata = cryptomatte.Metadata("CryptoAsset", "abc1234", "MurmurHash3_32", "uint32_to_float32")
    assert metadata.name() == "CryptoAsset"
    assert metadata.key() == "abc1234"
    assert metadata.hash_method() == "MurmurHash3_32"
    assert metadata.conversion_method() == "uint32_to_float32"
    assert metadata.manifest() is None


def test_invalid_hash_or_conversion():
    with pytest.raises(RuntimeError):
        metadata = cryptomatte.Metadata("CryptoAsset", "abc1234", "BadHash", "uint32_to_float32")
    with pytest.raises(RuntimeError):
        metadata = cryptomatte.Metadata("CryptoAsset", "abc1234", "MurmurHash3_32", "BadConversion")


def test_metadata_chanel_name_validation():
    meta = cryptomatte.Metadata("CryptoAsset", "abc1234", "MurmurHash3_32", "uint32_to_float32");

    assert meta.is_valid_channel_name("CryptoAsset00.r")
    assert meta.is_valid_channel_name("CryptoAsset01.g")

    assert meta.is_valid_channel_name("CryptoAsset.r") == False
    assert meta.is_valid_channel_name("WrongName00.r") == False

    assert meta.is_valid_legacy_channel_name("CryptoAsset.r")
    assert meta.is_valid_legacy_channel_name("CryptoAsset.g")

    assert meta.is_valid_legacy_channel_name("CryptoAsset00.r") == False


def test_metadata_filters_valid_channels():
    meta = cryptomatte.Metadata("CryptoAsset", "abc1234", "MurmurHash3_32", "uint32_to_float32");

    channels = ["CryptoAsset00.r", "CryptoAsset00.g", "CryptoAsset.r", "OtherAsset00.r"]

    actual = meta.channel_names(channels)
    legacy = meta.legacy_channel_names(channels)

    assert actual == ["CryptoAsset00.r", "CryptoAsset00.g"]
    assert legacy == ["CryptoAsset.r"]


def test_metadata_attribute_identifiers():
    assert cryptomatte.Metadata.attrib_name_identifier() == "name"
    assert cryptomatte.Metadata.attrib_hash_method_identifier() == "hash"
    assert cryptomatte.Metadata.attrib_conversion_method_identifier() == "conversion"
    assert cryptomatte.Metadata.attrib_manifest_identifier() == "manifest"
    assert cryptomatte.Metadata.attrib_manif_file_identifier() == "manif_file"


def test_from_json_parses_metadata():
    
    data = {}
    data["cryptomatte/abc123/name"] = "CryptoAsset"
    data["cryptomatte/abc123/hash"] = "MurmurHash3_32"
    data["cryptomatte/abc123/conversion"] = "uint32_to_float32"

    dummy_path = "dummy.exr"
    result = cryptomatte.Metadata.from_json(data, dummy_path)

    assert len(result) == 1
    assert result[0].name() == "CryptoAsset"
    assert result[0].key() == "abc123"
    assert result[0].hash_method() == "MurmurHash3_32"
    assert result[0].conversion_method() == "uint32_to_float32"


def test_from_json_parses_embedded_manif():
    data = {}
    data["cryptomatte/abc123/name"] = "CryptoAsset"
    data["cryptomatte/abc123/hash"] = "MurmurHash3_32"
    data["cryptomatte/abc123/conversion"] = "uint32_to_float32"
    data["cryptomatte/abc123/manifest"] = '{"hero": "00000001", "villain": "00000002"}'

    dummy_path = "dummy.exr"
    result = cryptomatte.Metadata.from_json(data, dummy_path)

    assert len(result) == 1
    manif = result[0].manifest()
    assert manif is not None
    assert manif.contains("hero")
    assert manif.hash_hex("hero") == "00000001"