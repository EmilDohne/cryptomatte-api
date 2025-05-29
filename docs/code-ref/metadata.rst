..
  Copyright Contributors to the cryptomatte-api project.


.. _cmatte_metadata:

metadata
#########

Cryptomattes are required to encode metadata about themselves on-disk such as the name of the channels,
the type of hashing method used and the data type conversion to get from pixel value -> hash value.

The ``cmatte::metadata`` class allows you to transparently access all of this information and gets
created automatically on load of a :ref:`cryptomatte_struct`.

It additionally provides some utility functions for checking if a given channel is part of a cryptomatte
defined by the ``metadata``. These functions are ``is_valid_legacy_channel_name`` and ``is_valid_channel_name``
for checking both the legacy channel names and the mask channels.

how metadata is stored
**********************

Cryptomatte metadata is stored as part of the images' extra attributes and is encoded as follows:

.. code-block:: none

    cryptomatte/<key>/name: "crypto_asset"
    cryptomatte/<key>/hash: "MurmurHash3_32"
    cryptomatte/<key>/conversion: "uint32_to_float32"
    cryptomatte/<key>/<manifest/manif_file>: "{'bunny': '3dcaf72e'}"

The first 3 of these attributes are mandatory, and the ``hash`` and ``conversion`` attributes only have
one valid option (the ones shown here).

The manifest on the other hand can either be an embedded json string or a sidecar json file which is stored
as a path relative to the image. This maps a human-readable name to its pixel hash, for more visit 
:ref:`manifest_struct`

The ``name`` attribute in this case refers to the ``{typename}`` which tells us what the channel names for
this cryptomatte are. 

The ``key`` is some kind of unique identifier that identifies this cryptomatte within the file. This is needed
to separate separate cryptomatte definitions within the same file as there is no limit to how many
different cryptomattes can be encoded in a single file. It sometimes is a hash, but could be 'banana' or 
'definitely_not_a_cryptomatte'. There is no convention or limitation, it should just be unique within the file.

.. note::
    
    Often times DCCs will use the same ``key`` for the same type of cryptomattes. So if they are encoding e.g.
    a ``crypto_asset`` they may always use the key ``3fe9c8`` for this.


checking metadata without loading a file
******************************************

Sometimes it may be desirable to load the metadatas without loading the whole image to be able to 
extract only the channel names. To do this. we provide static creation functions which allow you to initialize
the metadata from the opened ``OIIO::ImageInput`` without having to open the file yourself.

These functions are ``cmatte::metadata::from_spec``, ``cmatte::metadata::from_param_value_list`` and 
``cmatte::metadata::from_json``. 

Note that these give a vector of ``cmatte::metadata`` as a single file may hold any number of cryptomattes.

Below you can find an example of retrieving the channelnames from the metadata without having to 
ever construct a :ref:`cryptomatte_struct` object.

.. code-block:: c++

    auto input_ptr = ImageInput::open("my_file");
    if (!inp) 
    {
        return;
    }

    auto metadatas = cmatte::metadata::from_spec(input_ptr->spec());
    auto& metadata = metadatas[0];
    auto mask_channel_names = metadata.channel_names(input_ptr->spec().channelnames);
    auto legacy_channel_names = metadata.legacy_channel_names(input_ptr->spec().channelnames);


.. _metadata_struct:

metadata 
*********

.. doxygenstruct:: cmatte::metadata
    :members:
    :undoc-members:

