# Accompanying Material for C-FLAT: Control Flow Attestation for Embedded Systems Software

This archive contains the accompanying material for [C-FLAT: Control Flow
Attestation for Embedded Systems Software][1].

This repository contains the source code for the C-FLAT Runtime Tracer,
Measurement engine, C-FLAT Library and the sample applications used for
evaluation. It also contains our binary instrumentation tool for ARM
binaries and pre-build kernel images of the sample applications
instrumented with C-FLAT runnable on Raspberry Pi 2.

Note: Due to confidentiality reasons, we are unable to make the complete
platform source code available in this release. It may, however, be open
sourced in the future, in which case this README will be updated with
information on how to access the complete source code.

Those wishing to reproduce C-FLAT on a Raspberry Pi 2 or other ARM-based
board will need to provide the implementation for the bootloader and
secure world kernel. In particular the kernel needs to provide:

- Basic UART functionality and printf-style formatted printing (used
  through `info()` / `debug()` macros throughout the source code).

- Dynamic memory allocation (through `malloc()` / `free()`). For the
  evaluation, we ported v3.0 of the [Two-Level Segregated Fit memory
  allocator implementation by Matthew Conte][2] to the Raspberry Pi 2

- [BLAKE2][3] cryptographic hash function. For the evaluation, we ported
  the [reference implementation of BLAKE2s][4] to the Raspberry Pi 2

- `memcpy()` and `memset()`. For the evaluation, we used the `memcpy`
  and `memset` optimized for the Raspberry Pi from [Simon J. Halls 
  copies-and-fills library][5].

## Contents

- `binaries`: Runnable pre-build kernel images for Raspberry Pi 2
- `instrumentation`: instrumentation tool 
- `library`: C-FLAT Library code
- `runtime-tracer`: Runtime Tracer Trampolines
- `measurement-engine`: C-FLAT Measurement Engine
- `samples/syringe`: Sample program and output discussed in paper
- `samples/soldering`: Sample program and output discussed in technical report

## Requirements

- [Raspberry Pi 2 Model B][6] with Broadcom BCM2836 SoC and ARM Cortex-A7 CPU
- USB TO UART interface, such as [Waveshare DVK512][7]

## Running the pre-built kernel images

The assume that the Raspberry Pi 2 SD card has been prepared with [pre-compiled
binaries of the current bootloader/GPU firmware][8] available from the Raspberry
Pi Foundation. The easiest way to obtain these is to install a [prebuilt Raspbian
image][9] on the SD card using the [offical instructions][9].

NOTE: WE DO NOT USE RASPBIAN LINUX IN THIS PROTOTYPE. THE ONLY THING NEEDED ARE
THE FIRMWARE IMAGES NEEDED TO BOOT UP THE BOARD AND THE BARE-METAL KERNEL IMAGE
REFERRED TO BELOW.

In order to seize control of the TrustZone secure world, the bootloader needs to
obtain control of the board while it’s still in secure mode. This can be
achieved by setting the `kernel_old=1` option in `config.txt` on the Raspberry 2
SD card `boot` partition:

    kernel_old=1
    disable_commandline_tags=0

With this done, the bare-metal `kernel.img` can be loaded onto the Raspberry Pi
2 SD card:

    $ cp binaries/kernel-<version>.img /boot/kernel.img

where `/boot` is the boot partition of the Raspberry Pi 2 SD card.

Please make sure that `kernel.img` is the only `.img` file present. In
particular that `kernel7.img` is NOT present on the SD card.

## Disclaimer

All implementations are only research prototypes!

Our code is NOT safe for production use! Please use it only for tests. 

## License

 Copyright (c) 2016 Aalto University

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

[http://www.apache.org/licenses/LICENSE-2.0](http://www.apache.org/licenses/LICENSE-2.0)

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.


[1]: https://arxiv.org/abs/1605.07763 "C-FLAT: Control-FLow ATtestation for Embedded Systems Software"
[2]: https://github.com/mattconte/tlsf "mattconte/tlsf: Two-Level Segregated Fit memory allocator implementation"
[3]: https://blake2.net/ "BLAKE2 — fast secure hashing"
[4]: https://github.com/BLAKE2/BLAKE2 "BLAKE2/BLAKE2: BLAKE2 official implementations"
[5]: https://github.com/simonjhall/copies-and-fills "simonjhall/copies-and-fills"
[6]: https://www.raspberrypi.org/products/raspberry-pi-2-model-b/ "Raspberry Pi 2 Model B"
[7]: http://www.waveshare.com/dvk512.htm "DVK512, Raspberry Pi Expansion Board"
[8]: https://github.com/raspberrypi/firmware "pre-compiled binaries of the current bootloader/GPU firmware"
[9]: https://www.raspberrypi.org/documentation/installation/installing-images/README.md "Installing Operating System Images"
