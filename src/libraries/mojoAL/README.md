# MojoAL

## What is this?

MojoAL is a full [OpenAL](https://openal.org/) 1.1 implementation, written
in C, in a single source file. It uses [Simple Directmedia Layer](https://libsdl.org/)
(SDL) 3.0 to handle much of the heavy lifting and platform abstractions,
allowing you to have a simple, portable OpenAL on any platform that SDL3
supports, from desktops to phones to web browsers to the Nintendo Switch. It
offers both stereo and surround sound outputs.

This project can be compiled directly into an app, or built as a shared
library, making it a drop-in replacement for other OpenAL implementations.

All of core OpenAL 1.1 is supported, including audio capture (recording)
and multiple device support. A handful of popular extensions are also included.

## SDL2

MojoAL was originally written for SDL2. This is still living in the `sdl2`
git branch, but is largely unmaintained now. We accept bug reports and patches
for it, but would encourage you to move to SDL3.

