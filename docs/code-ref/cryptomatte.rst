..
  Copyright Contributors to the cryptomatte-api project.


.. _cmatte_cryptomatte:

cryptomatte
############

This is the main entry point for loading and interacting with cryptomatte files and their associated data.
It fully supports loading any number of cryptomattes from a given file and performs validation on the metadata,
channels and masks to ensure correctness.

Cryptomattes are a special image type that stores ID masks with support for transparency. To do this it stores
alternating ``rank`` and ``coverage`` channels where the ``rank`` channel is the ID of the pixel and the ``coverage`` 
channel is an additive value describing how much of that mask is stored in this rank-coverage pair.

This is to allow for transparent pixels by storing only one mask per pixel per rank-coverage pair and then storing
further ids that lie on the same pixel in another rank-coverage pair. The cryptomatte channels are stored
as follows:

.. code-block:: none

    {typename}.r    - deprecated -> may be skipped
    {typename}.g    - deprecated (may contain filtered preview colors) -> may be skipped
    {typename}.b    - deprecated (may contain filtered preview colors) -> may be skipped
    {typename}00.r  - id rank channel 0
    {typename}00.g  - coverage channel 0
    {typename}00.b  - id rank channel 1
    {typename}00.a  - coverage channel 1
    {typename}01.r  - id rank channel 2
    {typename}01.g  - coverage channel 2
    {typename}01.b  - id rank channel 3
    {typename}01.a  - coverage channel 3
    ...

Here typename can be anything, but usually it will be something along the lines of ``Cryptomatte``, ``CryptoAsset``,
``CryptoMaterial`` etc. Due to this cascading rank system you will typically find most data on the first rank-coverage
pairs with the further pairs being used to resolve intersections on transparent pixels with diminishing returns. 

In DCCs you will often find a setting allowing you to control the numer of ``levels``, this is the number of
rank-coverage pairs and dictates how many objects may overlap on a single pixel.

The ``cmatte::cryptomatte`` struct loads the above mentioned channels and performs mask computation on the fly. 
It allows you to either target a specific mask, multiple masks or all masks using the ``mask``, ``masks`` or 
``mask_compressed`` and ``masks_compressed`` functions (more on those later). This has the benefit of only loading
the masks you need into memory, skipping over the rest. 

As a rule of thumb however, you should always try to extract as many masks in one go as you need, as for each
mask query we need to iterate all of the above channels.

**Compression**

This Cryptomatte implementation uses in-memory compression for the channels, and optionally also compresses the
masks as they are being extracted. This is primarily to greatly reduce memory usage as masks typically compress
quite well, but also for performance reasons (less allocations -> greater speed). For specifics please check out
the benchmarking section in the documentation.

.. note::

    In-memory compression for extracted masks is currently only supported for the c++ library.


loading a cryptomatte from disk
*******************************

This is a simple example of getting you up and running with the cryptomatte-api, loading a file from 
disk (which validates it) and then extract one or more masks from the image.

.. tab:: c++

    .. code-block:: cpp

        #include <cryptomatte/cryptomatte.h>

        auto matte = cmatte::cryptomatte::load("from/disk/path", false /* load preview channels */);
        auto mask = matte.mask("my_mask"); // will throw if 'my_mask' is not available
        auto all_masks = matte.masks_compressed();

.. tab:: python

    .. code-block:: python
            
        import cryptomatte_api as cmatte

        matte = cmatte.Cryptomatte.load("from/disk/path", load_preview=False)
        mask = matte.mask("my_mask") # will raise if 'my_mask' is not available
        all_masks = matte.masks()


.. _cryptomatte_struct:

cryptomatte 
************

.. tab:: c++


    .. doxygenstruct:: cmatte::cryptomatte
            :members:
            :undoc-members:

.. tab:: python

    .. autoclass:: cryptomatte_api.Cryptomatte
        :members:
        :inherited-members:

        .. automethod:: __init__