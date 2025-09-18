..
  Copyright Contributors to the cryptomatte-api project.


.. _cmatte_manifest:

manifest
#########

The manifest of a cryptomatte is an optional field that encodes a key-value list (implemented as json)
mapping the name of the mask to its hash value. For example:

.. code-block:: json

    {
        "bunny" : "13851a76", 
        "default" : "42c9679f"
    }

This manifest serves to give the decoding additional information and to give names to the masks. 
It should however **never** be relied on for decoding. This is because the manifest is not required
to be encoded and is also in practice quite unreliable to read from across different DCCs.

Internally we only use this to assign names to masks but our decoding happens separate of it.

It is part of the :ref:`metadata_struct` stored as an optional value. Loading the manifest from 
a file should be done through the :ref:`metadata_struct` rather than doing it directly.


.. _manifest_struct:

manifest 
*********



.. tab:: c++

    .. doxygenstruct:: cmatte::manifest
            :members:
            :undoc-members:

.. tab:: python

    .. autoclass:: cryptomatte_api.Manifest
        :members:
        :inherited-members:

        .. automethod:: __init__