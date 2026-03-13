#include "collision_shapes.h"

// Auto-generated collision shapes from blockCollisionShapes.json
// Each shape is an array of AABBs (Axis-Aligned Bounding Boxes)
// Coordinates are normalized: 0.0 = block edge, 1.0 = opposite edge

const CollisionShape collision_shapes[NUM_COLLISION_SHAPES] = {
    // Shape 0
    {
        .num_boxes = 0  // No collision (air, water, etc.)
    },
    // Shape 1
    {
        .num_boxes = 1,
        .boxes = {
            {{0.0000f, 0.0000f, 0.0000f}, {1.0000f, 1.0000f, 1.0000f}},
        }
    },
    // Shape 2
    {
        .num_boxes = 1,
        .boxes = {
            {{0.0000f, 0.0000f, 0.0000f}, {1.0000f, 0.5625f, 1.0000f}},
        }
    },
    // Shape 3
    {
        .num_boxes = 1,
        .boxes = {
            {{0.0000f, 0.2500f, 0.0000f}, {1.0000f, 1.0000f, 1.0000f}},
        }
    },
    // Shape 4
    {
        .num_boxes = 1,
        .boxes = {
            {{0.0000f, 0.0000f, 0.0000f}, {1.0000f, 0.7500f, 1.0000f}},
        }
    },
    // Shape 5
    {
        .num_boxes = 1,
        .boxes = {
            {{0.0000f, 0.0000f, 0.2500f}, {1.0000f, 1.0000f, 1.0000f}},
        }
    },
    // Shape 6
    {
        .num_boxes = 1,
        .boxes = {
            {{0.0000f, 0.0000f, 0.0000f}, {1.0000f, 1.0000f, 0.7500f}},
        }
    },
    // Shape 7
    {
        .num_boxes = 1,
        .boxes = {
            {{0.2500f, 0.0000f, 0.0000f}, {1.0000f, 1.0000f, 1.0000f}},
        }
    },
    // Shape 8
    {
        .num_boxes = 1,
        .boxes = {
            {{0.0000f, 0.0000f, 0.0000f}, {0.7500f, 1.0000f, 1.0000f}},
        }
    },
    // Shape 9
    {
        .num_boxes = 2,
        .boxes = {
            {{0.0000f, 0.0000f, 0.0000f}, {1.0000f, 1.0000f, 0.2500f}},
            {{0.3750f, 0.3750f, 0.2500f}, {0.6250f, 0.6250f, 1.2500f}},
        }
    },
    // Shape 10
    {
        .num_boxes = 2,
        .boxes = {
            {{0.0000f, 0.0000f, 0.0000f}, {1.0000f, 0.2500f, 1.0000f}},
            {{0.3750f, 0.2500f, 0.3750f}, {0.6250f, 1.2500f, 0.6250f}},
        }
    },
    // Shape 11
    {
        .num_boxes = 2,
        .boxes = {
            {{0.0000f, 0.7500f, 0.0000f}, {1.0000f, 1.0000f, 1.0000f}},
            {{0.3750f, -0.2500f, 0.3750f}, {0.6250f, 0.7500f, 0.6250f}},
        }
    },
    // Shape 12
    {
        .num_boxes = 2,
        .boxes = {
            {{0.0000f, 0.0000f, 0.7500f}, {1.0000f, 1.0000f, 1.0000f}},
            {{0.3750f, 0.3750f, -0.2500f}, {0.6250f, 0.6250f, 0.7500f}},
        }
    },
    // Shape 13
    {
        .num_boxes = 2,
        .boxes = {
            {{0.0000f, 0.0000f, 0.0000f}, {0.2500f, 1.0000f, 1.0000f}},
            {{0.2500f, 0.3750f, 0.3750f}, {1.2500f, 0.6250f, 0.6250f}},
        }
    },
    // Shape 14
    {
        .num_boxes = 2,
        .boxes = {
            {{0.7500f, 0.0000f, 0.0000f}, {1.0000f, 1.0000f, 1.0000f}},
            {{-0.2500f, 0.3750f, 0.3750f}, {0.7500f, 0.6250f, 0.6250f}},
        }
    },
    // Shape 15
    {
        .num_boxes = 1,
        .boxes = {
            {{0.0000f, 0.0000f, 0.0000f}, {1.0000f, 0.5000f, 1.0000f}},
        }
    },
    // Shape 16
    {
        .num_boxes = 1,
        .boxes = {
            {{0.0000f, 0.5000f, 0.0000f}, {1.0000f, 1.0000f, 1.0000f}},
        }
    },
    // Shape 17
    {
        .num_boxes = 2,
        .boxes = {
            {{0.0000f, 0.0000f, 0.0000f}, {1.0000f, 0.5000f, 1.0000f}},
            {{0.0000f, 0.5000f, 0.0000f}, {1.0000f, 1.0000f, 0.5000f}},
        }
    },
    // Shape 18
    {
        .num_boxes = 2,
        .boxes = {
            {{0.0000f, 0.0000f, 0.0000f}, {1.0000f, 0.5000f, 1.0000f}},
            {{0.5000f, 0.5000f, 0.0000f}, {1.0000f, 1.0000f, 1.0000f}},
        }
    },
    // Shape 19
    {
        .num_boxes = 2,
        .boxes = {
            {{0.0000f, 0.0000f, 0.0000f}, {1.0000f, 0.5000f, 1.0000f}},
            {{0.0000f, 0.5000f, 0.0000f}, {0.5000f, 1.0000f, 1.0000f}},
        }
    },
    // Shape 20
    {
        .num_boxes = 2,
        .boxes = {
            {{0.0000f, 0.0000f, 0.0000f}, {1.0000f, 0.5000f, 1.0000f}},
            {{0.0000f, 0.5000f, 0.5000f}, {1.0000f, 1.0000f, 1.0000f}},
        }
    },
    // Shape 21
    {
        .num_boxes = 2,
        .boxes = {
            {{0.0000f, 0.5000f, 0.0000f}, {1.0000f, 1.0000f, 1.0000f}},
            {{0.5000f, 0.0000f, 0.0000f}, {1.0000f, 0.5000f, 1.0000f}},
        }
    },
    // Shape 22
    {
        .num_boxes = 2,
        .boxes = {
            {{0.0000f, 0.5000f, 0.0000f}, {1.0000f, 1.0000f, 1.0000f}},
            {{0.0000f, 0.0000f, 0.0000f}, {0.5000f, 0.5000f, 1.0000f}},
        }
    },
    // Shape 23
    {
        .num_boxes = 2,
        .boxes = {
            {{0.0000f, 0.5000f, 0.0000f}, {1.0000f, 1.0000f, 1.0000f}},
            {{0.0000f, 0.0000f, 0.5000f}, {1.0000f, 0.5000f, 1.0000f}},
        }
    },
    // Shape 24
    {
        .num_boxes = 2,
        .boxes = {
            {{0.0000f, 0.5000f, 0.0000f}, {1.0000f, 1.0000f, 1.0000f}},
            {{0.0000f, 0.0000f, 0.0000f}, {1.0000f, 0.5000f, 0.5000f}},
        }
    },
    // Shape 25
    {
        .num_boxes = 1,
        .boxes = {
            {{0.0625f, 0.0000f, 0.0625f}, {0.9375f, 0.8750f, 0.9375f}},
        }
    },
    // Shape 26
    {
        .num_boxes = 1,
        .boxes = {
            {{0.0000f, 0.0000f, 0.0000f}, {1.0000f, 0.9375f, 1.0000f}},
        }
    },
    // Shape 27
    {
        .num_boxes = 1,
        .boxes = {
            {{0.0000f, 0.0000f, 0.8125f}, {1.0000f, 1.0000f, 1.0000f}},
        }
    },
    // Shape 28
    {
        .num_boxes = 1,
        .boxes = {
            {{0.0000f, 0.0000f, 0.0000f}, {0.1875f, 1.0000f, 1.0000f}},
        }
    },
    // Shape 29
    {
        .num_boxes = 1,
        .boxes = {
            {{0.0000f, 0.0000f, 0.0000f}, {1.0000f, 1.0000f, 0.1875f}},
        }
    },
    // Shape 30
    {
        .num_boxes = 1,
        .boxes = {
            {{0.8125f, 0.0000f, 0.0000f}, {1.0000f, 1.0000f, 1.0000f}},
        }
    },
    // Shape 31
    {
        .num_boxes = 1,
        .boxes = {
            {{0.0000f, 0.0000f, 0.0000f}, {1.0000f, 0.1250f, 1.0000f}},
        }
    },
    // Shape 32
    {
        .num_boxes = 1,
        .boxes = {
            {{0.0000f, 0.0000f, 0.0000f}, {1.0000f, 0.2500f, 1.0000f}},
        }
    },
    // Shape 33
    {
        .num_boxes = 1,
        .boxes = {
            {{0.0000f, 0.0000f, 0.0000f}, {1.0000f, 0.3750f, 1.0000f}},
        }
    },
    // Shape 34
    {
        .num_boxes = 1,
        .boxes = {
            {{0.0000f, 0.0000f, 0.0000f}, {1.0000f, 0.6250f, 1.0000f}},
        }
    },
    // Shape 35
    {
        .num_boxes = 1,
        .boxes = {
            {{0.0000f, 0.0000f, 0.0000f}, {1.0000f, 0.8750f, 1.0000f}},
        }
    },
    // Shape 36
    {
        .num_boxes = 1,
        .boxes = {
            {{0.0625f, 0.0000f, 0.0625f}, {0.9375f, 0.9375f, 0.9375f}},
        }
    },
    // Shape 37
    {
        .num_boxes = 1,
        .boxes = {
            {{0.3750f, 0.0000f, 0.3750f}, {0.6250f, 1.5000f, 0.6250f}},
        }
    },
    // Shape 38
    {
        .num_boxes = 1,
        .boxes = {
            {{0.0625f, 0.0000f, 0.0625f}, {0.9375f, 0.5000f, 0.9375f}},
        }
    },
    // Shape 39
    {
        .num_boxes = 1,
        .boxes = {
            {{0.1875f, 0.0000f, 0.0625f}, {0.9375f, 0.5000f, 0.9375f}},
        }
    },
    // Shape 40
    {
        .num_boxes = 1,
        .boxes = {
            {{0.3125f, 0.0000f, 0.0625f}, {0.9375f, 0.5000f, 0.9375f}},
        }
    },
    // Shape 41
    {
        .num_boxes = 1,
        .boxes = {
            {{0.4375f, 0.0000f, 0.0625f}, {0.9375f, 0.5000f, 0.9375f}},
        }
    },
    // Shape 42
    {
        .num_boxes = 1,
        .boxes = {
            {{0.5625f, 0.0000f, 0.0625f}, {0.9375f, 0.5000f, 0.9375f}},
        }
    },
    // Shape 43
    {
        .num_boxes = 1,
        .boxes = {
            {{0.6875f, 0.0000f, 0.0625f}, {0.9375f, 0.5000f, 0.9375f}},
        }
    },
    // Shape 44
    {
        .num_boxes = 1,
        .boxes = {
            {{0.8125f, 0.0000f, 0.0625f}, {0.9375f, 0.5000f, 0.9375f}},
        }
    },
    // Shape 45
    {
        .num_boxes = 1,
        .boxes = {
            {{0.0000f, 0.0000f, 0.0000f}, {1.0000f, 0.1875f, 1.0000f}},
        }
    },
    // Shape 46
    {
        .num_boxes = 1,
        .boxes = {
            {{0.0000f, 0.8125f, 0.0000f}, {1.0000f, 1.0000f, 1.0000f}},
        }
    },
    // Shape 47
    {
        .num_boxes = 1,
        .boxes = {
            {{0.4375f, 0.0000f, 0.4375f}, {0.5625f, 1.0000f, 0.5625f}},
        }
    },
    // Shape 48
    {
        .num_boxes = 1,
        .boxes = {
            {{0.0000f, 0.0000f, 0.3750f}, {1.0000f, 1.5000f, 0.6250f}},
        }
    },
    // Shape 49
    {
        .num_boxes = 1,
        .boxes = {
            {{0.3750f, 0.0000f, 0.0000f}, {0.6250f, 1.5000f, 1.0000f}},
        }
    },
    // Shape 50
    {
        .num_boxes = 1,
        .boxes = {
            {{0.0625f, 0.0000f, 0.0625f}, {0.9375f, 0.0938f, 0.9375f}},
        }
    },
    // Shape 51
    {
        .num_boxes = 2,
        .boxes = {
            {{0.4375f, 0.0000f, 0.4375f}, {0.5625f, 0.8750f, 0.5625f}},
            {{0.0000f, 0.0000f, 0.0000f}, {1.0000f, 0.1250f, 1.0000f}},
        }
    },
    // Shape 52
    {
        .num_boxes = 5,
        .boxes = {
            {{0.0000f, 0.0000f, 0.0000f}, {1.0000f, 0.3125f, 1.0000f}},
            {{0.0000f, 0.0000f, 0.0000f}, {0.1250f, 1.0000f, 1.0000f}},
            {{0.0000f, 0.0000f, 0.0000f}, {1.0000f, 1.0000f, 0.1250f}},
            {{0.8750f, 0.0000f, 0.0000f}, {1.0000f, 1.0000f, 1.0000f}},
            {{0.0000f, 0.0000f, 0.8750f}, {1.0000f, 1.0000f, 1.0000f}},
        }
    },
    // Shape 53
    {
        .num_boxes = 1,
        .boxes = {
            {{0.0625f, 0.0000f, 0.0625f}, {0.9375f, 1.0000f, 0.9375f}},
        }
    },
    // Shape 54
    {
        .num_boxes = 1,
        .boxes = {
            {{0.3750f, 0.4375f, 0.0625f}, {0.6250f, 0.7500f, 0.3125f}},
        }
    },
    // Shape 55
    {
        .num_boxes = 1,
        .boxes = {
            {{0.3750f, 0.4375f, 0.6875f}, {0.6250f, 0.7500f, 0.9375f}},
        }
    },
    // Shape 56
    {
        .num_boxes = 1,
        .boxes = {
            {{0.0625f, 0.4375f, 0.3750f}, {0.3125f, 0.7500f, 0.6250f}},
        }
    },
    // Shape 57
    {
        .num_boxes = 1,
        .boxes = {
            {{0.6875f, 0.4375f, 0.3750f}, {0.9375f, 0.7500f, 0.6250f}},
        }
    },
    // Shape 58
    {
        .num_boxes = 1,
        .boxes = {
            {{0.3125f, 0.3125f, 0.5625f}, {0.6875f, 0.7500f, 0.9375f}},
        }
    },
    // Shape 59
    {
        .num_boxes = 1,
        .boxes = {
            {{0.0625f, 0.3125f, 0.3125f}, {0.4375f, 0.7500f, 0.6875f}},
        }
    },
    // Shape 60
    {
        .num_boxes = 1,
        .boxes = {
            {{0.3125f, 0.3125f, 0.0625f}, {0.6875f, 0.7500f, 0.4375f}},
        }
    },
    // Shape 61
    {
        .num_boxes = 1,
        .boxes = {
            {{0.5625f, 0.3125f, 0.3125f}, {0.9375f, 0.7500f, 0.6875f}},
        }
    },
    // Shape 62
    {
        .num_boxes = 1,
        .boxes = {
            {{0.2500f, 0.1875f, 0.4375f}, {0.7500f, 0.7500f, 0.9375f}},
        }
    },
    // Shape 63
    {
        .num_boxes = 1,
        .boxes = {
            {{0.0625f, 0.1875f, 0.2500f}, {0.5625f, 0.7500f, 0.7500f}},
        }
    },
    // Shape 64
    {
        .num_boxes = 1,
        .boxes = {
            {{0.2500f, 0.1875f, 0.0625f}, {0.7500f, 0.7500f, 0.5625f}},
        }
    },
    // Shape 65
    {
        .num_boxes = 1,
        .boxes = {
            {{0.4375f, 0.1875f, 0.2500f}, {0.9375f, 0.7500f, 0.7500f}},
        }
    },
    // Shape 66
    {
        .num_boxes = 1,
        .boxes = {
            {{0.2500f, 0.0000f, 0.2500f}, {0.7500f, 1.5000f, 0.7500f}},
        }
    },
    // Shape 67
    {
        .num_boxes = 1,
        .boxes = {
            {{0.3125f, 0.0000f, 0.3125f}, {0.6875f, 0.3750f, 0.6875f}},
        }
    },
    // Shape 68
    {
        .num_boxes = 1,
        .boxes = {
            {{0.2500f, 0.2500f, 0.5000f}, {0.7500f, 0.7500f, 1.0000f}},
        }
    },
    // Shape 69
    {
        .num_boxes = 1,
        .boxes = {
            {{0.2500f, 0.0000f, 0.2500f}, {0.7500f, 0.5000f, 0.7500f}},
        }
    },
    // Shape 70
    {
        .num_boxes = 1,
        .boxes = {
            {{0.2500f, 0.2500f, 0.0000f}, {0.7500f, 0.7500f, 0.5000f}},
        }
    },
    // Shape 71
    {
        .num_boxes = 1,
        .boxes = {
            {{0.5000f, 0.2500f, 0.2500f}, {1.0000f, 0.7500f, 0.7500f}},
        }
    },
    // Shape 72
    {
        .num_boxes = 1,
        .boxes = {
            {{0.0000f, 0.2500f, 0.2500f}, {0.5000f, 0.7500f, 0.7500f}},
        }
    },
    // Shape 73
    {
        .num_boxes = 1,
        .boxes = {
            {{0.1250f, 0.0000f, 0.0000f}, {0.8750f, 1.0000f, 1.0000f}},
        }
    },
    // Shape 74
    {
        .num_boxes = 1,
        .boxes = {
            {{0.0000f, 0.0000f, 0.1250f}, {1.0000f, 1.0000f, 0.8750f}},
        }
    },
    // Shape 75
    {
        .num_boxes = 5,
        .boxes = {
            {{0.0000f, 0.0000f, 0.0000f}, {1.0000f, 0.6250f, 1.0000f}},
            {{0.0000f, 0.0000f, 0.0000f}, {0.1250f, 1.0000f, 1.0000f}},
            {{0.8750f, 0.0000f, 0.0000f}, {1.0000f, 1.0000f, 1.0000f}},
            {{0.0000f, 0.0000f, 0.0000f}, {1.0000f, 1.0000f, 0.1250f}},
            {{0.0000f, 0.0000f, 0.8750f}, {1.0000f, 1.0000f, 1.0000f}},
        }
    },
    // Shape 76
    {
        .num_boxes = 1,
        .boxes = {
            {{0.0000f, 0.0000f, 0.0000f}, {1.0000f, 0.0625f, 1.0000f}},
        }
    },
    // Shape 81
    {
        .num_boxes = 1,
        .boxes = {
            {{0.0000f, 0.0000f, 0.0000f}, {1.0000f, 0.8125f, 1.0000f}},
        }
    },
    // Shape 82
    {
        .num_boxes = 2,
        .boxes = {
            {{0.0000f, 0.0000f, 0.0000f}, {1.0000f, 0.8125f, 1.0000f}},
            {{0.3125f, 0.8125f, 0.3125f}, {0.6875f, 1.0000f, 0.6875f}},
        }
    }
};

// Block ID to collision shape ID lookup
uint8_t get_collision_shape_id(uint16_t block_id, uint8_t meta) {
    switch(block_id) {
    case 0: { // air
        return 0;
    }
    case 1: { // stone
        return 1;
    }
    case 2: { // grass
        return 1;
    }
    case 3: { // dirt
        return 1;
    }
    case 4: { // cobblestone
        return 1;
    }
    case 5: { // planks
        return 1;
    }
    case 6: { // sapling
        return 0;
    }
    case 7: { // bedrock
        return 1;
    }
    case 8: { // flowing_water
        return 0;
    }
    case 9: { // water
        return 0;
    }
    case 10: { // flowing_lava
        return 0;
    }
    case 11: { // lava
        return 0;
    }
    case 12: { // sand
        return 1;
    }
    case 13: { // gravel
        return 1;
    }
    case 14: { // gold_ore
        return 1;
    }
    case 15: { // iron_ore
        return 1;
    }
    case 16: { // coal_ore
        return 1;
    }
    case 17: { // log
        return 1;
    }
    case 18: { // leaves
        return 1;
    }
    case 19: { // sponge
        return 1;
    }
    case 20: { // glass
        return 1;
    }
    case 21: { // lapis_ore
        return 1;
    }
    case 22: { // lapis_block
        return 1;
    }
    case 23: { // dispenser
        return 1;
    }
    case 24: { // sandstone
        return 1;
    }
    case 25: { // noteblock
        return 1;
    }
    case 26: { // bed
        return 2;
    }
    case 27: { // golden_rail
        return 0;
    }
    case 28: { // detector_rail
        return 0;
    }
    case 29: { // sticky_piston
        static const uint8_t shapes[16] = {1, 1, 1, 1, 1, 1, 0, 0, 3, 4, 5, 6, 7, 8, 0, 0};
        return shapes[meta & 0xF];
    }
    case 30: { // web
        return 0;
    }
    case 31: { // tallgrass
        return 0;
    }
    case 32: { // deadbush
        return 0;
    }
    case 33: { // piston
        static const uint8_t shapes[16] = {1, 1, 1, 1, 1, 1, 0, 0, 3, 4, 5, 6, 7, 8, 0, 0};
        return shapes[meta & 0xF];
    }
    case 34: { // piston_head
        static const uint8_t shapes[16] = {10, 11, 9, 12, 13, 14, 0, 0, 10, 11, 9, 12, 13, 14, 0, 0};
        return shapes[meta & 0xF];
    }
    case 35: { // wool
        return 1;
    }
    case 36: { // piston_extension
        return 0;
    }
    case 37: { // yellow_flower
        return 0;
    }
    case 38: { // red_flower
        return 0;
    }
    case 39: { // brown_mushroom
        return 0;
    }
    case 40: { // red_mushroom
        return 0;
    }
    case 41: { // gold_block
        return 1;
    }
    case 42: { // iron_block
        return 1;
    }
    case 43: { // double_stone_slab
        return 1;
    }
    case 44: { // stone_slab
        static const uint8_t shapes[16] = {15, 15, 15, 15, 15, 15, 15, 15, 16, 16, 16, 16, 16, 16, 16, 16};
        return shapes[meta & 0xF];
    }
    case 45: { // brick_block
        return 1;
    }
    case 46: { // tnt
        return 1;
    }
    case 47: { // bookshelf
        return 1;
    }
    case 48: { // mossy_cobblestone
        return 1;
    }
    case 49: { // obsidian
        return 1;
    }
    case 50: { // torch
        return 0;
    }
    case 51: { // fire
        return 0;
    }
    case 52: { // mob_spawner
        return 1;
    }
    case 53: { // oak_stairs
        static const uint8_t shapes[16] = {18, 19, 20, 17, 21, 22, 23, 24, 18, 19, 20, 17, 21, 22, 23, 24};
        return shapes[meta & 0xF];
    }
    case 54: { // chest
        return 25;
    }
    case 55: { // redstone_wire
        return 0;
    }
    case 56: { // diamond_ore
        return 1;
    }
    case 57: { // diamond_block
        return 1;
    }
    case 58: { // crafting_table
        return 1;
    }
    case 59: { // wheat
        return 0;
    }
    case 60: { // farmland
        return 26;
    }
    case 61: { // furnace
        return 1;
    }
    case 62: { // lit_furnace
        return 1;
    }
    case 63: { // standing_sign
        return 0;
    }
    case 64: { // wooden_door
        static const uint8_t shapes[16] = {28, 29, 30, 27, 29, 30, 27, 28, 28, 29, 30, 27, 29, 30, 27, 28};
        return shapes[meta & 0xF];
    }
    case 65: { // ladder
        static const uint8_t shapes[16] = {27, 27, 27, 29, 30, 28, 27, 27, 27, 29, 30, 28, 27, 27, 27, 29};
        return shapes[meta & 0xF];
    }
    case 66: { // rail
        return 0;
    }
    case 67: { // stone_stairs
        static const uint8_t shapes[16] = {18, 19, 20, 17, 21, 22, 23, 24, 18, 19, 20, 17, 21, 22, 23, 24};
        return shapes[meta & 0xF];
    }
    case 68: { // wall_sign
        return 0;
    }
    case 69: { // lever
        return 0;
    }
    case 70: { // stone_pressure_plate
        return 0;
    }
    case 71: { // iron_door
        static const uint8_t shapes[16] = {28, 29, 30, 27, 29, 30, 27, 28, 28, 29, 30, 27, 29, 30, 27, 28};
        return shapes[meta & 0xF];
    }
    case 72: { // wooden_pressure_plate
        return 0;
    }
    case 73: { // redstone_ore
        return 1;
    }
    case 74: { // lit_redstone_ore
        return 1;
    }
    case 75: { // unlit_redstone_torch
        return 0;
    }
    case 76: { // redstone_torch
        return 0;
    }
    case 77: { // stone_button
        return 0;
    }
    case 78: { // snow_layer
        static const uint8_t shapes[16] = {0, 31, 32, 33, 15, 34, 4, 35, 0, 31, 32, 33, 15, 34, 4, 35};
        return shapes[meta & 0xF];
    }
    case 79: { // ice
        return 1;
    }
    case 80: { // snow
        return 1;
    }
    case 81: { // cactus
        return 36;
    }
    case 82: { // clay
        return 1;
    }
    case 83: { // reeds
        return 0;
    }
    case 84: { // jukebox
        return 1;
    }
    case 85: { // fence
        return 37;
    }
    case 86: { // pumpkin
        return 1;
    }
    case 87: { // netherrack
        return 1;
    }
    case 88: { // soul_sand
        return 35;
    }
    case 89: { // glowstone
        return 1;
    }
    case 90: { // portal
        return 0;
    }
    case 91: { // lit_pumpkin
        return 1;
    }
    case 92: { // cake
        static const uint8_t shapes[16] = {38, 39, 40, 41, 42, 43, 44, 0, 0, 0, 0, 0, 0, 0, 0, 0};
        return shapes[meta & 0xF];
    }
    case 93: { // unpowered_repeater
        return 31;
    }
    case 94: { // powered_repeater
        return 31;
    }
    case 95: { // stained_glass
        return 1;
    }
    case 96: { // trapdoor
        static const uint8_t shapes[16] = {45, 45, 45, 45, 27, 29, 30, 28, 46, 46, 46, 46, 27, 29, 30, 28};
        return shapes[meta & 0xF];
    }
    case 97: { // monster_egg
        return 1;
    }
    case 98: { // stonebrick
        return 1;
    }
    case 99: { // brown_mushroom_block
        return 1;
    }
    case 100: { // red_mushroom_block
        return 1;
    }
    case 101: { // iron_bars
        return 47;
    }
    case 102: { // glass_pane
        return 47;
    }
    case 103: { // melon_block
        return 1;
    }
    case 104: { // pumpkin_stem
        return 0;
    }
    case 105: { // melon_stem
        return 0;
    }
    case 106: { // vine
        return 0;
    }
    case 107: { // fence_gate
        static const uint8_t shapes[16] = {48, 49, 48, 49, 0, 0, 0, 0, 48, 49, 48, 49, 0, 0, 0, 0};
        return shapes[meta & 0xF];
    }
    case 108: { // brick_stairs
        static const uint8_t shapes[16] = {18, 19, 20, 17, 21, 22, 23, 24, 18, 19, 20, 17, 21, 22, 23, 24};
        return shapes[meta & 0xF];
    }
    case 109: { // stone_brick_stairs
        static const uint8_t shapes[16] = {18, 19, 20, 17, 21, 22, 23, 24, 18, 19, 20, 17, 21, 22, 23, 24};
        return shapes[meta & 0xF];
    }
    case 110: { // mycelium
        return 1;
    }
    case 111: { // waterlily
        return 50;
    }
    case 112: { // nether_brick
        return 1;
    }
    case 113: { // nether_brick_fence
        return 37;
    }
    case 114: { // nether_brick_stairs
        static const uint8_t shapes[16] = {18, 19, 20, 17, 21, 22, 23, 24, 18, 19, 20, 17, 21, 22, 23, 24};
        return shapes[meta & 0xF];
    }
    case 115: { // nether_wart
        return 0;
    }
    case 116: { // enchanting_table
        return 4;
    }
    case 117: { // brewing_stand
        return 51;
    }
    case 118: { // cauldron
        return 52;
    }
    case 119: { // end_portal
        return 0;
    }
    case 120: { // end_portal_frame
        static const uint8_t shapes[16] = {81, 81, 81, 81, 82, 82, 82, 82, 81, 81, 81, 81, 82, 82, 82, 82};
        return shapes[meta & 0xF];
    }
    case 121: { // end_stone
        return 1;
    }
    case 122: { // dragon_egg
        return 53;
    }
    case 123: { // redstone_lamp
        return 1;
    }
    case 124: { // lit_redstone_lamp
        return 1;
    }
    case 125: { // double_wooden_slab
        return 1;
    }
    case 126: { // wooden_slab
        static const uint8_t shapes[16] = {15, 15, 15, 15, 15, 15, 15, 15, 16, 16, 16, 16, 16, 16, 16, 16};
        return shapes[meta & 0xF];
    }
    case 127: { // cocoa
        static const uint8_t shapes[16] = {55, 56, 54, 57, 58, 59, 60, 61, 62, 63, 64, 65, 0, 0, 0, 0};
        return shapes[meta & 0xF];
    }
    case 128: { // sandstone_stairs
        static const uint8_t shapes[16] = {18, 19, 20, 17, 21, 22, 23, 24, 18, 19, 20, 17, 21, 22, 23, 24};
        return shapes[meta & 0xF];
    }
    case 129: { // emerald_ore
        return 1;
    }
    case 130: { // ender_chest
        return 25;
    }
    case 131: { // tripwire_hook
        return 0;
    }
    case 132: { // tripwire
        return 0;
    }
    case 133: { // emerald_block
        return 1;
    }
    case 134: { // spruce_stairs
        static const uint8_t shapes[16] = {18, 19, 20, 17, 21, 22, 23, 24, 18, 19, 20, 17, 21, 22, 23, 24};
        return shapes[meta & 0xF];
    }
    case 135: { // birch_stairs
        static const uint8_t shapes[16] = {18, 19, 20, 17, 21, 22, 23, 24, 18, 19, 20, 17, 21, 22, 23, 24};
        return shapes[meta & 0xF];
    }
    case 136: { // jungle_stairs
        static const uint8_t shapes[16] = {18, 19, 20, 17, 21, 22, 23, 24, 18, 19, 20, 17, 21, 22, 23, 24};
        return shapes[meta & 0xF];
    }
    case 137: { // command_block
        return 1;
    }
    case 138: { // beacon
        return 1;
    }
    case 139: { // cobblestone_wall
        return 66;
    }
    case 140: { // flower_pot
        return 67;
    }
    case 141: { // carrots
        return 0;
    }
    case 142: { // potatoes
        return 0;
    }
    case 143: { // wooden_button
        return 0;
    }
    case 144: { // skull
        static const uint8_t shapes[16] = {69, 69, 68, 70, 71, 72, 69, 69, 69, 69, 68, 70, 71, 72, 69, 69};
        return shapes[meta & 0xF];
    }
    case 145: { // anvil
        static const uint8_t shapes[16] = {73, 74, 73, 74, 73, 74, 73, 74, 73, 74, 73, 74, 0, 0, 0, 0};
        return shapes[meta & 0xF];
    }
    case 146: { // trapped_chest
        return 25;
    }
    case 147: { // light_weighted_pressure_plate
        return 0;
    }
    case 148: { // heavy_weighted_pressure_plate
        return 0;
    }
    case 149: { // unpowered_comparator
        return 31;
    }
    case 150: { // powered_comparator
        return 31;
    }
    case 151: { // daylight_detector
        return 33;
    }
    case 152: { // redstone_block
        return 1;
    }
    case 153: { // quartz_ore
        return 1;
    }
    case 154: { // hopper
        return 75;
    }
    case 155: { // quartz_block
        return 1;
    }
    case 156: { // quartz_stairs
        static const uint8_t shapes[16] = {18, 19, 20, 17, 21, 22, 23, 24, 18, 19, 20, 17, 21, 22, 23, 24};
        return shapes[meta & 0xF];
    }
    case 157: { // activator_rail
        return 0;
    }
    case 158: { // dropper
        return 1;
    }
    case 159: { // stained_hardened_clay
        return 1;
    }
    case 160: { // stained_glass_pane
        return 47;
    }
    case 161: { // leaves2
        return 1;
    }
    case 162: { // log2
        return 1;
    }
    case 163: { // acacia_stairs
        static const uint8_t shapes[16] = {18, 19, 20, 17, 21, 22, 23, 24, 18, 19, 20, 17, 21, 22, 23, 24};
        return shapes[meta & 0xF];
    }
    case 164: { // dark_oak_stairs
        static const uint8_t shapes[16] = {18, 19, 20, 17, 21, 22, 23, 24, 18, 19, 20, 17, 21, 22, 23, 24};
        return shapes[meta & 0xF];
    }
    case 165: { // slime
        return 1;
    }
    case 166: { // barrier
        return 1;
    }
    case 167: { // iron_trapdoor
        static const uint8_t shapes[16] = {45, 45, 45, 45, 27, 29, 30, 28, 46, 46, 46, 46, 27, 29, 30, 28};
        return shapes[meta & 0xF];
    }
    case 168: { // prismarine
        return 1;
    }
    case 169: { // sea_lantern
        return 1;
    }
    case 170: { // hay_block
        return 1;
    }
    case 171: { // carpet
        return 76;
    }
    case 172: { // hardened_clay
        return 1;
    }
    case 173: { // coal_block
        return 1;
    }
    case 174: { // packed_ice
        return 1;
    }
    case 175: { // double_plant
        return 0;
    }
    case 176: { // standing_banner
        return 0;
    }
    case 177: { // wall_banner
        return 0;
    }
    case 178: { // daylight_detector_inverted
        return 33;
    }
    case 179: { // red_sandstone
        return 1;
    }
    case 180: { // red_sandstone_stairs
        static const uint8_t shapes[16] = {18, 19, 20, 17, 21, 22, 23, 24, 18, 19, 20, 17, 21, 22, 23, 24};
        return shapes[meta & 0xF];
    }
    case 181: { // double_stone_slab2
        return 1;
    }
    case 182: { // stone_slab2
        static const uint8_t shapes[16] = {15, 15, 15, 15, 15, 15, 15, 15, 16, 16, 16, 16, 16, 16, 16, 16};
        return shapes[meta & 0xF];
    }
    case 183: { // spruce_fence_gate
        static const uint8_t shapes[16] = {48, 49, 48, 49, 0, 0, 0, 0, 48, 49, 48, 49, 0, 0, 0, 0};
        return shapes[meta & 0xF];
    }
    case 184: { // birch_fence_gate
        static const uint8_t shapes[16] = {48, 49, 48, 49, 0, 0, 0, 0, 48, 49, 48, 49, 0, 0, 0, 0};
        return shapes[meta & 0xF];
    }
    case 185: { // jungle_fence_gate
        static const uint8_t shapes[16] = {48, 49, 48, 49, 0, 0, 0, 0, 48, 49, 48, 49, 0, 0, 0, 0};
        return shapes[meta & 0xF];
    }
    case 186: { // dark_oak_fence_gate
        static const uint8_t shapes[16] = {48, 49, 48, 49, 0, 0, 0, 0, 48, 49, 48, 49, 0, 0, 0, 0};
        return shapes[meta & 0xF];
    }
    case 187: { // acacia_fence_gate
        static const uint8_t shapes[16] = {48, 49, 48, 49, 0, 0, 0, 0, 48, 49, 48, 49, 0, 0, 0, 0};
        return shapes[meta & 0xF];
    }
    case 188: { // spruce_fence
        return 37;
    }
    case 189: { // birch_fence
        return 37;
    }
    case 190: { // jungle_fence
        return 37;
    }
    case 191: { // dark_oak_fence
        return 37;
    }
    case 192: { // acacia_fence
        return 37;
    }
    case 193: { // spruce_door
        static const uint8_t shapes[16] = {28, 29, 30, 27, 29, 30, 27, 28, 28, 29, 30, 27, 29, 30, 27, 28};
        return shapes[meta & 0xF];
    }
    case 194: { // birch_door
        static const uint8_t shapes[16] = {28, 29, 30, 27, 29, 30, 27, 28, 28, 29, 30, 27, 29, 30, 27, 28};
        return shapes[meta & 0xF];
    }
    case 195: { // jungle_door
        static const uint8_t shapes[16] = {28, 29, 30, 27, 29, 30, 27, 28, 28, 29, 30, 27, 29, 30, 27, 28};
        return shapes[meta & 0xF];
    }
    case 196: { // acacia_door
        static const uint8_t shapes[16] = {28, 29, 30, 27, 29, 30, 27, 28, 28, 29, 30, 27, 29, 30, 27, 28};
        return shapes[meta & 0xF];
    }
    case 197: { // dark_oak_door
        static const uint8_t shapes[16] = {28, 29, 30, 27, 29, 30, 27, 28, 28, 29, 30, 27, 29, 30, 27, 28};
        return shapes[meta & 0xF];
    }
    default:
        return 0;  // No collision
    }
}

CollisionShape get_selector_override(uint8_t block_id, uint8_t meta) { // generated with clanker https://chatgpt.com/c/69ab9021-e00c-8332-8652-dd8647fb2e1e
    CollisionShape s = {0};

    switch (block_id) {

        // --------------------------------------------------
        // PLANTS (no collision but selectable)
        // --------------------------------------------------

        case 31: // tall grass
        case 32: // dead bush
        case 37: // dandelion
        case 38: // flowers
        case 39: // brown mushroom
        case 40: // red mushroom
        case 6:  // sapling
        {
            s.num_boxes = 1;
            s.boxes[0] = (AABB){ {0.1f,0,0.1f}, {0.9f,0.8f,0.9f} };
            return s;
        }

        // wheat (height depends on growth stage)
        case 59:
        {
            float height = 0.25f + (meta / 7.0f) * 0.75f;

            s.num_boxes = 1;
            s.boxes[0] = (AABB){ {0.1f,0,0.1f}, {0.9f,height,0.9f} };
            return s;
        }

        // --------------------------------------------------
        // TORCH
        // --------------------------------------------------

        case 50:
        {
            s.num_boxes = 1;

            switch (meta) {

                // floor
                case 5:
                default:
                    s.boxes[0] = (AABB){{0.4f,0.0f,0.4f},{0.6f,0.6f,0.6f}};
                    break;

                // east wall
                case 1:
                    s.boxes[0] = (AABB){{0.0f,0.2f,0.4f},{0.3f,0.8f,0.6f}};
                    break;

                // west wall
                case 2:
                    s.boxes[0] = (AABB){{0.7f,0.2f,0.4f},{1.0f,0.8f,0.6f}};
                    break;

                // south wall
                case 3:
                    s.boxes[0] = (AABB){{0.4f,0.2f,0.0f},{0.6f,0.8f,0.3f}};
                    break;

                // north wall
                case 4:
                    s.boxes[0] = (AABB){{0.4f,0.2f,0.7f},{0.6f,0.8f,1.0f}};
                    break;
            }

            return s;
        }

        // --------------------------------------------------
        // RAILS
        // --------------------------------------------------

        case 27: // powered
        case 28: // detector
        case 66: // rail
        {
            s.num_boxes = 1;
            s.boxes[0] = (AABB){{0,0,0},{1,0.125f,1}};
            return s;
        }

        // --------------------------------------------------
        // CARPET
        // --------------------------------------------------

        case 171:
        {
            s.num_boxes = 1;
            s.boxes[0] = (AABB){{0,0,0},{1,0.0625f,1}};
            return s;
        }

        // --------------------------------------------------
        // PRESSURE PLATES
        // --------------------------------------------------

        case 70: // stone
        case 72: // wood
        case 147:
        case 148:
        {
            s.num_boxes = 1;
            s.boxes[0] = (AABB){{0.0625f,0,0.0625f},{0.9375f,0.0625f,0.9375f}};
            return s;
        }

        // --------------------------------------------------
        // BUTTONS
        // --------------------------------------------------

        case 77: // stone button
        case 143: // wood button
        {
            s.num_boxes = 1;

            int face = meta & 7;

            switch (face) {

                case 1: // east
                    s.boxes[0] = (AABB){{0,0.375f,0.3125f},{0.125f,0.625f,0.6875f}};
                    break;

                case 2: // west
                    s.boxes[0] = (AABB){{0.875f,0.375f,0.3125f},{1,0.625f,0.6875f}};
                    break;

                case 3: // south
                    s.boxes[0] = (AABB){{0.3125f,0.375f,0},{0.6875f,0.625f,0.125f}};
                    break;

                case 4: // north
                    s.boxes[0] = (AABB){{0.3125f,0.375f,0.875f},{0.6875f,0.625f,1}};
                    break;

                default:
                    s.boxes[0] = (AABB){{0.3125f,0,0.3125f},{0.6875f,0.125f,0.6875f}};
                    break;
            }

            return s;
        }

        // --------------------------------------------------
        // LADDER
        // --------------------------------------------------

        case 65:
        {
            s.num_boxes = 1;

            switch (meta) {

                case 2:
                    s.boxes[0] = (AABB){{0,0,0.875f},{1,1,1}};
                    break;

                case 3:
                    s.boxes[0] = (AABB){{0,0,0},{1,1,0.125f}};
                    break;

                case 4:
                    s.boxes[0] = (AABB){{0.875f,0,0},{1,1,1}};
                    break;

                case 5:
                    s.boxes[0] = (AABB){{0,0,0},{0.125f,1,1}};
                    break;
            }

            return s;
        }

        // --------------------------------------------------
        // SIGN (standing)
        // --------------------------------------------------

        case 63:
        {
            s.num_boxes = 1;
            s.boxes[0] = (AABB){{0.25f,0,0.25f},{0.75f,1,0.75f}};
            return s;
        }

        // --------------------------------------------------
        // WALL SIGN
        // --------------------------------------------------

        case 68:
        {
            s.num_boxes = 1;

            switch (meta) {

                case 2:
                    s.boxes[0] = (AABB){{0,0.25f,0.875f},{1,0.75f,1}};
                    break;

                case 3:
                    s.boxes[0] = (AABB){{0,0.25f,0},{1,0.75f,0.125f}};
                    break;

                case 4:
                    s.boxes[0] = (AABB){{0.875f,0.25f,0},{1,0.75f,1}};
                    break;

                case 5:
                    s.boxes[0] = (AABB){{0,0.25f,0},{0.125f,0.75f,1}};
                    break;
            }

            return s;
        }

        default:
            s.num_boxes = 0;
            return s;
    }
}

CollisionShape get_bounding_box_shape(uint8_t block_id, uint8_t meta) {
    CollisionShape override = get_selector_override(block_id, meta);
    if (override.num_boxes != 0) return override;
    uint8_t shape_id = get_collision_shape_id(block_id, meta);
    return collision_shapes[shape_id];
}