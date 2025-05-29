..
  Copyright Contributors to the cryptomatte-api project.


.. _cmatte_introduction:

overview
###########

The ``cryptomatte-api`` is a fast, memory-efficient and robust library for loading, validating 
and decoding cryptomattes. It is meant to serve as a de-facto standard implementation of cryptomatte
loading that is DCC-agnostic and runs as a standalone.

It was created out of the need for such a library as there are little open-source alternatives with the
main one being the reference implementation and specification by `Psyop <https://github.com/Psyop/Cryptomatte>`_.

This library sets the foundation, but it does not provide a standalone library that can be used by consumers
outside of nuke or fusion who are then forced to hand-roll their own implementation.

The ``cryptomatte-api`` is continuously tested for accuracy and error-handling capabilities to ensure
that it can be used with confidence in production. It optionally uses in-memory compression using a 
fast compression algorithm to limit memory usage making it very efficient to extract a large
number of masks using a trivial amount of memory. For more details visit the :ref:`cmatte_benchmarks` page.

Challenges
===========

A cryptomatte library typically has a couple of challenges to overcome. The ``cryptomatte-api``
attempts to tackle all of these while still giving the users full control over what type of data they
want and how to store this data.

**Slow decoding**

As cryptomattes store potentially hundreds or thousands of masks in just a couple of channels, they are
often times slow to decode due to a large number of memory allocations needed and frequent context switching
and the resulting cache misses. 

**High memory usage**

Especially when loading many large masks, you often are memory-bound. This forces you to either decode
the cryptomattes in parts, increasing the work load, or skipping some masks.

**Invalid data**

No DCC/Render engine is perfect, files do often times come out incorrect with regards to their metadata
and manifest. This is something that needs to be handled well to avoid issues during decoding.



Acknowledgments
=================

``cryptomatte-api`` depends upon several other open source packages, detailed below. 
These other packages are all distributed under licenses that allow them to be used by cryptomatte-api. 
Any omissions or inaccuracies in this list are inadvertent and will be fixed if pointed out.

- `compressed-image <https://github.com/EmilDohne/compressed-image>`_ for reading, storing and iterating images as 
  compressed buffers rather than plain vectors. 
- `c-blosc2 <https://github.com/Blosc/c-blosc2>`_ which acts as a backbone to ``compressed-image`` for compression and
  decompression.
- `OpenImageIO <https://github.com/AcademySoftwareFoundation/OpenImageIO>`_ for interacting with image files
  on-disk, extracting their metadata etc. This is also a dependency of ``compressed-image``
- `spdlog <https://github.com/gabime/spdlog>`_ for providing a uniform logging interface.
- `nlohmann json <https://github.com/nlohmann/json>`_ for interacting with metadata and manifests as json objects 
  directly.
- `doctest <https://github.com/doctest/doctest>`_ for our test suite.
- `google-benchmark <https://github.com/google/benchmark>`_ for our benchmarking suite.
- `pybind11 <https://github.com/pybind/pybind11>`_ for our python bindings.
- `vckpg <https://github.com/microsoft/vcpkg>`_ for handling complex dependencies such as OpenImageIO.