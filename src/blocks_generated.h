#ifndef BLOCKS_GENERATED_H
#define BLOCKS_GENERATED_H

#include "block_textures.h"
#include <stdint.h>

typedef struct {
    const uint32_t *top;
    const uint32_t *bottom;
    const uint32_t *north;
    const uint32_t *south;
    const uint32_t *east;
    const uint32_t *west;
} BlockTextures;

const BlockTextures* get_block_textures(uint16_t id, uint8_t meta);

#endif