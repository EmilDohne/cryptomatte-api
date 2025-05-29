# cryptomatte-api


[![ko-fi](https://ko-fi.com/img/githubbutton_sm.svg)](https://ko-fi.com/Q5Q4TYALW)

![](./docs/images/test_image.png)

About
=========

The ``cryptomatte-api`` is a fast, memory-efficient and robust library for loading, validating 
and decoding cryptomattes. It is meant to serve as a de-facto standard implementation of cryptomatte
loading that is DCC-agnostic and runs as a standalone.

It is written entirely using C++20 while also providing pre-built python binaries that are pip-installable.

Features
=========

- Robust and accurate decoding of cryptomattes (v1.2.0 spec)
- Fully compliant with the specification
- Extremely fast even at high resolutions
- Very memory efficient
- Rigorously tested

Performance
===========

The ``cryptomatte-api`` allows you to decode hundreds of cryptomattes for billions of pixels in 
less than a second. It is fast and efficient for both small and large images. During our :ref:`cmatte_benchmarks`
we test from just 320x140 pixels to 14480x8370 pixels for over 200 masks per-image.

We allow you to decode using in-memory compression or directly into a flat buffer giving you the 
flexibility to choose depending on your performance and memory needs.

![](./docs/images/bench_time/compressed/more_samples_log-linear.png)

![](./docs/images/bench_mem_usage/compressed/more_samples_log-log.png)


Quickstart
==========

This is a simple example of getting you up and running with the cryptomatte-api, loading a file from 
disk (which validates it) and then extract one or more masks from the image.

## C++
```cpp
#include <cryptomatte/cryptomatte.h>

auto matte = cmatte::cryptomatte::load("from/disk/path", false /* load preview channels */);
auto mask = matte.mask("my_mask"); // will throw if 'my_mask' is not available
auto all_masks = matte.masks_compressed(); // get all the masks as compressed channels.
```

## Python
```py
import cryptomatte_api as cmatte

matte = cmatte.Cryptomatte.load("from/disk/path", load_preview=False)
mask = matte.mask("my_mask") // will raise if 'my_mask' is not available
all_masks = matte.masks_compressed() // get all the masks as compressed channels.
```

License
=======
```
BSD 3-Clause License

Copyright (c) 2025, Emil Dohne

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
	list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
	this list of conditions and the following disclaimer in the documentation
	and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its
	contributors may be used to endorse or promote products derived from
	this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
```