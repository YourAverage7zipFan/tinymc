# TinyMC
TinyMC is a C implementation of a Minecraft 1.8 client, without using any graphics libraries. It is designed to run on any system, even without a GPU. All the rendering is CPU accelerated. 

Currently it is really unstable and has a lack of features, making more of a proof of concept. But my plan is to get this to the point where bedwars is playable.

Current version: a0.0.2

Planned features: Block break/place, entities, crafting / chests / other inventory GUI stuff

My usage of AI: LLMs are currently still tools, not at a place where they can write an entire app. In this project, I use AI to write helper functions that I can't be bothered to write myself. Probably <20% of it is written with AI.

I am a C noob, sorry if I traumatized someone with my terrible code.

# System requirements

- 32MB RAM
- Win95 or higher (?)
- Basically any x86 CPU (for prebuilt binary)

Tested on Intel Pentium M. A consistent 30FPS can be achieved on default settings.

# Credits
Textures are from Coterie Craft

Block properties and item properties are collected from prismarine-data and prismarine-assets

[Miniz](https://github.com/richgel999/miniz) for decompressing server packets

[Font6x8](https://github.com/fantasticmao/font6x8/tree/main) for F3 font

[Varint.C](https://github.com/tidwall/varint.c) for varint parser

# Updates

- a0.0.2 introduced block placing!! Build it yourself you lazy bum im not uploading a prebuild binary
