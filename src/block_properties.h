#ifndef BLOCK_PROPERTIES_H
#define BLOCK_PROPERTIES_H

#include <stdint.h>
#include <stdbool.h>

// Block model types (based on model parent hierarchy)
typedef enum {
    MODEL_UNKNOWN = 0,
    MODEL_ANVIL,
    MODEL_BARS_NSEW,
    MODEL_BEACON,
    MODEL_BED_FOOT,
    MODEL_BREWING_STAND_EMPTY,
    MODEL_BUTTON,
    MODEL_CACTUS,
    MODEL_CAKE_UNEATEN,
    MODEL_CARPET,
    MODEL_CAULDRON_EMPTY,
    MODEL_COCOA_AGE0_S,
    MODEL_COMPARATOR_UNLIT,
    MODEL_CROP,
    MODEL_CROSS,
    MODEL_CUBE,
    MODEL_DAYLIGHT_DETECTOR,
    MODEL_DAYLIGHT_DETECTOR_INVERTED,
    MODEL_DOOR_BOTTOM,
    MODEL_DRAGON_EGG,
    MODEL_ENCHANTING_TABLE_BASE,
    MODEL_END_PORTAL_FRAME_EMPTY,
    MODEL_FARMLAND,
    MODEL_FENCE_GATE_CLOSED,
    MODEL_FENCE_POST,
    MODEL_FIRE_FLOOR,
    MODEL_FLOWER_POT,
    MODEL_GRASS,
    MODEL_HALF_SLAB,
    MODEL_HOPPER_DOWN,
    MODEL_LADDER,
    MODEL_LEAVES,
    MODEL_LEVER_OFF,
    MODEL_PANE_NSEW,
    MODEL_PISTON,
    MODEL_PISTON_HEAD,
    MODEL_PORTAL_EW,
    MODEL_PRESSURE_PLATE_UP,
    MODEL_RAIL_FLAT,
    MODEL_REDSTONE_NONE,
    MODEL_REPEATER_1TICK,
    MODEL_REPEATER_ON_1TICK,
    MODEL_SLIME,
    MODEL_SNOW_HEIGHT2,
    MODEL_STAIRS,
    MODEL_STEM_GROWTH0,
    MODEL_TALLGRASS,
    MODEL_TORCH,
    MODEL_TRAPDOOR_BOTTOM,
    MODEL_TRIPWIRE_HOOK,
    MODEL_TRIPWIRE_NS,
    MODEL_VINE_1,
    MODEL_WALL_POST,
    MODEL_WATERLILY,
} BlockModelType;

// Check if block is fully opaque (not transparent)
bool get_is_full_opaque(uint16_t block_id);

// Get block model type
BlockModelType get_block_model_type(uint16_t block_id);

// Check if block is minable (diggable)
bool get_block_minable(uint16_t block_id);

// Get block hardness (0.0 = instant break, -1.0 = unbreakable)
float get_block_hardness(uint16_t block_id);

// Get block stack size (usually 64)
uint8_t get_block_stack_size(uint16_t block_id);

#endif // BLOCK_PROPERTIES_H
