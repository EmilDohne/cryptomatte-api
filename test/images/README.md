# cryptomatte-api test images

This directory houses a multitude of images meant for testing the read capabilities of the cryptomatte-api. The image data stems from various different dccs and render engines to ensure that the cryptomatte-api can successfully decode mattes from many different sources.

## Tested Render engines

Below you can find a list of all of the tested render engines as well as the images used for these. If you wish to add additional images, please also update the list below to give new developers a head start.

### Arnold

#### arnold_one_crypto_sidecar_manif.exr

File with a single cryptomatte 'crypto_object' using a sidecar manifest file.

#### arnold_three_crypto

File with three cryptomattes 'crypto_object', 'crypto_material' and 'crypto_asset' all living as part of a multichannel exr (not multipart)

### Karma CPU

#### karma_cpu_solaris_one_crypto.exr

### Karma XPU

### Clarisse

#### clarisse_two_crypto.exr

File with two cryptomattes 'cryptomatte_asset' and 'cryptomatte_material' all living as part of a multichannel exr (not multipart) 

### VRay