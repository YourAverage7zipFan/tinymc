#ifndef COLLISION_SHAPES_H
#define COLLISION_SHAPES_H

#include <stdint.h>

// Axis-Aligned Bounding Box
typedef struct {
    float min[3];  // minX, minY, minZ
    float max[3];  // maxX, maxY, maxZ
} AABB;

// Collision shape (can contain multiple AABBs)
typedef struct {
    uint8_t num_boxes;
    AABB boxes[5];
} CollisionShape;

// Total number of collision shapes
#define NUM_COLLISION_SHAPES 82

// Global collision shape lookup table
extern const CollisionShape collision_shapes[NUM_COLLISION_SHAPES];

// Get collision shape ID for a block
// Returns shape ID (0-78), or 0 if block has no collision
uint8_t get_collision_shape_id(uint16_t block_id, uint8_t meta);

CollisionShape get_selector_override(uint8_t block_id, uint8_t meta);
CollisionShape get_bounding_box_shape(uint8_t block_id, uint8_t meta);
//tall grass is a known inaccuracy but im too lazy to care

#endif // COLLISION_SHAPES_H
