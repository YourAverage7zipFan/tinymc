//TODO tmrw

//maybe parse chunk queue in proper order? 

//Renderer lower res when closer to player, higher res when farther
//research what mipmapping is cuz interesting
//chunk meshing queue, x chunk /s or maybe even timed, certain len = continue
//fix textures transparency
//auto apply biome tint in script for generating textures
//certain size triangle = use old renderer?
//inveotnry, block place/break by player
//chunk render dist may be always tied to server, so fix chunk unload and inc buffer to allow for at least 16 render dist
//make chunks only get meshed if within x render dist from player
//actually make the defined render dist values do stuff
//z-sort of triangles, front first so reject quickly
//wireframe mode at certain dist - only way to make it work on old systems
//no texture mode at certain dist
//blob mode at certain dist (just draw blob around size of cube)
//far entities = 2d sprites, close entities = 3d model
//leaves can be made opaque if they are neighbors

// implement inventory struct and packets
// implement block place and break packets - 0x22 - 0x24
#include <stdio.h>
#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#include "varint.h"
#include <stdbool.h>
#include <math.h>
#include "collision_shapes.h"
#include "font6x8.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define MINIZ_NO_STDIO
#define MINIZ_NO_TIME
#define MINIZ_NO_ARCHIVE_APIS
#define MINIZ_NO_ARCHIVE_WRITING_APIS
#define MINIZ_NO_ZLIB_COMPATIBLE_NAMES  
#define MINIZ_NO_MALLOC 

#include "miniz.h"

#include <inttypes.h>
#include <sys/time.h>
#include "blocks_generated.h"
#include "block_properties.h"
#include "items_generated.h"
#include <stdlib.h>

#define RECV_BUF_SIZE (4 * 1024 * 1024)
#define SMALL_BUF_SIZE 8192
#define MAX_LEVEL_TYPE_LEN 32
#define MAX_ENTITIES 1024
#define WORLD_CHUNKS 256 // max render dist 16
#define RENDER_W 640
#define RENDER_H 480

#define BLOCK_MESH_QUEUE_SIZE 3072

#define MAX_INPUT_EVENTS 16

//to add
//certain distance  = wireframe mode
//certain distance = solid block no texture
//certain size of triangle = use old renderer (less initial calculations)

//render block flags
#define RENDER_SKIP_ALL    0x00
#define RENDER_FACE_TOP    0x01
#define RENDER_FACE_BOTTOM 0x02
#define RENDER_FACE_NORTH  0x04
#define RENDER_FACE_SOUTH  0x08
#define RENDER_FACE_EAST   0x10
#define RENDER_FACE_WEST   0x20
#define RENDER_ALL_FACES   0x3F

#define TICK_TIME 50000 // 50000

#define SWAP_INT(a, b) do { int temp = a; a = b; b = temp; } while(0)
#define SWAP_FLOAT(a, b) do { float temp = a; a = b; b = temp; } while(0)

#define FP_SHIFT 20
#define FP_ONE   (1 << FP_SHIFT)

#define INTERP_SHIFT 14
#define INTERP_ONE (1 << INTERP_SHIFT)

uint8_t renderer_distance = 24; // in blocks or chunks depending on mode
int8_t dist_mode = 1; // 0 = chunk player is in, 1 = blocks around player within view dist, 2 = chunks around player within view dist
int8_t dist_before_solid_color = 4; // in blocks from player
int8_t dist_before_wireframe = 8; // in blocks from player
int8_t renderer_res_mul = 17; // triangles closer will have lower resolution. res defined by ceil(renderer_res_mul/dist).

int8_t gui_size_mp = 2;

bool renderer_res_mul_dynamic = false; // if true then the above will apply. don't use this shit it makes it slower

unsigned char recv_buf[RECV_BUF_SIZE];
unsigned char send_pk[SMALL_BUF_SIZE];
unsigned char send_pk_periodic[SMALL_BUF_SIZE];
char debug_menu_buf[1024];

const char *ipname = "152.70.118.6";
const char *usrname = "test";
char my_uuid[37];
const u_short port = 25718;
int recv_len = 0;
int compress_threshold = -1;
int8_t packet_view_distance = 2; // the amount of chunks for the server to send, not nessecarily render
int8_t meshing_distance = 2;
bool running = true;
uint32_t pxbuf[RENDER_W * RENDER_H];
float zbuffer[1]; // old z-buffer kept for ref and for compatability, but will have size 1. To define properly, use RENDER_W * RENDER_H
int zbuffer_new[RENDER_W * RENDER_H]; // fixed point 16.16 for better performance



int pixels_rendered;

int8_t display_info_type; // 0 = nothing, 1 = non technical (just coords), 2 = technical (render times, chunks loaded), 3 = packet feed

float mouse_dx = 0.0f;
float mouse_dy = 0.0f;
float sensitivity = 0.4f;

BITMAPINFO bmi = {0};


uint64_t start_physics_update;
uint64_t last_physics_update, next_physics_update;
int physics_tick_count = 0;

bool mouse_locked = false;

typedef enum {
    STATE_HANDSHAKING = 0,
    STATE_LOGIN = 1,
    STATE_PLAY = 2
} ConnectionState;

ConnectionState current_state = STATE_HANDSHAKING;

typedef struct {
    uint64_t msb; // Most Significant Bits (first 8 bytes)
    uint64_t lsb; // Least Significant Bits (last 8 bytes)
} mc_uuid_t; // uuid storage

//timing helper
uint64_t get_time_us(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000ULL + ts.tv_nsec / 1000;
}

///text renderer
static inline void putpx(uint32_t *fb, int w, int h, int x, int y, uint32_t color) {
    if ((unsigned)x >= (unsigned)w || (unsigned)y >= (unsigned)h)
        return;
    fb[y * w + x] = color;
}

void draw_char6x8_scaled(uint32_t *fb, int fb_w, int fb_h, int x, int y, const uint8_t glyph[6],int scale,uint32_t color) {
    for (int col = 0; col < 6; col++) {
        uint8_t bits = glyph[col];

        for (int row = 0; row < 8; row++) {
            if (bits & (1u << row)) {
                for (int dy = 0; dy < scale; dy++) {
                    for (int dx = 0; dx < scale; dx++) {
                        putpx(
                            fb, fb_w, fb_h,
                            x + col * scale + dx,
                            y + row * scale + dy,
                            color
                        );
                    }
                }
            }
        }
    }
}

void draw_text6x8_scaled(uint32_t *fb, int fb_w, int fb_h,int x, int y,const char *text,int scale,uint32_t color) {
    int cursor_x = x;
    int cursor_y = y;
    while (*text) {
        if (cursor_x > fb_w) {
            cursor_x = x;
            cursor_y += 9 * scale;
        }
        unsigned char c = *text++;

        if (c == '\n') {
            cursor_x = x;
            cursor_y += 9 * scale;
            continue;
        }

        if (c < 32 || c > 127) {
            cursor_x += 6 * scale;
            continue;
        }

        draw_char6x8_scaled(
            fb, fb_w, fb_h,
            cursor_x, cursor_y,
            font6x8_ascii[c],
            scale,
            color
        );

        cursor_x += 6 * scale;
    }
}
void draw_num5x8_scaled(uint32_t *fb, int fb_w, int fb_h,int x, int y,const char *text,int scale,uint32_t color) {
    int cursor_x = x;
    int cursor_y = y;
    while (*text) {
        if (cursor_x > fb_w) {
            cursor_x = x;
            cursor_y += 9 * scale;
        }
        unsigned char c = *text++;

        if (c == '\n') {
            cursor_x = x;
            cursor_y += 9 * scale;
            continue;
        }

        if (c < 32 || c > 127) {
            cursor_x += 5 * scale;
            continue;
        }

        draw_char6x8_scaled(
            fb, fb_w, fb_h,
            cursor_x, cursor_y,
            font6x8_ascii[c],
            scale,
            color
        );

        cursor_x += 5 * scale;
    }
}


//read

uint8_t read_u8(const unsigned char *packet, int *pos) {
    return packet[(*pos)++];
}

int8_t read_i8(const unsigned char *packet, int *pos) {
    return (int8_t)packet[(*pos)++];
}

int32_t read_i32(const unsigned char *packet, int *pos) {
    int32_t val = (packet[*pos] << 24) | (packet[*pos + 1] << 16) | 
                  (packet[*pos + 2] << 8) | packet[*pos + 3];
    *pos += 4;
    return val;
}

int64_t read_i64(const unsigned char *packet, int *pos) {
    int64_t val = ((int64_t)packet[*pos] << 56) | ((int64_t)packet[*pos + 1] << 48) |
                  ((int64_t)packet[*pos + 2] << 40) | ((int64_t)packet[*pos + 3] << 32) |
                  ((int64_t)packet[*pos + 4] << 24) | ((int64_t)packet[*pos + 5] << 16) |
                  ((int64_t)packet[*pos + 6] << 8) | (int64_t)packet[*pos + 7];
    *pos += 8;
    return val;
}

float read_f32(const unsigned char *packet, int *pos) { //f32
    uint32_t val = ((uint32_t)packet[*pos] << 24) |
                   ((uint32_t)packet[*pos + 1] << 16) |
                   ((uint32_t)packet[*pos + 2] << 8) |
                   packet[*pos + 3];
    *pos += 4;

    union {
        uint32_t i;
        float f;
    } conv;
    
    conv.i = val;
    return conv.f;
}

double read_f64(const unsigned char *packet, int *pos) {
    uint64_t val = 
        ((uint64_t)packet[*pos] << 56) |
        ((uint64_t)packet[*pos+1] << 48) |
        ((uint64_t)packet[*pos+2] << 40) |
        ((uint64_t)packet[*pos+3] << 32) |
        ((uint64_t)packet[*pos+4] << 24) |
        ((uint64_t)packet[*pos+5] << 16) |
        ((uint64_t)packet[*pos+6] << 8) |
        ((uint64_t)packet[*pos+7]);
    *pos += 8;

    union {
        uint64_t i;
        double d;
    } conv;
    conv.i = val;
    return conv.d;
}

void read_string(const unsigned char *packet, int max_packet, int *pos, char *out_buf, int max_buf) {
    uint64_t str_len;
    int len_bytes = varint_read_u64(packet + *pos, max_packet - *pos, &str_len);
    *pos += len_bytes;

    if (str_len < (uint64_t)max_buf) {
        memcpy(out_buf, packet + *pos, str_len);
        out_buf[str_len] = '\0';
    }
    *pos += str_len;
}
int16_t read_i16(const unsigned char *packet, int *pos) {
    int16_t res = (int16_t)((packet[*pos] << 8) | packet[*pos + 1]);
    *pos += 2;
    return res;
}
////
////write helpers
int write_u8(unsigned char *buf, uint8_t val) {
    buf[0] = val;
    return 1;
}

int write_i8(unsigned char *buf, int8_t val) { // for readability
    buf[0] = val;
    return 1;
}

int write_i32(unsigned char *buf, int32_t val) {
    buf[0] = (val >> 24) & 0xFF;
    buf[1] = (val >> 16) & 0xFF;
    buf[2] = (val >> 8) & 0xFF;
    buf[3] = val & 0xFF;
    return 4;
}

int write_f32(unsigned char *buf, float val) {
    union {
        float f;
        uint32_t i;
    } conv;
    conv.f = val;
    return write_i32(buf, (int32_t)conv.i); 
}

int write_f64(unsigned char *buf, double val) {
    union {
        double d;
        uint64_t i;
    } conv;
    conv.d = val;
    
    buf[0] = (conv.i >> 56) & 0xFF;
    buf[1] = (conv.i >> 48) & 0xFF;
    buf[2] = (conv.i >> 40) & 0xFF;
    buf[3] = (conv.i >> 32) & 0xFF;
    buf[4] = (conv.i >> 24) & 0xFF;
    buf[5] = (conv.i >> 16) & 0xFF;
    buf[6] = (conv.i >> 8) & 0xFF;
    buf[7] = conv.i & 0xFF;
    return 8;
}

int write_string(unsigned char *buf, const char *str) {
    int len = strlen(str);
    int pos = varint_write_u64(buf, len);
    memcpy(buf + pos, str, len);
    return pos + len;
}
//// 
//// special helpers
mc_uuid_t read_uuid(const unsigned char *packet, int *pos) {
    mc_uuid_t uuid;
    
    // Read 8 bytes as big-endian u64 for MSB
    uuid.msb = ((uint64_t)packet[*pos + 0] << 56) |
               ((uint64_t)packet[*pos + 1] << 48) |
               ((uint64_t)packet[*pos + 2] << 40) |
               ((uint64_t)packet[*pos + 3] << 32) |
               ((uint64_t)packet[*pos + 4] << 24) |
               ((uint64_t)packet[*pos + 5] << 16) |
               ((uint64_t)packet[*pos + 6] << 8)  |
               ((uint64_t)packet[*pos + 7]);
    *pos += 8;
    
    // Read 8 bytes as big-endian u64 for LSB
    uuid.lsb = ((uint64_t)packet[*pos + 0] << 56) |
               ((uint64_t)packet[*pos + 1] << 48) |
               ((uint64_t)packet[*pos + 2] << 40) |
               ((uint64_t)packet[*pos + 3] << 32) |
               ((uint64_t)packet[*pos + 4] << 24) |
               ((uint64_t)packet[*pos + 5] << 16) |
               ((uint64_t)packet[*pos + 6] << 8)  |
               ((uint64_t)packet[*pos + 7]);
    *pos += 8;
    
    return uuid;
}
mc_uuid_t uuid_from_string(const char *str) {
    // Input: "550e8400-e29b-41d4-a716-446655440000"
    mc_uuid_t uuid = {0, 0};
    
    // Remove dashes and parse as two hex numbers
    char hex[33];
    int j = 0;
    for (int i = 0; str[i] && j < 32; i++) {
        if (str[i] != '-') {
            hex[j++] = str[i];
        }
    }
    hex[32] = '\0';
    
    // Parse first 16 hex chars -> msb, last 16 -> lsb
    char msb_str[17], lsb_str[17];
    memcpy(msb_str, hex, 16); msb_str[16] = '\0';
    memcpy(lsb_str, hex + 16, 16); lsb_str[16] = '\0';
    
    uuid.msb = strtoull(msb_str, NULL, 16);
    uuid.lsb = strtoull(lsb_str, NULL, 16);
    
    return uuid;
}
int uuid_equals(mc_uuid_t a, mc_uuid_t b) {
    return a.msb == b.msb && a.lsb == b.lsb;
}
void print_uuid(mc_uuid_t uuid) {
    printf("%08X-%04X-%04X-%04X-%012llX\n",
        (uint32_t)(uuid.msb >> 32),
        (uint16_t)(uuid.msb >> 16),
        (uint16_t)(uuid.msb),
        (uint16_t)(uuid.lsb >> 48),
        (unsigned long long)(uuid.lsb & 0xFFFFFFFFFFFFULL));
}
mc_uuid_t uuid_nil() {
    mc_uuid_t nil = {0, 0};
    return nil;
}
////
//BlockModelType moved to block_properties.h 
typedef struct {
    uint16_t id;
    bool is_full_opaque; // For culling optimization
    // for whatever reason, is_full_opaque is mapped to transparent, which means that for non-full-block-shapes like cross, it will still be true
    // a double check is required for this, comparing to the model of the block
    // actually, I will check this on init and set accordingly, so that the renderer can just check this flag for culling and not have to care about the model
    bool has_transparency; // For rendering order
    BlockModelType model;
    const BlockTextures* text; // Up, Down, N, S, E, W
    int solid_color; // Used if texture_id is -1
    uint16_t collision_shape_id; // Index into collision_shapes[] array
    bool mineable;
    int8_t hardness; // speed of mining, -1 if not mineable
    // int tools_usable[8]; //ids of usable tools for mining, not implemented yet - not used for now
    int8_t stackSize; // max stack size, 1 for non-stackable
    // collision later
} BlockDef;

typedef struct {
    uint16_t id;
    const uint32_t *texture; // can be different sizes. will be cropped to 16x16 for animated textures. only item not 16x16 is slot
    uint32_t solid_color; // Used if texture is null
    int8_t stackSize; // max stack size, 1 for non-stackable
} ItemDef;

BlockDef block_defs[256][16]; // one for each variant
ItemDef item_defs[512][1]; // currently no variants
//

typedef struct {
    float x;
    float y;
    float z;
} Vector3; // used for physics only, not for rendering. 

typedef struct {
    int32_t entity_id;
    int entity_type; // change data type later
    mc_uuid_t uuid; 
    double x, y, z;
    float yaw, pitch;
    double last_x, last_y, last_z; 
    float last_yaw, last_pitch;
    double interp_x, interp_y, interp_z;
    float interp_yaw, interp_pitch;
    int8_t flags;
    int8_t dimension;
} EntityBase;

typedef struct {
    EntityBase base;
    char name[17]; //player name
    uint8_t gamemode;
    int ping;
} Player;

typedef struct {
    EntityBase base;
    float head_pitch;
} MobEntity;

typedef struct {
    EntityBase base;
} NonLivingEntity;

typedef enum {
    ENT_NONE = 0, // Slot is empty
    ENT_PLAYER, // Contains Player
    ENT_GEN, // Contains Generic entity data (EntityBase)
    ENT_MOB, // Contains MobEntity
    ENT_NONLIVING, // Contains NonLivingEntity
} EntityType;

typedef struct {
    EntityType type; // type of entity stored
    int active; // in use flag
    union {
        EntityBase base; // For generic access
        Player player; // Access as player
        MobEntity mob; // Access as mob
        NonLivingEntity nonliving; // Access as non-living entity
    } u; // brooo i gotta name this shii or else compiler yells at me
} GameEntity;

//implement inventory struct 

typedef struct {
    uint16_t id;
    uint16_t meta_or_dmg;
    uint8_t count;
} SlotItem;

typedef struct {
    // 9 for hotbar, 27 for inventory, 4 for armor, 5 for crafting
    SlotItem inv[45];
    // ew no nbt data :vomit: :vomit: :vomit:
} Inventory;

typedef struct {
    uint8_t window_id;
    //54 for container, 36 for p inventory
    SlotItem inv[90];
    // ew no nbt data :vomit: :vomit: :vomit:
} StorageBlock;

typedef struct {
    SlotItem inv[1];
    // ew no nbt data :vomit: :vomit: :vomit:
} HoldingItem;

typedef struct {
    uint8_t type; // 1 = key, 2 = mouse button, 0 = inactive
    uint8_t code; // keycode or mouse button code - 0 = lb, 1 = rb, 2 = mb, keyboard is just keycodes
    uint8_t action; // keypress = 0 (that is the only thing necessary rn)
} InputEvent;

typedef struct {
    Player base;
    int hp;
    int hunger;
    int8_t invulnerable;
    int8_t flying;
    int8_t allow_flying;
    int8_t creative_mode;
    float fly_speed;
    float walk_speed;
    int8_t selected_slot;
    double x_vel;
    double y_vel;
    double z_vel;
    bool w_key_down, a_key_down, s_key_down, d_key_down, space_key_down, shift_key_down, ctrl_key_down;
    float base_move_speed; 
    float sprint_multiplier; 
    float friction_multiplier;
    float crouch_multiplier;
    float jump_vel;
    float gravity;
    bool on_ground;
    bool inited;
    double last_pk_x, last_pk_y, last_pk_z;
    float last_pk_yaw, last_pk_pitch;

    Inventory inventory;

    StorageBlock active_container;
    HoldingItem holding_item;

    bool inventory_open;
    bool chest_open;
    uint8_t open_chest_window_id;
    int chest_x, chest_y, chest_z;

    int cursor_on_block_x, cursor_on_block_y, cursor_on_block_z;
    bool is_cursor_on_block;

    InputEvent input_events[MAX_INPUT_EVENTS];
    bool rmb_down;
    bool lmb_down;
    int tick_at_rmb_down, tick_at_lmb_down;
    // inventory would go here
} ClientPlayer;

typedef struct {
    int32_t x;
    int32_t z;
    uint16_t bitmap;
} ChunkMeta;

typedef struct {
    uint16_t id; 
    uint8_t meta : 4; // 0-15
    uint8_t render_flags; // faces of block to draw
    //uint8_t light : 4;    // 0-15
    //uint8_t sky_light : 4; // 0-15
    // ^ light can be implemented later
} Block;

typedef struct {
    Block blocks[256][16][16]; // [y][z][x]
} ChunkBlocks;

typedef struct {
    ChunkMeta meta;
    ChunkBlocks data;
    int8_t dimension;
    int active;
    int initialized;
    int meshed;
} ChunkData;

ChunkData world[WORLD_CHUNKS];

typedef struct {
    int x, y, z;
    int active;
} BlockMeshQueueItem;

BlockMeshQueueItem block_mesh_queue[BLOCK_MESH_QUEUE_SIZE];

///world helpers

ChunkData* get_chunk(int x, int z) {
    for(int i = 0; i < WORLD_CHUNKS; i++) {
        if(world[i].active && world[i].meta.x == x && world[i].meta.z == z) {
            return &world[i];
        }
    }
    return NULL;
}

ChunkData* allocate_chunk(int x, int z) {
    ChunkData* existing = get_chunk(x, z);
    if (existing) return existing;
    for(int i = 0; i < WORLD_CHUNKS; i++) {
        if (!world[i].active) {
            world[i].active = 1;
            world[i].meta.x = x;
            world[i].meta.z = z;
            return &world[i];
        }
    }
    return NULL; 
}

void clear_chunk_data(ChunkData* chunk) {
    for (int y = 0; y < 256; y++) {
        for (int z = 0; z < 16; z++) {
            for (int x = 0; x < 16; x++) {
                chunk->data.blocks[y][z][x].id = 0;
                chunk->data.blocks[y][z][x].meta = 0;
                chunk->data.blocks[y][z][x].render_flags = 0x0; 
            }
        }
    }
}
void clear_chunk_sector_data(ChunkData* chunk, int sector) {
    for (int y = sector*16; y < (sector + 1) * 16; y++) {
        for (int z = 0; z < 16; z++) {
            for (int x = 0; x < 16; x++) {
                chunk->data.blocks[y][z][x].id = 0;
                chunk->data.blocks[y][z][x].meta = 0;
                chunk->data.blocks[y][z][x].render_flags = 0x0; 
            }
        }
    }
}

Block* get_block_xyz(int x, int y, int z) {
    int chunk_x = x >> 4;
    int chunk_z = z >> 4;
    if (y < 0 || y >= 256) {
        return NULL; // out of bounds
    }
    ChunkData *chunk = get_chunk(chunk_x, chunk_z);
    if (chunk == NULL) {
        return NULL; // chunk not found
    }
    return &chunk->data.blocks[y][z & 15][x & 15];
}

int set_block_xyz(int x, int y, int z, uint16_t block_id, uint8_t block_meta) {
    int chunk_x = x >> 4;
    int chunk_z = z >> 4;
    if (y < 0 || y >= 256) {
        return 0; // out of bounds
    }
    ChunkData *chunk = get_chunk(chunk_x, chunk_z);
    if (chunk == NULL) {
        return 0; // chunk not found
    }
    bool block_to_n_opaque, block_to_s_opaque, block_to_e_opaque, block_to_w_opaque, block_above_opaque, block_below_opaque;
    if ((z & 15) + 1 < 16){
        block_to_s_opaque = block_defs[chunk->data.blocks[y][(z & 15) + 1][x & 15].id][0].is_full_opaque;
    } else {
        ChunkData *south_chunk = get_chunk(chunk_x, chunk_z + 1);
        if (south_chunk) {
            block_to_s_opaque = south_chunk->data.blocks[y][0][x & 15].id != 0 && block_defs[south_chunk->data.blocks[y][0][x & 15].id][0].is_full_opaque;
        } else {
            block_to_s_opaque = false; // assume non-opaque if chunk not loaded
        }
    }
    if ((z & 15) - 1 >= 0){
        block_to_n_opaque = block_defs[chunk->data.blocks[y][(z & 15) - 1][x & 15].id][0].is_full_opaque;
    } else {
        ChunkData *north_chunk = get_chunk(chunk_x, chunk_z - 1);
        if (north_chunk) {
            block_to_n_opaque = north_chunk->data.blocks[y][15][x & 15].id != 0 && block_defs[north_chunk->data.blocks[y][15][x & 15].id][0].is_full_opaque;
        } else {
            block_to_n_opaque = false; // assume non-opaque if chunk not loaded
        }
    }
    if ((x & 15) + 1 < 16){
        block_to_e_opaque = block_defs[chunk->data.blocks[y][z & 15][(x & 15) + 1].id][0].is_full_opaque;
    } else {
        ChunkData *east_chunk = get_chunk(chunk_x + 1, chunk_z);
        if (east_chunk) {
            block_to_e_opaque = east_chunk->data.blocks[y][z & 15][0].id != 0 && block_defs[east_chunk->data.blocks[y][z & 15][0].id][0].is_full_opaque;
        } else {
            block_to_e_opaque = false; // assume non-opaque if chunk not loaded
        }
    }
    if ((x & 15) - 1 >= 0){
        block_to_w_opaque = block_defs[chunk->data.blocks[y][z & 15][(x & 15) - 1].id][0].is_full_opaque;
    } else {
        ChunkData *west_chunk = get_chunk(chunk_x - 1, chunk_z);
        if (west_chunk) {
            block_to_w_opaque = west_chunk->data.blocks[y][z & 15][15].id != 0 && block_defs[west_chunk->data.blocks[y][z & 15][15].id][0].is_full_opaque;
        } else {
            block_to_w_opaque = false; // assume non-opaque if chunk not loaded
        }
    }
    if (y + 1 < 256){
        block_above_opaque = block_defs[chunk->data.blocks[y + 1][z & 15][x & 15].id][0].is_full_opaque;
    } else {
        block_above_opaque = false; // assume non-opaque if out of bounds
    }
    if (y - 1 >= 0){
        block_below_opaque = block_defs[chunk->data.blocks[y - 1][z & 15][x & 15].id][0].is_full_opaque;
    } else {
        block_below_opaque = false; // assume non-opaque if out of bounds
    }
    uint8_t render_flags = 0;
    if (!block_to_n_opaque) render_flags |= RENDER_FACE_NORTH;
    if (!block_to_s_opaque) render_flags |= RENDER_FACE_SOUTH;
    if (!block_to_e_opaque) render_flags |= RENDER_FACE_EAST;
    if (!block_to_w_opaque) render_flags |= RENDER_FACE_WEST;
    if (!block_above_opaque) render_flags |= RENDER_FACE_TOP;
    if (!block_below_opaque) render_flags |= RENDER_FACE_BOTTOM;
    Block block = {block_id, block_meta, render_flags};
    chunk->data.blocks[y][z & 15][x & 15] = block;
    return 1;
}

int set_block_cxcz_r(int cx, int cz, int rx, int ry, int rz, uint16_t block_id, uint8_t block_meta) {
    if (ry < 0 || ry >= 256) {
        return 0; // out of bounds
    }
    ChunkData *chunk = get_chunk(cx, cz);
    if (chunk == NULL) {
        return 0; // chunk not found
    }
    bool block_to_n_opaque, block_to_s_opaque, block_to_e_opaque, block_to_w_opaque, block_above_opaque, block_below_opaque;
    if ((rz & 15) + 1 < 16){
        block_to_s_opaque = block_defs[chunk->data.blocks[ry][(rz) + 1][rx & 15].id][0].is_full_opaque;
    } else {
        ChunkData *south_chunk = get_chunk(cx, cz + 1);
        if (south_chunk) {
            block_to_s_opaque = south_chunk->data.blocks[ry][0][rx & 15].id != 0 && block_defs[south_chunk->data.blocks[ry][0][rx & 15].id][0].is_full_opaque;
        } else {
            block_to_s_opaque = false; // assume non-opaque if chunk not loaded
        }
    }
    if ((rz & 15) - 1 >= 0){
        block_to_n_opaque = block_defs[chunk->data.blocks[ry][(rz & 15) - 1][rx & 15].id][0].is_full_opaque;
    } else {
        ChunkData *north_chunk = get_chunk(cx, cz - 1);
        if (north_chunk) {
            block_to_n_opaque = north_chunk->data.blocks[ry][15][rx & 15].id != 0 && block_defs[north_chunk->data.blocks[ry][15][rx & 15].id][0].is_full_opaque;
        } else {
            block_to_n_opaque = false; // assume non-opaque if chunk not loaded
        }
    }
    if ((rx & 15) + 1 < 16){
        block_to_e_opaque = block_defs[chunk->data.blocks[ry][rz & 15][(rx & 15) + 1].id][0].is_full_opaque;
    } else {
        ChunkData *east_chunk = get_chunk(cx + 1, cz);
        if (east_chunk) {
            block_to_e_opaque = east_chunk->data.blocks[ry][rz & 15][0].id != 0 && block_defs[east_chunk->data.blocks[ry][rz & 15][0].id][0].is_full_opaque;
        } else {
            block_to_e_opaque = false; // assume non-opaque if chunk not loaded
        }
    }
    if ((rx & 15) - 1 >= 0){
        block_to_w_opaque = block_defs[chunk->data.blocks[ry][rz & 15][(rx & 15) - 1].id][0].is_full_opaque;
    } else {
        ChunkData *west_chunk = get_chunk(cx - 1, cz);
        if (west_chunk) {
            block_to_w_opaque = west_chunk->data.blocks[ry][rz & 15][15].id != 0 && block_defs[west_chunk->data.blocks[ry][rz & 15][15].id][0].is_full_opaque;
        } else {
            block_to_w_opaque = false; // assume non-opaque if chunk not loaded
        }
    }
    if (ry + 1 < 256){
        block_above_opaque = block_defs[chunk->data.blocks[ry + 1][rz & 15][rx & 15].id][0].is_full_opaque;
    } else {
        block_above_opaque = false; // assume non-opaque if out of bounds
    }
    if (ry - 1 >= 0){
        block_below_opaque = block_defs[chunk->data.blocks[ry - 1][rz & 15][rx & 15].id][0].is_full_opaque;
    } else {
        block_below_opaque = false; // assume non-opaque if out of bounds
    }
    uint8_t render_flags = 0;
    if (!block_to_n_opaque) render_flags |= RENDER_FACE_NORTH;
    if (!block_to_s_opaque) render_flags |= RENDER_FACE_SOUTH;
    if (!block_to_e_opaque) render_flags |= RENDER_FACE_EAST;
    if (!block_to_w_opaque) render_flags |= RENDER_FACE_WEST;
    if (!block_above_opaque) render_flags |= RENDER_FACE_TOP;
    if (!block_below_opaque) render_flags |= RENDER_FACE_BOTTOM;
    Block block = {block_id, block_meta, render_flags};
    chunk->data.blocks[ry][rz][rx] = block;
    return 1;
}

///

GameEntity entity_arena[MAX_ENTITIES];

//// game entity helpers
GameEntity *get_entity(int32_t id) {
    for (int i = 0; i < MAX_ENTITIES; i++) {
        if (entity_arena[i].active && entity_arena[i].u.base.entity_id == id) {
            return &entity_arena[i];
        }
    }
    return NULL;
}
GameEntity *allocate_entity(int32_t id, EntityType type) {
    GameEntity *existing = get_entity(id);
    if (existing) {
        existing->active = 1;
        existing->type = type;
        return existing;
    }
    for (int i = 0; i < MAX_ENTITIES; i++) {
        if (!entity_arena[i].active) {
            entity_arena[i].active = 1;
            entity_arena[i].type = type;
            entity_arena[i].u.base.entity_id = id;
            return &entity_arena[i];
        }
    }
    printf("ERROR: Max entities reached!\n");
    return NULL;
}
void destroy_entity(int32_t id) {
    GameEntity *e = get_entity(id);
    if (e) {
        e->active = 0;
        e->type = ENT_NONE;
    }
}
////

//// 3d math helpers and structs
typedef struct {
    float x, y, z;
} vec3;
typedef struct {
    float m[16];
} mat4;
typedef struct {
    float x, y, z, w;
} vec4;
mat4 mat4_perspective(mat4 out, float fovy, float aspect, float near_, float far_) {
    float f = 1.0 / tanf(fovy / 2);
    float nf = 1 / (near_ - far_);
    out.m[0] = f / aspect;
    out.m[1] = 0;
    out.m[2] = 0;
    out.m[3] = 0;
    out.m[4] = 0;
    out.m[5] = f;
    out.m[6] = 0;
    out.m[7] = 0;
    out.m[8] = 0;
    out.m[9] = 0;
    out.m[10] = (far_ + near_) * nf;
    out.m[11] = -1;
    out.m[12] = 0;
    out.m[13] = 0;
    out.m[14] = (2 * far_ * near_) * nf;
    out.m[15] = 0;
    return out;
}
mat4 mat4_identity(mat4 out) {
    out.m[0] = 1;
    out.m[1] = 0;
    out.m[2] = 0;
    out.m[3] = 0;
    out.m[4] = 0;
    out.m[5] = 1;
    out.m[6] = 0;
    out.m[7] = 0;
    out.m[8] = 0;
    out.m[9] = 0;
    out.m[10] = 1;
    out.m[11] = 0;
    out.m[12] = 0;
    out.m[13] = 0;
    out.m[14] = 0;
    out.m[15] = 1;
    return out;
}

mat4 mat4_lookAt(mat4 out, float eye[3], float center[3], float up[3]) {
    float x0, x1, x2, y0, y1, y2, z0, z1, z2, len,
        eyex = eye[0],
        eyey = eye[1],
        eyez = eye[2],
        upx = up[0],
        upy = up[1],
        upz = up[2],
        centerx = center[0],
        centery = center[1],
        centerz = center[2];

    if (fabs(eyex - centerx) < 0.000001 &&
        fabs(eyey - centery) < 0.000001 &&
        fabs(eyez - centerz) < 0.000001) {
        return mat4_identity(out);
    }

    z0 = eyex - centerx;
    z1 = eyey - centery;
    z2 = eyez - centerz;

    len = 1 / sqrtf(z0 * z0 + z1 * z1 + z2 * z2);
    z0 *= len;
    z1 *= len;
    z2 *= len;

    x0 = upy * z2 - upz * z1;
    x1 = upz * z0 - upx * z2;
    x2 = upx * z1 - upy * z0;
    len = sqrt(x0 * x0 + x1 * x1 + x2 * x2);
    if (!len) {
        x0 = 0;
        x1 = 0;
        x2 = 0;
    } else {
        len = 1 / len;
        x0 *= len;
        x1 *= len;
        x2 *= len;
    }

    y0 = z1 * x2 - z2 * x1;
    y1 = z2 * x0 - z0 * x2;
    y2 = z0 * x1 - z1 * x0;

    len = sqrt(y0 * y0 + y1 * y1 + y2 * y2);
    if (!len) {
        y0 = 0;
        y1 = 0;
        y2 = 0;
    } else {
        len = 1 / len;
        y0 *= len;
        y1 *= len;
        y2 *= len;
    }

    out.m[0] = x0;
    out.m[1] = y0;
    out.m[2] = z0;
    out.m[3] = 0;
    out.m[4] = x1;
    out.m[5] = y1;
    out.m[6] = z1;
    out.m[7] = 0;
    out.m[8] = x2;
    out.m[9] = y2;
    out.m[10] = z2;
    out.m[11] = 0;
    out.m[12] = -(x0 * eyex + x1 * eyey + x2 * eyez);
    out.m[13] = -(y0 * eyex + y1 * eyey + y2 * eyez);
    out.m[14] = -(z0 * eyex + z1 * eyey + z2 * eyez);
    out.m[15] = 1;

    return out;
}

vec4 mat4_mul_vec4(mat4 m, vec4 v) {
    vec4 res;
    
    // Column 0 * x
    res.x  = m.m[0] * v.x;
    res.y  = m.m[1] * v.x;
    res.z  = m.m[2] * v.x;
    res.w  = m.m[3] * v.x;

    // Column 1 * y
    res.x += m.m[4] * v.y;
    res.y += m.m[5] * v.y;
    res.z += m.m[6] * v.y;
    res.w += m.m[7] * v.y;

    // Column 2 * z
    res.x += m.m[8] * v.z;
    res.y += m.m[9] * v.z;
    res.z += m.m[10] * v.z;
    res.w += m.m[11] * v.z;

    // Column 3 * w
    res.x += m.m[12] * v.w;
    res.y += m.m[13] * v.w;
    res.z += m.m[14] * v.w;
    res.w += m.m[15] * v.w;

    return res;
}
int get_coords_from_3d(int *screen_x, int *screen_y, float *depth, float x, float y, float z, mat4 vp) {
    vec4 world_pos = {x, y, z, 1.0f};
    vec4 clip = mat4_mul_vec4(vp, world_pos);
    if (clip.w > 0) {
        float ndc_x = clip.x / clip.w;
        float ndc_y = clip.y / clip.w;
        *screen_x = (int)((ndc_x + 1.0f) * 0.5f * RENDER_W);
        *screen_y = (int)((1.0f - ndc_y) * 0.5f * RENDER_H);
        *depth = clip.w;   // used to be just clip.w
        
        return 1;
    }
    return 0;
}
mat4 mat4_mul(mat4 a, mat4 b) {
    mat4 out;
    for (int col = 0; col < 4; col++) {
        for (int row = 0; row < 4; row++) {
            float sum = 0.0f;
            for (int k = 0; k < 4; k++) {
                // out[col][row] = a[k][row] * b[col][k]
                sum += a.m[k * 4 + row] * b.m[col * 4 + k];
            }
            out.m[col * 4 + row] = sum;
        }
    }
    return out;
}





int min3(int a, int b, int c) { return (a < b) ? ((a < c) ? a : c) : ((b < c) ? b : c); }
int max3(int a, int b, int c) { return (a > b) ? ((a > c) ? a : c) : ((b > c) ? b : c); }
int edge_function(int ax, int ay, int bx, int by, int px, int py) {
    return (px - ax) * (by - ay) - (py - ay) * (bx - ax);
}
void draw_triangle_z(int x0, int y0, float z0, 
                     int x1, int y1, float z1, 
                     int x2, int y2, float z2, 
                     uint32_t color) {
    int min_x = min3(x0, x1, x2);
    int min_y = min3(y0, y1, y2);
    int max_x = max3(x0, x1, x2);
    int max_y = max3(y0, y1, y2);

    // Screen Clipping
    min_x = (min_x < 0) ? 0 : min_x;
    min_y = (min_y < 0) ? 0 : min_y;
    max_x = (max_x >= RENDER_W) ? RENDER_W - 1 : max_x;
    max_y = (max_y >= RENDER_H) ? RENDER_H - 1 : max_y;

    // Edge function area (triangle area * 2)
    float area = (float)edge_function(x0, y0, x1, y1, x2, y2);
    
    // Back-face culling (optional but good for speed)
    if (area <= 0) return; 

    for (int y = min_y; y <= max_y; y++) {
        for (int x = min_x; x <= max_x; x++) {
            // Barycentric weights
            float w0 = edge_function(x1, y1, x2, y2, x, y);
            float w1 = edge_function(x2, y2, x0, y0, x, y);
            float w2 = edge_function(x0, y0, x1, y1, x, y);

            if (w0 >= 0 && w1 >= 0 && w2 >= 0) {
                // Normalize weights
                w0 /= area; 
                w1 /= area; 
                w2 /= area;

                // Interpolate Z (1/w interpolation is technically more correct for perspective, 
                // but linear interpolation of W usually looks "ok" for blocky voxel games)
                float z = w0 * z0 + w1 * z1 + w2 * z2;

                int idx = x + y * RENDER_W;
                
                if (z < zbuffer[idx]) {
                    zbuffer[idx] = z;
                    pxbuf[idx] = color;
                }
            }
        }
    }
}
void draw_triangle(int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color) {
    int min_x = min3(x0, x1, x2);
    int min_y = min3(y0, y1, y2);
    int max_x = max3(x0, x1, x2);
    int max_y = max3(y0, y1, y2);
    if (min_x < 0) min_x = 0;
    if (min_y < 0) min_y = 0;
    if (max_x >= RENDER_W) max_x = RENDER_W - 1;
    if (max_y >= RENDER_H) max_y = RENDER_H - 1;

    // Iterate pixels in bounding box
    for (int y = min_y; y <= max_y; y++) {
        for (int x = min_x; x <= max_x; x++) {
            int w0 = edge_function(x1, y1, x2, y2, x, y);
            int w1 = edge_function(x2, y2, x0, y0, x, y);
            int w2 = edge_function(x0, y0, x1, y1, x, y);

            // If all positive, we are inside
            if (w0 >= 0 && w1 >= 0 && w2 >= 0) {
                pxbuf[x + y * RENDER_W] = color;
            }
        }
    }
}
void draw_quad(int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3, uint32_t color) {
    draw_triangle(x0, y0, x1, y1, x2, y2, color);
    draw_triangle(x0, y0, x2, y2, x3, y3, color);
}
void draw_quad_z(int x0, int y0, float z0, int x1, int y1, float z1, int x2, int y2, float z2, int x3, int y3, float z3, uint32_t color) {
    draw_triangle_z(x0, y0, z0, x1, y1, z1, x2, y2, z2, color);
    draw_triangle_z(x0, y0, z0, x2, y2, z2, x3, y3, z3, color);
}

void draw_triangle_tex_z(int x0, int y0, float z0, float u0, float v0,
                         int x1, int y1, float z1, float u1, float v1,
                         int x2, int y2, float z2, float u2, float v2,
                         const uint32_t *texture) {
    if (!texture) return; 
    
    int min_x = min3(x0, x1, x2);
    int min_y = min3(y0, y1, y2);
    int max_x = max3(x0, x1, x2);
    int max_y = max3(y0, y1, y2);

    min_x = (min_x < 0) ? 0 : min_x;
    min_y = (min_y < 0) ? 0 : min_y;
    max_x = (max_x >= RENDER_W) ? RENDER_W - 1 : max_x;
    max_y = (max_y >= RENDER_H) ? RENDER_H - 1 : max_y;

    float area = (float)edge_function(x0, y0, x1, y1, x2, y2);
    
    if (area <= 0) return; 

    // Precompute 1/z and perspective-correct UV for each vertex
    float invz0 = 1.0f / z0;
    float invz1 = 1.0f / z1;
    float invz2 = 1.0f / z2;
    float u0_z = u0 * invz0;
    float v0_z = v0 * invz0;
    float u1_z = u1 * invz1;
    float v1_z = v1 * invz1;
    float u2_z = u2 * invz2;
    float v2_z = v2 * invz2;

    for (int y = min_y; y <= max_y; y++) {
        for (int x = min_x; x <= max_x; x++) {
            float w0 = edge_function(x1, y1, x2, y2, x, y);
            float w1 = edge_function(x2, y2, x0, y0, x, y);
            float w2 = edge_function(x0, y0, x1, y1, x, y);

            if (w0 >= 0 && w1 >= 0 && w2 >= 0) {
                w0 /= area; 
                w1 /= area; 
                w2 /= area;

                float z = w0 * z0 + w1 * z1 + w2 * z2;

                int idx = x + y * RENDER_W;
                
                if (z < zbuffer[idx]) {
                    // Perspective-correct UV interpolation
                    float invz = w0 * invz0 + w1 * invz1 + w2 * invz2;
                    float u = (w0 * u0_z + w1 * u1_z + w2 * u2_z) / invz;
                    float v = (w0 * v0_z + w1 * v1_z + w2 * v2_z) / invz;
                    
                    if (u < 0.0f) u = 0.0f;
                    if (u > 1.0f) u = 1.0f;
                    if (v < 0.0f) v = 0.0f;
                    if (v > 1.0f) v = 1.0f;
                    
                    // Sample texture 
                    int tex_x = (int)(u * 15.0f); // 0-15
                    int tex_y = (int)(v * 15.0f); // 0-15
                    uint32_t tex_color = texture[tex_y * 16 + tex_x];
                    
                    zbuffer[idx] = z;
                    pxbuf[idx] = tex_color;
                }
            }
        }
    }
}

void draw_triangle_sp_tex_z(int x0, int y0, float z0, float u0, float v0,
                           int x1, int y1, float z1, float u1, float v1,
                           int x2, int y2, float z2, float u2, float v2,
                           const uint32_t *texture) {
    if (!texture) return;
    
    // sort Y vertices
    if (y0 > y1) { 
        SWAP_INT(x0, x1); SWAP_INT(y0, y1); 
        SWAP_FLOAT(z0, z1); SWAP_FLOAT(u0, u1); SWAP_FLOAT(v0, v1);
    }
    if (y1 > y2) { 
        SWAP_INT(x1, x2); SWAP_INT(y1, y2); 
        SWAP_FLOAT(z1, z2); SWAP_FLOAT(u1, u2); SWAP_FLOAT(v1, v2);
    }
    if (y0 > y1) { 
        SWAP_INT(x0, x1); SWAP_INT(y0, y1); 
        SWAP_FLOAT(z0, z1); SWAP_FLOAT(u0, u1); SWAP_FLOAT(v0, v1);
    }
    
    int total_height = y2 - y0;
    if (total_height == 0) return; // Degenerate triangle (flat line)
    
    for (int y = y0; y <= y2; y++) {
        if (y < 0 || y >= RENDER_H) continue;
        
        // Determine which half of triangle 
        bool second_half = (y >= y1);
        int segment_height = second_half ? (y2 - y1) : (y1 - y0);
        if (segment_height == 0) continue; 
        
        // interpolation factor along long edge
        float alpha = (float)(y - y0) / (float)total_height;
        
        // interpolation factor along short edge
        float beta = second_half ? (float)(y - y1) / (float)segment_height
                                 : (float)(y - y0) / (float)segment_height;
        
        int xa = x0 + (int)((x2 - x0) * alpha);
        float za = z0 + (z2 - z0) * alpha;
        float ua = u0 + (u2 - u0) * alpha;
        float va = v0 + (v2 - v0) * alpha;
        
        // Short edge v0-v1 or v1-v2 depending on half
        int xb;
        float zb, ub, vb;
        if (second_half) {
            // v1-v2
            xb = x1 + (int)((x2 - x1) * beta);
            zb = z1 + (z2 - z1) * beta;
            ub = u1 + (u2 - u1) * beta;
            vb = v1 + (v2 - v1) * beta;
        } else {
            // v0-v1
            xb = x0 + (int)((x1 - x0) * beta);
            zb = z0 + (z1 - z0) * beta;
            ub = u0 + (u1 - u0) * beta;
            vb = v0 + (v1 - v0) * beta;
        }
        if (xa > xb) {
            SWAP_INT(xa, xb);
            SWAP_FLOAT(za, zb);
            SWAP_FLOAT(ua, ub);
            SWAP_FLOAT(va, vb);
        }
        int x_start = (xa < 0) ? 0 : xa;
        int x_end = (xb >= RENDER_W) ? RENDER_W - 1 : xb;
        
        // Rasterize horizontal span 
        int span_width = xb - xa;
        if (span_width == 0) {
            if (x_start >= 0 && x_start < RENDER_W) {
                int idx = x_start + y * RENDER_W;
                if (za < zbuffer_new[idx]) {
                    int tex_x = (int)(ua * 15.0f) & 15;
                    int tex_y = (int)(va * 15.0f) & 15;
                    zbuffer_new[idx] = za;
                    pxbuf[idx] = texture[tex_y * 16 + tex_x];
                }
            }
            continue;
        }
        
        for (int x = x_start; x <= x_end; x++) {
            float t = (float)(x - xa) / (float)span_width;
            
            float z = za + (zb - za) * t;
            float u = ua + (ub - ua) * t;
            float v = va + (vb - va) * t;
            
            int idx = x + y * RENDER_W;
            
            if (z < zbuffer_new[idx]) {
                //clamp uv
                if (u < 0.0f) u = 0.0f;
                if (u > 1.0f) u = 1.0f;
                if (v < 0.0f) v = 0.0f;
                if (v > 1.0f) v = 1.0f;
                int tex_x = (int)(u * 15.0f) & 15;
                int tex_y = (int)(v * 15.0f) & 15;
                zbuffer_new[idx] = z;
                pxbuf[idx] = texture[tex_y * 16 + tex_x];
            }
        }
    }
}

void draw_triangle_tex_z_optimized_old(int x0, int y0, float z0, float u0, float v0,
                                   int x1, int y1, float z1, float u1, float v1,
                                   int x2, int y2, float z2, float u2, float v2,
                                   const uint32_t *texture) {
    if (!texture) return;
    
    int min_x = min3(x0, x1, x2);
    int min_y = min3(y0, y1, y2);
    int max_x = max3(x0, x1, x2);
    int max_y = max3(y0, y1, y2);

    min_x = (min_x < 0) ? 0 : min_x;
    min_y = (min_y < 0) ? 0 : min_y;
    max_x = (max_x >= RENDER_W) ? RENDER_W - 1 : max_x;
    max_y = (max_y >= RENDER_H) ? RENDER_H - 1 : max_y;

    int area_int = edge_function(x0, y0, x1, y1, x2, y2);
    if (area_int == 0) return;

    int sign = (area_int > 0) ? 1 : -1;
    float area = (float)(area_int * sign);

    int inv_area = (FP_ONE / area);

    float invz0 = 1.0f / z0;
    float invz1 = 1.0f / z1;
    float invz2 = 1.0f / z2;
    float u0_z = u0 * invz0;
    float v0_z = v0 * invz0;
    float u1_z = u1 * invz1;
    float v1_z = v1 * invz1;
    float u2_z = u2 * invz2;
    float v2_z = v2 * invz2;
    int A12 = y2 - y1;
    int B12 = x1 - x2;
    int A20 = y0 - y2;
    int B20 = x2 - x0;
    int A01 = y1 - y0;
    int B01 = x0 - x1;
    A12 *= sign; B12 *= sign;
    A20 *= sign; B20 *= sign;
    A01 *= sign; B01 *= sign;
    int w0_row = edge_function(x1, y1, x2, y2, min_x, min_y) * sign;
    int w1_row = edge_function(x2, y2, x0, y0, min_x, min_y) * sign;
    int w2_row = edge_function(x0, y0, x1, y1, min_x, min_y) * sign;
    int W0 = (int)(invz0 * FP_ONE);
    int W1 = (int)(invz1 * FP_ONE);
    int W2 = (int)(invz2 * FP_ONE);
    int U0 = (int)(u0_z * FP_ONE);
    int U1 = (int)(u1_z * FP_ONE);
    int U2 = (int)(u2_z * FP_ONE);
    int V0 = (int)(v0_z * FP_ONE);
    int V1 = (int)(v1_z * FP_ONE);
    int V2 = (int)(v2_z * FP_ONE);
    
    for (int y = min_y; y <= max_y; y++) {
        int w0 = w0_row;
        int w1 = w1_row;
        int w2 = w2_row;
        
        for (int x = min_x; x <= max_x; x++) {
            // Check if pixel is inside triangle
            /*if (w0 >= 0 && w1 >= 0 && w2 >= 0) {
                float fw0 = (float)w0 / area;
                float fw1 = (float)w1 / area;
                float fw2 = (float)w2 / area;
                float invz = fw0 * invz0 + fw1 * invz1 + fw2 * invz2;
                int idx = x + y * RENDER_W;
                if (invz > zbuffer[idx]) {
                    float u = (fw0 * u0_z + fw1 * u1_z + fw2 * u2_z) / invz;
                    float v = (fw0 * v0_z + fw1 * v1_z + fw2 * v2_z) / invz;
                    
                    if (u < 0.0f) u = 0.0f;
                    if (u > 1.0f) u = 1.0f;
                    if (v < 0.0f) v = 0.0f;
                    if (v > 1.0f) v = 1.0f;
                    
                    int tex_x = (int)(u * 16.0f);
                    if (tex_x == 16) tex_x = 15;

                    int tex_y = (int)(v * 16.0f);
                    if (tex_y == 16) tex_y = 15;
                    
                    zbuffer[idx] = invz;
                    pxbuf[idx] = texture[tex_y * 16 + tex_x];
                }
            }*/
            if (w0 >= 0 && w1 >= 0 && w2 >= 0) {
                int64_t W = ((int64_t)w0 * W0 + (int64_t)w1 * W1 + (int64_t)w2 * W2);
                

                W = (W * inv_area) >> FP_SHIFT;
                int idx = x + y * RENDER_W;

                if (W > zbuffer_new[idx]) {
                    zbuffer_new[idx] = W;
                    int U = (w0 * U0 + w1 * U1 + w2 * U2) * inv_area;
                    int V = (w0 * V0 + w1 * V1 + w2 * V2) * inv_area;

                    int u_fp = (U << FP_SHIFT) / W;
                    int v_fp = (V << FP_SHIFT) / W;

                    int tex_x = (u_fp * 16) >> FP_SHIFT;
                    int tex_y = (v_fp * 16) >> FP_SHIFT;

                    pxbuf[idx] = texture[(tex_y << 4) | tex_x];
                }
            }

            w0 += A12;
            w1 += A20;
            w2 += A01;
        }
        w0_row += B12;
        w1_row += B20;
        w2_row += B01;
    }
}

void draw_triangle_tex_z_optimized_almost_old(int x0, int y0, float z0, float u0, float v0,
                                   int x1, int y1, float z1, float u1, float v1,
                                   int x2, int y2, float z2, float u2, float v2,
                                   const uint32_t *texture) {
    if (!texture) return;

    
    int min_x = min3(x0, x1, x2);
    int min_y = min3(y0, y1, y2);
    int max_x = max3(x0, x1, x2);
    int max_y = max3(y0, y1, y2);

    min_x = (min_x < 0) ? 0 : min_x;
    min_y = (min_y < 0) ? 0 : min_y;
    max_x = (max_x >= RENDER_W) ? RENDER_W - 1 : max_x;
    max_y = (max_y >= RENDER_H) ? RENDER_H - 1 : max_y;

    int area_int = edge_function(x0, y0, x1, y1, x2, y2);
    if (area_int == 0) return;

    if (area_int < 0) {
        // swap vertices 1 and 2
        SWAP_INT(x1, x2);
        SWAP_INT(y1, y2);
        SWAP_FLOAT(z1, z2);
        SWAP_FLOAT(u1, u2);
        SWAP_FLOAT(v1, v2);
    }

    int sign = 1;//(area_int > 0) ? 1 : -1;

    float invz0 = 1.0f / z0;
    float invz1 = 1.0f / z1;
    float invz2 = 1.0f / z2;
    float u0_z = (u0 * 16.0f) * invz0;
    float v0_z = (v0 * 16.0f) * invz0;
    float u1_z = (u1 * 16.0f) * invz1;
    float v1_z = (v1 * 16.0f) * invz1;
    float u2_z = (u2 * 16.0f) * invz2;
    float v2_z = (v2 * 16.0f) * invz2;
    
    int dW0dx =  (y2 - y1) * sign;
    int dW0dy = -(x2 - x1) * sign;
    int dW1dx =  (y0 - y2) * sign;
    int dW1dy = -(x0 - x2) * sign;
    int dW2dx =  (y1 - y0) * sign;
    int dW2dy = -(x1 - x0) * sign;

    int w0_row = edge_function(x1, y1, x2, y2, min_x, min_y) * sign;
    int w1_row = edge_function(x2, y2, x0, y0, min_x, min_y) * sign;
    int w2_row = edge_function(x0, y0, x1, y1, min_x, min_y) * sign;

    int edge_bias0 = ((y2 < y1) || (y2 == y1 && x2 < x1)) ? 0 : -1;
    int edge_bias1 = ((y0 < y2) || (y0 == y2 && x0 < x2)) ? 0 : -1;
    int edge_bias2 = ((y1 < y0) || (y1 == y0 && x1 < x0)) ? 0 : -1;

    w0_row += edge_bias0;
    w1_row += edge_bias1;
    w2_row += edge_bias2;
    
    
    int W0 = (int)(invz0 * INTERP_ONE);
    int W1 = (int)(invz1 * INTERP_ONE);
    int W2 = (int)(invz2 * INTERP_ONE);
    int U0 = (int)(u0_z * INTERP_ONE);
    int U1 = (int)(u1_z * INTERP_ONE);
    int U2 = (int)(u2_z * INTERP_ONE);
    int V0 = (int)(v0_z * INTERP_ONE);
    int V1 = (int)(v1_z * INTERP_ONE);
    int V2 = (int)(v2_z * INTERP_ONE);

    int dx01 = x1 - x0, dy01 = y1 - y0;
    int dx02 = x2 - x0, dy02 = y2 - y0;

    int det = dx01 * dy02 - dy01 * dx02;
    if (det == 0) return;
    det *= sign;

    // Gradients for perspective-correct interpolation
    int dWdx = (int)(((long long)(W1 - W0) * dy02 - (long long)(W2 - W0) * dy01) / det);
    int dWdy = (int)((-(long long)(W1 - W0) * dx02 + (long long)(W2 - W0) * dx01) / det);

    int dUdx = (int)(((long long)(U1 - U0) * dy02 - (long long)(U2 - U0) * dy01) / det);
    int dUdy = (int)((-(long long)(U1 - U0) * dx02 + (long long)(U2 - U0) * dx01) / det);

    int dVdx = (int)(((long long)(V1 - V0) * dy02 - (long long)(V2 - V0) * dy01) / det);
    int dVdy = (int)((-(long long)(V1 - V0) * dx02 + (long long)(V2 - V0) * dx01) / det);

    int start_x = min_x;
    int start_y = min_y;

    int dx = start_x - x0;
    int dy = start_y - y0;

    int W_row = W0 + dx * dWdx + dy * dWdy;
    int U_row = U0 + dx * dUdx + dy * dUdy;
    int V_row = V0 + dx * dVdx + dy * dVdy;


    for (int y = min_y; y <= max_y; y++) {
        int w0 = w0_row;
        int w1 = w1_row;
        int w2 = w2_row;

        int W = W_row;
        printf("W = %d\n", W);
        int U = U_row;
        int V = V_row;

        for (int x = min_x; x <= max_x; x++) {
            
            if (w0 >= 0 && w1 >= 0 && w2 >= 0) {
                int idx = x + y * RENDER_W;
                if (W > zbuffer_new[idx]) {
                    int u_fp = (int)(((long long)U << INTERP_SHIFT) / W);
                    int v_fp = (int)(((long long)V << INTERP_SHIFT) / W);
                    int tx = u_fp >> INTERP_SHIFT;
                    int ty = v_fp >> INTERP_SHIFT;
                    tx &= 15; 
                    ty &= 15;

                    zbuffer_new[idx] = W;
                    pxbuf[idx] = texture[(ty << 4) | tx];
                }
            }
            w0 += dW0dx;
            w1 += dW1dx;
            w2 += dW2dx;
            W += dWdx;
            U += dUdx;
            V += dVdx;
        }

        w0_row += dW0dy;
        w1_row += dW1dy;
        w2_row += dW2dy;
        W_row += dWdy;
        U_row += dUdy;
        V_row += dVdy;
    }
}

void draw_triangle_z_optimized(int x0, int y0, float z0,
                                int x1, int y1, float z1,
                                int x2, int y2, float z2,
                                uint32_t color) {
    int min_x = min3(x0, x1, x2);
    int min_y = min3(y0, y1, y2);
    int max_x = max3(x0, x1, x2);
    int max_y = max3(y0, y1, y2);
    min_x = (min_x < 0) ? 0 : min_x;
    min_y = (min_y < 0) ? 0 : min_y;
    max_x = (max_x >= RENDER_W) ? RENDER_W - 1 : max_x;
    max_y = (max_y >= RENDER_H) ? RENDER_H - 1 : max_y;
    int area_int = edge_function(x0, y0, x1, y1, x2, y2);
    if (area_int == 0) return;
    if (area_int < 0) {
        SWAP_INT(x1, x2);
        SWAP_INT(y1, y2);
        SWAP_FLOAT(z1, z2);
        area_int = -area_int;
    }
    int sign = 1;
    float invz0 = 1.0f / z0;
    float invz1 = 1.0f / z1;
    float invz2 = 1.0f / z2;
    int W0 = (int)(invz0 * INTERP_ONE);
    int W1 = (int)(invz1 * INTERP_ONE);
    int W2 = (int)(invz2 * INTERP_ONE);
    int dW0dx =  (y2 - y1) * sign;
    int dW0dy = -(x2 - x1) * sign;
    int dW1dx =  (y0 - y2) * sign;
    int dW1dy = -(x0 - x2) * sign;
    int dW2dx =  (y1 - y0) * sign;
    int dW2dy = -(x1 - x0) * sign;
    int w0_row = edge_function(x1, y1, x2, y2, min_x, min_y) * sign;
    int w1_row = edge_function(x2, y2, x0, y0, min_x, min_y) * sign;
    int w2_row = edge_function(x0, y0, x1, y1, min_x, min_y) * sign;
    int edge_bias0 = ((y2 < y1) || (y2 == y1 && x2 < x1)) ? 0 : -1;
    int edge_bias1 = ((y0 < y2) || (y0 == y2 && x0 < x2)) ? 0 : -1;
    int edge_bias2 = ((y1 < y0) || (y1 == y0 && x1 < x0)) ? 0 : -1;
    w0_row += edge_bias0;
    w1_row += edge_bias1;
    w2_row += edge_bias2;
    int dx01 = x1 - x0, dy01 = y1 - y0;
    int dx02 = x2 - x0, dy02 = y2 - y0;
    int det = dx01 * dy02 - dy01 * dx02;
    if (det == 0) return;
    det *= sign;
    int dWdx = (int)(((long long)(W1 - W0) * dy02 - (long long)(W2 - W0) * dy01) / det);
    int dWdy = (int)((-(long long)(W1 - W0) * dx02 + (long long)(W2 - W0) * dx01) / det);
    int dx = min_x - x0;
    int dy = min_y - y0;
    int W_row = W0 + dx * dWdx + dy * dWdy;
    for (int y = min_y; y <= max_y; y++) {
        int x = min_x;
        int w0 = w0_row, w1 = w1_row, w2 = w2_row;
        int W = W_row;
        while (x <= max_x && (w0 < 0 || w1 < 0 || w2 < 0)) {
            w0 += dW0dx; w1 += dW1dx; w2 += dW2dx;
            W += dWdx;
            x++;
        }
        int x_start = x;
        while (x <= max_x && (w0 >= 0 && w1 >= 0 && w2 >= 0)) {
            w0 += dW0dx; w1 += dW1dx; w2 += dW2dx;
            W += dWdx;
            x++;
        }
        int x_end = x - 1;
        if (x_start <= x_end) {
            int idx = y * RENDER_W + x_start;
            int dx_span = x_start - min_x;
            W = W_row + dx_span * dWdx;

            for (int x = x_start; x <= x_end; x++) {
                if (W > zbuffer_new[idx]) {
                    zbuffer_new[idx] = W;
                    pxbuf[idx] = color;
                    pixels_rendered++;
                }
                idx++;
                W += dWdx;
            }
        }
        w0_row += dW0dy;
        w1_row += dW1dy;
        w2_row += dW2dy;
        W_row += dWdy;
    }
}
void draw_line_z_fast(int x0, int y0, int W0,
                       int x1, int y1, int W1,
                       uint32_t color) {
    int dx = abs(x1 - x0);
    int dy = abs(y1 - y0);
    
    if (dx == 0 && dy == 0) return; // Skip degenerate lines
    
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;
    int steps = (dx > dy) ? dx : dy;
    int dW = (W1 - W0) / steps;

    int x = x0;
    int y = y0;
    int W = W0;
    int err = dx - dy;
    //int stride = sy * RENDER_W + sx;

    // Clip the entire line - if completely outside, skip it
    if ((x0 < 0 && x1 < 0) || (x0 >= RENDER_W && x1 >= RENDER_W) ||
        (y0 < 0 && y1 < 0) || (y0 >= RENDER_H && y1 >= RENDER_H)) {
        return;
    }

    for (int step = 0; step <= steps; step++) {
        // Only bounds check, don't branch on it every pixel if possible
        if ((unsigned)x < RENDER_W && (unsigned)y < RENDER_H) {
            int idx = y * RENDER_W + x;
            if (W > zbuffer_new[idx]) {
                zbuffer_new[idx] = W;
                pxbuf[idx] = color;
                pixels_rendered++;
            }
        }

        if (step == steps) break;

        int e2 = err << 1;
        if (e2 > -dy) {
            err -= dy;
            x += sx;
        }
        if (e2 < dx) {
            err += dx;
            y += sy;
        }
        W += dW;
    }
}

void draw_triangle_wireframe(int x0, int y0, float z0,
                               int x1, int y1, float z1,
                               int x2, int y2, float z2,
                               uint32_t color) {
    // Pre-compute W values once
    int W0 = (int)((1.0f / z0) * INTERP_ONE);
    int W1 = (int)((1.0f / z1) * INTERP_ONE);
    int W2 = (int)((1.0f / z2) * INTERP_ONE);
    
    draw_line_z_fast(x0, y0, W0, x1, y1, W1, color);
    draw_line_z_fast(x1, y1, W1, x2, y2, W2, color);
    draw_line_z_fast(x2, y2, W2, x0, y0, W0, color);
}


void draw_triangle_tex_z_optimized_bearly_old(int x0, int y0, float z0, float u0, float v0,
                                   int x1, int y1, float z1, float u1, float v1,
                                   int x2, int y2, float z2, float u2, float v2,
                                   const uint32_t *texture) {
    if (!texture) return;

    
    int min_x = min3(x0, x1, x2);
    int min_y = min3(y0, y1, y2);
    int max_x = max3(x0, x1, x2);
    int max_y = max3(y0, y1, y2);

    min_x = (min_x < 0) ? 0 : min_x;
    min_y = (min_y < 0) ? 0 : min_y;
    max_x = (max_x >= RENDER_W) ? RENDER_W - 1 : max_x;
    max_y = (max_y >= RENDER_H) ? RENDER_H - 1 : max_y;

    int area_int = edge_function(x0, y0, x1, y1, x2, y2);
    if (area_int == 0) return;

    if (area_int < 0) {
        // swap vertices 1 and 2
        SWAP_INT(x1, x2);
        SWAP_INT(y1, y2);
        SWAP_FLOAT(z1, z2);
        SWAP_FLOAT(u1, u2);
        SWAP_FLOAT(v1, v2);
    }

    int sign = 1;//(area_int > 0) ? 1 : -1;

    float invz0 = 1.0f / z0;
    float invz1 = 1.0f / z1;
    float invz2 = 1.0f / z2;
    float u0_z = (u0 * 16.0f) * invz0;
    float v0_z = (v0 * 16.0f) * invz0;
    float u1_z = (u1 * 16.0f) * invz1;
    float v1_z = (v1 * 16.0f) * invz1;
    float u2_z = (u2 * 16.0f) * invz2;
    float v2_z = (v2 * 16.0f) * invz2;
    
    int dW0dx =  (y2 - y1) * sign;
    int dW0dy = -(x2 - x1) * sign;
    int dW1dx =  (y0 - y2) * sign;
    int dW1dy = -(x0 - x2) * sign;
    int dW2dx =  (y1 - y0) * sign;
    int dW2dy = -(x1 - x0) * sign;

    int w0_row = edge_function(x1, y1, x2, y2, min_x, min_y) * sign;
    int w1_row = edge_function(x2, y2, x0, y0, min_x, min_y) * sign;
    int w2_row = edge_function(x0, y0, x1, y1, min_x, min_y) * sign;

    int edge_bias0 = ((y2 < y1) || (y2 == y1 && x2 < x1)) ? 0 : -1;
    int edge_bias1 = ((y0 < y2) || (y0 == y2 && x0 < x2)) ? 0 : -1;
    int edge_bias2 = ((y1 < y0) || (y1 == y0 && x1 < x0)) ? 0 : -1;

    w0_row += edge_bias0;
    w1_row += edge_bias1;
    w2_row += edge_bias2;
    
    
    int W0 = (int)(invz0 * INTERP_ONE);
    int W1 = (int)(invz1 * INTERP_ONE);
    int W2 = (int)(invz2 * INTERP_ONE);
    int U0 = (int)(u0_z * INTERP_ONE);
    int U1 = (int)(u1_z * INTERP_ONE);
    int U2 = (int)(u2_z * INTERP_ONE);
    int V0 = (int)(v0_z * INTERP_ONE);
    int V1 = (int)(v1_z * INTERP_ONE);
    int V2 = (int)(v2_z * INTERP_ONE);

    int dx01 = x1 - x0, dy01 = y1 - y0;
    int dx02 = x2 - x0, dy02 = y2 - y0;

    int det = dx01 * dy02 - dy01 * dx02;
    if (det == 0) return;
    det *= sign;

    // Gradients for perspective-correct interpolation
    int dWdx = (int)(((long long)(W1 - W0) * dy02 - (long long)(W2 - W0) * dy01) / det);
    int dWdy = (int)((-(long long)(W1 - W0) * dx02 + (long long)(W2 - W0) * dx01) / det);

    int dUdx = (int)(((long long)(U1 - U0) * dy02 - (long long)(U2 - U0) * dy01) / det);
    int dUdy = (int)((-(long long)(U1 - U0) * dx02 + (long long)(U2 - U0) * dx01) / det);

    int dVdx = (int)(((long long)(V1 - V0) * dy02 - (long long)(V2 - V0) * dy01) / det);
    int dVdy = (int)((-(long long)(V1 - V0) * dx02 + (long long)(V2 - V0) * dx01) / det);

    int start_x = min_x;
    int start_y = min_y;

    int dx = start_x - x0;
    int dy = start_y - y0;

    int W_row = W0 + dx * dWdx + dy * dWdy;
    int U_row = U0 + dx * dUdx + dy * dUdy;
    int V_row = V0 + dx * dVdx + dy * dVdy;


    for (int y = min_y; y <= max_y; y++) {

        int x = min_x;
        int w0 = w0_row, w1 = w1_row, w2 = w2_row;
        int W = W_row, U = U_row, V = V_row;

        /* find entry */
        while (x <= max_x && (w0 < 0 || w1 < 0 || w2 < 0)) {
            w0 += dW0dx; w1 += dW1dx; w2 += dW2dx;
            W += dWdx; U += dUdx; V += dVdx;
            x++;
        }

        int x_start = x;

        /* find exit */
        while (x <= max_x && (w0 >= 0 && w1 >= 0 && w2 >= 0)) {
            w0 += dW0dx; w1 += dW1dx; w2 += dW2dx;
            W += dWdx; U += dUdx; V += dVdx;
            x++;
        }

        int x_end = x - 1;

        if (x_start <= x_end) {
            int idx = y * RENDER_W + x_start;

            int dx = x_start - min_x;

            w0 = w0_row + dx * dW0dx;
            w1 = w1_row + dx * dW1dx;
            w2 = w2_row + dx * dW2dx;

            W  = W_row  + dx * dWdx;
            U  = U_row  + dx * dUdx;
            V  = V_row  + dx * dVdx;

            for (int x = x_start; x <= x_end; x++) {
                if (W > zbuffer_new[idx]) {
                    int u_fp = (int)(((long long)U << INTERP_SHIFT) / W);
                    int v_fp = (int)(((long long)V << INTERP_SHIFT) / W);
                    int tx = u_fp >> INTERP_SHIFT;
                    int ty = v_fp >> INTERP_SHIFT;
                    tx &= 15; 
                    ty &= 15;

                    uint32_t pixel = texture[(ty << 4) | tx];
                    if ((pixel >> 24) & 0xFF) { //check alpha
                        zbuffer_new[idx] = W;
                        pxbuf[idx] = pixel & 0xFFFFFF;
                    }
                }
                idx++;
                W += dWdx; U += dUdx; V += dVdx;
            }
        }
        w0_row += dW0dy;
        w1_row += dW1dy;
        w2_row += dW2dy;

        W_row += dWdy;
        U_row += dUdy;
        V_row += dVdy;
    }
}


void draw_triangle_tex_z_optimized(int x0, int y0, float z0, float u0, float v0,
                                   int x1, int y1, float z1, float u1, float v1,
                                   int x2, int y2, float z2, float u2, float v2,
                                   const uint32_t *texture) {
    if (!texture) return;
    float furthest_z = z0;
    if (z1 > furthest_z) furthest_z = z1;
    if (z2 > furthest_z) furthest_z = z2;
    
    int res_step = (int)ceilf(10.0f / furthest_z);
    if (res_step < 1) res_step = 1;
    
    int min_x = min3(x0, x1, x2);
    int min_y = min3(y0, y1, y2);
    int max_x = max3(x0, x1, x2);
    int max_y = max3(y0, y1, y2);

    min_x = (min_x < 0) ? 0 : min_x;
    min_y = (min_y < 0) ? 0 : min_y;
    max_x = (max_x >= RENDER_W) ? RENDER_W - 1 : max_x;
    max_y = (max_y >= RENDER_H) ? RENDER_H - 1 : max_y;

    int area_int = edge_function(x0, y0, x1, y1, x2, y2);
    if (area_int == 0) return;

    if (area_int < 0) {
        // swap vertices 1 and 2
        SWAP_INT(x1, x2);
        SWAP_INT(y1, y2);
        SWAP_FLOAT(z1, z2);
        SWAP_FLOAT(u1, u2);
        SWAP_FLOAT(v1, v2);
    }

    int sign = 1;//(area_int > 0) ? 1 : -1;

    float invz0 = 1.0f / z0;
    float invz1 = 1.0f / z1;
    float invz2 = 1.0f / z2;
    float u0_z = (u0 * 16.0f) * invz0;
    float v0_z = (v0 * 16.0f) * invz0;
    float u1_z = (u1 * 16.0f) * invz1;
    float v1_z = (v1 * 16.0f) * invz1;
    float u2_z = (u2 * 16.0f) * invz2;
    float v2_z = (v2 * 16.0f) * invz2;
    
    int dW0dx =  (y2 - y1) * sign * res_step;
    int dW0dy = -(x2 - x1) * sign * res_step;
    int dW1dx =  (y0 - y2) * sign * res_step;
    int dW1dy = -(x0 - x2) * sign * res_step;
    int dW2dx =  (y1 - y0) * sign * res_step;
    int dW2dy = -(x1 - x0) * sign * res_step;

    int w0_row = edge_function(x1, y1, x2, y2, min_x, min_y) * sign;
    int w1_row = edge_function(x2, y2, x0, y0, min_x, min_y) * sign;
    int w2_row = edge_function(x0, y0, x1, y1, min_x, min_y) * sign;

    int edge_bias0 = ((y2 < y1) || (y2 == y1 && x2 < x1)) ? 0 : -1;
    int edge_bias1 = ((y0 < y2) || (y0 == y2 && x0 < x2)) ? 0 : -1;
    int edge_bias2 = ((y1 < y0) || (y1 == y0 && x1 < x0)) ? 0 : -1;

    w0_row += edge_bias0;
    w1_row += edge_bias1;
    w2_row += edge_bias2;
    
    
    int W0 = (int)(invz0 * INTERP_ONE);
    int W1 = (int)(invz1 * INTERP_ONE);
    int W2 = (int)(invz2 * INTERP_ONE);
    int U0 = (int)(u0_z * INTERP_ONE);
    int U1 = (int)(u1_z * INTERP_ONE);
    int U2 = (int)(u2_z * INTERP_ONE);
    int V0 = (int)(v0_z * INTERP_ONE);
    int V1 = (int)(v1_z * INTERP_ONE);
    int V2 = (int)(v2_z * INTERP_ONE);

    int dx01 = x1 - x0, dy01 = y1 - y0;
    int dx02 = x2 - x0, dy02 = y2 - y0;

    int det = dx01 * dy02 - dy01 * dx02;
    if (det == 0) return;
    det *= sign;

    int dWdx = (int)(((long long)(W1 - W0) * dy02 - (long long)(W2 - W0) * dy01) / det);
    int dWdy = (int)((-(long long)(W1 - W0) * dx02 + (long long)(W2 - W0) * dx01) / det);

    int dUdx = (int)(((long long)(U1 - U0) * dy02 - (long long)(U2 - U0) * dy01) / det);
    int dUdy = (int)((-(long long)(U1 - U0) * dx02 + (long long)(U2 - U0) * dx01) / det);

    int dVdx = (int)(((long long)(V1 - V0) * dy02 - (long long)(V2 - V0) * dy01) / det);
    int dVdy = (int)((-(long long)(V1 - V0) * dx02 + (long long)(V2 - V0) * dx01) / det);
    
    int dWdx_step = dWdx * res_step;
    int dWdy_step = dWdy * res_step;
    int dUdx_step = dUdx * res_step;
    int dUdy_step = dUdy * res_step;
    int dVdx_step = dVdx * res_step;
    int dVdy_step = dVdy * res_step;

    int start_x = min_x;
    int start_y = min_y;

    int dx = start_x - x0;
    int dy = start_y - y0;

    int W_row = W0 + dx * dWdx + dy * dWdy;
    int U_row = U0 + dx * dUdx + dy * dUdy;
    int V_row = V0 + dx * dVdx + dy * dVdy;


    for (int y = min_y; y <= max_y; y += res_step) {

        int x = min_x;
        int w0 = w0_row, w1 = w1_row, w2 = w2_row;
        int W = W_row, U = U_row, V = V_row;
        while (x <= max_x && (w0 < 0 || w1 < 0 || w2 < 0)) {
            w0 += dW0dx; w1 += dW1dx; w2 += dW2dx;
            W += dWdx_step; U += dUdx_step; V += dVdx_step;
            x += res_step;
        }
        int x_start = x;
        while (x <= max_x && (w0 >= 0 && w1 >= 0 && w2 >= 0)) {
            w0 += dW0dx; w1 += dW1dx; w2 += dW2dx;
            W += dWdx_step; U += dUdx_step; V += dVdx_step;
            x += res_step;
        }

        int x_end = x - res_step;

        if (x_start <= x_end) {
            int idx = y * RENDER_W + x_start;

            int dx = x_start - min_x;

            w0 = w0_row + dx * dW0dx;
            w1 = w1_row + dx * dW1dx;
            w2 = w2_row + dx * dW2dx;

            W  = W_row  + dx * dWdx;
            U  = U_row  + dx * dUdx;
            V  = V_row  + dx * dVdx;

            for (int x = x_start; x <= x_end; x += res_step) {
                if (W > zbuffer_new[idx]) {
                    int u_fp = (int)(((long long)U << INTERP_SHIFT) / W);
                    int v_fp = (int)(((long long)V << INTERP_SHIFT) / W);
                    int tx = u_fp >> INTERP_SHIFT;
                    int ty = v_fp >> INTERP_SHIFT;
                    tx &= 15; 
                    ty &= 15;

                    uint32_t pixel = texture[(ty << 4) | tx];
                    if ((pixel >> 24) & 0xFF) {
                        uint32_t color = pixel & 0xFFFFFF;
                        
                        switch(res_step) { // faster than for loop
                            case 1:
                                zbuffer_new[idx] = W;
                                pxbuf[idx] = color;
                                break;
                                
                            case 2:
                                if (x + 1 <= max_x && y + 1 <= max_y) {
                                    int i1 = idx;
                                    int i2 = idx + RENDER_W;
                                    zbuffer_new[i1] = zbuffer_new[i1 + 1] = W;
                                    zbuffer_new[i2] = zbuffer_new[i2 + 1] = W;
                                    pxbuf[i1] = pxbuf[i1 + 1] = color;
                                    pxbuf[i2] = pxbuf[i2 + 1] = color;
                                } else {
                                    for (int by = 0; by < 2 && (y + by) <= max_y; by++) {
                                        for (int bx = 0; bx < 2 && (x + bx) <= max_x; bx++) {
                                            int fill_idx = (y + by) * RENDER_W + (x + bx);
                                            zbuffer_new[fill_idx] = W;
                                            pxbuf[fill_idx] = color;
                                        }
                                    }
                                }
                                break;
                                
                            case 3:
                                if (x + 2 <= max_x && y + 2 <= max_y) {
                                    int i = idx;
                                    zbuffer_new[i] = zbuffer_new[i+1] = zbuffer_new[i+2] = W;
                                    pxbuf[i] = pxbuf[i+1] = pxbuf[i+2] = color;
                                    i += RENDER_W;
                                    zbuffer_new[i] = zbuffer_new[i+1] = zbuffer_new[i+2] = W;
                                    pxbuf[i] = pxbuf[i+1] = pxbuf[i+2] = color;
                                    i += RENDER_W;
                                    zbuffer_new[i] = zbuffer_new[i+1] = zbuffer_new[i+2] = W;
                                    pxbuf[i] = pxbuf[i+1] = pxbuf[i+2] = color;
                                } else {
                                    for (int by = 0; by < 3 && (y + by) <= max_y; by++) {
                                        for (int bx = 0; bx < 3 && (x + bx) <= max_x; bx++) {
                                            int fill_idx = (y + by) * RENDER_W + (x + bx);
                                            zbuffer_new[fill_idx] = W;
                                            pxbuf[fill_idx] = color;
                                        }
                                    }
                                }
                                break;
                                
                            case 4:
                                if (x + 3 <= max_x && y + 3 <= max_y) {
                                    int i = idx;
                                    zbuffer_new[i] = zbuffer_new[i+1] = zbuffer_new[i+2] = zbuffer_new[i+3] = W;
                                    pxbuf[i] = pxbuf[i+1] = pxbuf[i+2] = pxbuf[i+3] = color;
                                    i += RENDER_W;
                                    zbuffer_new[i] = zbuffer_new[i+1] = zbuffer_new[i+2] = zbuffer_new[i+3] = W;
                                    pxbuf[i] = pxbuf[i+1] = pxbuf[i+2] = pxbuf[i+3] = color;
                                    i += RENDER_W;
                                    zbuffer_new[i] = zbuffer_new[i+1] = zbuffer_new[i+2] = zbuffer_new[i+3] = W;
                                    pxbuf[i] = pxbuf[i+1] = pxbuf[i+2] = pxbuf[i+3] = color;
                                    i += RENDER_W;
                                    zbuffer_new[i] = zbuffer_new[i+1] = zbuffer_new[i+2] = zbuffer_new[i+3] = W;
                                    pxbuf[i] = pxbuf[i+1] = pxbuf[i+2] = pxbuf[i+3] = color;
                                } else {
                                    for (int by = 0; by < 4 && (y + by) <= max_y; by++) {
                                        for (int bx = 0; bx < 4 && (x + bx) <= max_x; bx++) {
                                            int fill_idx = (y + by) * RENDER_W + (x + bx);
                                            zbuffer_new[fill_idx] = W;
                                            pxbuf[fill_idx] = color;
                                        }
                                    }
                                }
                                break;
                            case 5:
                                if (x + 4 <= max_x && y + 4 <= max_y) {
                                    int i = idx;
                                    zbuffer_new[i] = zbuffer_new[i+1] = zbuffer_new[i+2] = zbuffer_new[i+3] = zbuffer_new[i+4] = W;
                                    pxbuf[i] = pxbuf[i+1] = pxbuf[i+2] = pxbuf[i+3] = pxbuf[i+4] = color;
                                    i += RENDER_W;
                                    zbuffer_new[i] = zbuffer_new[i+1] = zbuffer_new[i+2] = zbuffer_new[i+3] = zbuffer_new[i+4] = W;
                                    pxbuf[i] = pxbuf[i+1] = pxbuf[i+2] = pxbuf[i+3] = pxbuf[i+4] = color;
                                    i += RENDER_W;
                                    zbuffer_new[i] = zbuffer_new[i+1] = zbuffer_new[i+2] = zbuffer_new[i+3] = zbuffer_new[i+4] = W;
                                    pxbuf[i] = pxbuf[i+1] = pxbuf[i+2] = pxbuf[i+3] = pxbuf[i+4] = color;
                                    i += RENDER_W;
                                    zbuffer_new[i] = zbuffer_new[i+1] = zbuffer_new[i+2] = zbuffer_new[i+3] = zbuffer_new[i+4] = W;
                                    pxbuf[i] = pxbuf[i+1] = pxbuf[i+2] = pxbuf[i+3] = pxbuf[i+4] = color;
                                    i += RENDER_W;
                                    zbuffer_new[i] = zbuffer_new[i+1] = zbuffer_new[i+2] = zbuffer_new[i+3] = zbuffer_new[i+4] = W;
                                    pxbuf[i] = pxbuf[i+1] = pxbuf[i+2] = pxbuf[i+3] = pxbuf[i+4] = color;
                                } else {
                                    for (int by = 0; by < res_step && (y + by) <= max_y; by++) {
                                        for (int bx = 0; bx < res_step && (x + bx) <= max_x; bx++) {
                                            int fill_idx = (y + by) * RENDER_W + (x + bx);
                                            zbuffer_new[fill_idx] = W;
                                            pxbuf[fill_idx] = color;
                                        }
                                    }
                                }
                                break;
                            case 6:
                                if (x + 5 <= max_x && y + 5 <= max_y) {
                                    int i = idx;
                                    zbuffer_new[i] = zbuffer_new[i+1] = zbuffer_new[i+2] = zbuffer_new[i+3] = zbuffer_new[i+4] = zbuffer_new[i+5] = W;
                                    pxbuf[i] = pxbuf[i+1] = pxbuf[i+2] = pxbuf[i+3] = pxbuf[i+4] = pxbuf[i+5] = color;
                                    i += RENDER_W;
                                    zbuffer_new[i] = zbuffer_new[i+1] = zbuffer_new[i+2] = zbuffer_new[i+3] = zbuffer_new[i+4] = zbuffer_new[i+5] = W;
                                    pxbuf[i] = pxbuf[i+1] = pxbuf[i+2] = pxbuf[i+3] = pxbuf[i+4] = pxbuf[i+5] = color;
                                    i += RENDER_W;
                                    zbuffer_new[i] = zbuffer_new[i+1] = zbuffer_new[i+2] = zbuffer_new[i+3] = zbuffer_new[i +4] = zbuffer_new[i+5] = W;
                                    pxbuf[i] = pxbuf[i+1] = pxbuf[i+2] = pxbuf[i+3] = pxbuf[i+4] = pxbuf[i+5] = color;
                                    i += RENDER_W;
                                    zbuffer_new[i] = zbuffer_new[i+1] = zbuffer_new[i+2] = zbuffer_new[i+3] = zbuffer_new[i+4] = zbuffer_new[i+5] = W;
                                    pxbuf[i] = pxbuf[i+1] = pxbuf[i+2] = pxbuf[i+3] = pxbuf[i+4] = pxbuf[i+5] = color;
                                    i += RENDER_W;
                                    zbuffer_new[i] = zbuffer_new[i+1] = zbuffer_new[i+2] = zbuffer_new[i+3] = zbuffer_new[i+4] = zbuffer_new[i+5] = W;
                                    pxbuf[i] = pxbuf[i+1] = pxbuf[i+2] = pxbuf[i+3] = pxbuf[i+4] = pxbuf[i+5] = color;
                                    i += RENDER_W;
                                    zbuffer_new[i] = zbuffer_new[i+1] = zbuffer_new[i+2] = zbuffer_new[i+3] = zbuffer_new[i+4] = zbuffer_new[i+5] = W;
                                    pxbuf[i] = pxbuf[i+1] = pxbuf[i+2] = pxbuf[i+3] = pxbuf[i+4] = pxbuf[i+5] = color;
                                } else {
                                    for (int by = 0; by < res_step && (y + by) <= max_y; by++) {
                                        for (int bx = 0; bx < res_step && (x + bx) <= max_x; bx++) {
                                            int fill_idx = (y + by) * RENDER_W + (x + bx);
                                            zbuffer_new[fill_idx] = W;
                                            pxbuf[fill_idx] = color;
                                        }
                                    }
                                }
                                break;
                            default:
                                for (int by = 0; by < res_step && (y + by) <= max_y; by++) {
                                    for (int bx = 0; bx < res_step && (x + bx) <= max_x; bx++) {
                                        int fill_idx = (y + by) * RENDER_W + (x + bx);
                                        zbuffer_new[fill_idx] = W;
                                        pxbuf[fill_idx] = color;
                                    }
                                }
                        }
                    }
                }
                idx += res_step;
                W += dWdx_step; U += dUdx_step; V += dVdx_step;
            }
        }
        w0_row += dW0dy;
        w1_row += dW1dy;
        w2_row += dW2dy;

        W_row += dWdy_step;
        U_row += dUdy_step;
        V_row += dVdy_step;
    }
}


void draw_quad_tex_z(int x0, int y0, float z0, float u0, float v0,
                     int x1, int y1, float z1, float u1, float v1,
                     int x2, int y2, float z2, float u2, float v2,
                     int x3, int y3, float z3, float u3, float v3,
                     const uint32_t *texture) {
    if (renderer_res_mul_dynamic){
        draw_triangle_tex_z_optimized(x0, y0, z0, u0, v0,
                            x1, y1, z1, u1, v1,
                            x2, y2, z2, u2, v2,
                            texture);
        draw_triangle_tex_z_optimized(x0, y0, z0, u0, v0,
                            x2, y2, z2, u2, v2,
                            x3, y3, z3, u3, v3,
                            texture);
    } else {
        draw_triangle_tex_z_optimized_bearly_old(x0, y0, z0, u0, v0,
                            x1, y1, z1, u1, v1,
                            x2, y2, z2, u2, v2,
                            texture);
        draw_triangle_tex_z_optimized_bearly_old(x0, y0, z0, u0, v0,
                            x2, y2, z2, u2, v2,
                            x3, y3, z3, u3, v3,
                            texture);
    }
}
void draw_quad_z_optimized(int x0, int y0, float z0,
                     int x1, int y1, float z1,
                     int x2, int y2, float z2,
                     int x3, int y3, float z3, uint32_t color) {
    draw_triangle_z_optimized(x0, y0, z0,
                            x1, y1, z1,
                            x2, y2, z2, color);
    draw_triangle_z_optimized(x0, y0, z0,
                            x2, y2, z2,
                            x3, y3, z3, color);
}
void draw_quad_z_wire(int x0, int y0, float z0,
                     int x1, int y1, float z1,
                     int x2, int y2, float z2,
                     int x3, int y3, float z3, uint32_t color) {
    draw_triangle_wireframe(x0, y0, z0,
                            x1, y1, z1,
                            x2, y2, z2, color);
    draw_triangle_wireframe(x0, y0, z0,
                            x2, y2, z2,
                            x3, y3, z3, color);
}
void draw_quad_sp_tex_z(int x0, int y0, float z0, float u0, float v0,
                     int x1, int y1, float z1, float u1, float v1,
                     int x2, int y2, float z2, float u2, float v2,
                     int x3, int y3, float z3, float u3, float v3,
                     const uint32_t *texture) {
    draw_triangle_sp_tex_z(x0, y0, z0, u0, v0,
                        x1, y1, z1, u1, v1,
                        x2, y2, z2, u2, v2,
                        texture);
    draw_triangle_sp_tex_z(x0, y0, z0, u0, v0,
                        x2, y2, z2, u2, v2,
                        x3, y3, z3, u3, v3,
                        texture);
}
////


ClientPlayer me; 

SOCKET server = INVALID_SOCKET;

int print_contents(unsigned char *pk, int pk_len) {
    printf("Dumped packet contents: ");
    for (int i = 0; i < pk_len; i++) {
        if (i < 500){
            printf("%02X ", pk[i]);
        } else {
            printf("... ");
            break;
        }
    }
    printf("\n");
    return 1;
}

int print_contents_full(unsigned char *pk, int pk_len) {
    printf("Dumped packet contents: ");
    for (int i = 0; i < pk_len; i++) {
        if (i < 50000000){
            printf("%02X ", pk[i]);
        } else {
            printf("... ");
            break;
        }
    }
    printf("\n");
    return 1;
}

int32_t sign_extend(int32_t val, int bits) {
    int32_t m = 1 << (bits - 1);
    return (val ^ m) - m;
}



void read_position(const unsigned char *packet, int *pos, int *x, int *y, int *z) { // lol stolen code, pos implemented
    // 1. Read 64-bit Big Endian Integer
    // Note: We can't use read_i32 twice because of endianness.
    // We need a read_u64_be helper.
    uint64_t val = 
        ((uint64_t)packet[*pos] << 56) |
        ((uint64_t)packet[*pos+1] << 48) |
        ((uint64_t)packet[*pos+2] << 40) |
        ((uint64_t)packet[*pos+3] << 32) |
        ((uint64_t)packet[*pos+4] << 24) |
        ((uint64_t)packet[*pos+5] << 16) |
        ((uint64_t)packet[*pos+6] << 8) |
        ((uint64_t)packet[*pos+7]);
    *pos += 8;

    // 2. Unpack
    // X: Top 26 bits
    // Y: Middle 12 bits
    // Z: Bottom 26 bits
    
    // Naive unpacking (Might have sign issues depending on compiler)
    // val >> 38 is X
    // (val >> 26) & 0xFFF is Y
    // val & 0x3FFFFFF is Z
    
    // Robust Unpacking with Sign Extension
    int32_t raw_x = (int32_t)(val >> 38);
    int32_t raw_y = (int32_t)((val >> 26) & 0xFFF);
    int32_t raw_z = (int32_t)(val & 0x3FFFFFF);

    // Apply proper sign extension
    // Since we are casting to int32, we check the highest bit of the N-bit segment
    if (raw_x >= (1 << 25)) raw_x -= (1 << 26);
    if (raw_y >= (1 << 11)) raw_y -= (1 << 12);
    if (raw_z >= (1 << 25)) raw_z -= (1 << 26);

    *x = raw_x;
    *y = raw_y;
    *z = raw_z;
}

void write_position(unsigned char *buffer, int *pos, int x, int y, int z) {
    if (x < -(1 << 25) || x >= (1 << 25) ||
        y < -(1 << 11) || y >= (1 << 11) ||
        z < -(1 << 25) || z >= (1 << 25)) {
        fprintf(stderr, "Error: Position values out of range\n");
        return;
    }

    uint64_t val = ((uint64_t)(x & 0x3FFFFFF) << 38) |
                   ((uint64_t)(y & 0xFFF) << 26) |
                   ((uint64_t)(z & 0x3FFFFFF));
    buffer[*pos] = (val >> 56) & 0xFF;
    buffer[*pos + 1] = (val >> 48) & 0xFF;
    buffer[*pos + 2] = (val >> 40) & 0xFF;
    buffer[*pos + 3] = (val >> 32) & 0xFF;
    buffer[*pos + 4] = (val >> 24) & 0xFF;
    buffer[*pos + 5] = (val >> 16) & 0xFF;
    buffer[*pos + 6] = (val >> 8) & 0xFF;
    buffer[*pos + 7] = val & 0xFF;

    *pos += 8;
}

int read_player_position_packet(const unsigned char *packet, int packet_len, int *pos, double *x, double *y, double *z, float *yaw, float *pitch, int8_t *x_relative, int8_t *y_relative, int8_t *z_relative, int8_t *yaw_relative, int8_t *pitch_relative) {
    (void)packet_len; //make compiler happy
    *x = read_f64(packet, pos);
    *y = read_f64(packet, pos);
    *z = read_f64(packet, pos);
    *yaw = read_f32(packet, pos);
    *pitch = read_f32(packet, pos);
    uint8_t flags = read_i8(packet, pos);
    
    *x_relative = (flags & 0x01) != 0;
    *y_relative = (flags & 0x02) != 0;
    *z_relative = (flags & 0x04) != 0;
    *yaw_relative = (flags & 0x08) != 0;
    *pitch_relative = (flags & 0x10) != 0;

    return 1;
}

int read_selected_slot_packet(const unsigned char *packet, int packet_len, int *pos, int8_t *selected_slot) {
    (void)packet_len; //make compiler happy
    *selected_slot = read_i8(packet, pos);
    return 1;
}

int read_player_spawn_packet(const unsigned char *packet, int packet_len, int *pos, int32_t *entityId, mc_uuid_t *uuid, double *x, double *y, double *z, float *yaw, float *pitch, int16_t *itemInHand) {
    (void)packet_len; //make compiler happy
    uint64_t tmp_entity_id;
    *pos += varint_read_u64(packet + *pos, packet_len - *pos, &tmp_entity_id);
    *entityId = (int32_t)tmp_entity_id;
    *uuid = read_uuid(packet, pos);

    // minecraft has ts thing called network pos or something and it is pos * 32
    int32_t net_x = read_i32(packet, pos);
    int32_t net_y = read_i32(packet, pos);
    int32_t net_z = read_i32(packet, pos);
    double real_x = (double)net_x / 32.0;
    double real_y = (double)net_y / 32.0;
    double real_z = (double)net_z / 32.0;
    *x = real_x;
    *y = real_y;
    *z = real_z;
    //network thingy again
    int8_t net_yaw = read_i8(packet, pos);
    int8_t net_pitch = read_i8(packet, pos);
    *yaw = ((float)net_yaw / 256.0f) * 360.0f;
    *pitch = ((float)net_pitch / 256.0f) * 360.0f;
    *itemInHand = read_i16(packet, pos);
    // ignore metadata, we skip anyways in while loop
    return 1;
}

int read_livingentity_spawn_packet(const unsigned char *packet, int packet_len, int *pos, int32_t *entityId, uint8_t *entityType, double *x, double *y, double *z, float *yaw, float *pitch, float *headPitch, double *vx, double *vy, double *vz) {
    (void)packet_len; //make compiler happy
    uint64_t tmp_entity_id;
    *pos += varint_read_u64(packet + *pos, packet_len - *pos, &tmp_entity_id);
    *entityId = (int32_t)tmp_entity_id;
    *entityType = read_u8(packet, pos);
    // minecraft has ts thing called network pos or something and it is pos * 32
    int32_t net_x = read_i32(packet, pos);
    int32_t net_y = read_i32(packet, pos);
    int32_t net_z = read_i32(packet, pos);
    double real_x = (double)net_x / 32.0;
    double real_y = (double)net_y / 32.0;
    double real_z = (double)net_z / 32.0;
    *x = real_x;
    *y = real_y;
    *z = real_z;
    //network thingy again
    int8_t net_yaw = read_i8(packet, pos);
    int8_t net_pitch = read_i8(packet, pos);
    int8_t net_head_pitch = read_i8(packet, pos);
    *yaw = ((float)net_yaw / 256.0f) * 360.0f;
    *pitch = ((float)net_pitch / 256.0f) * 360.0f;
    *headPitch = ((float)net_head_pitch / 256.0f) * 360.0f;
    int16_t vx_raw = read_i16(packet, pos);
    int16_t vy_raw = read_i16(packet, pos);
    int16_t vz_raw = read_i16(packet, pos);
    *vx = (double)vx_raw / 8000.0;
    *vy = (double)vy_raw / 8000.0;
    *vz = (double)vz_raw / 8000.0;
    // ignore metadata, we skip anyways in while loop
    return 1;
}


int read_nonlivingentity_spawn_packet(const unsigned char *packet, int packet_len, int *pos, int32_t *entityId, uint8_t *entityType, double *x, double *y, double *z, float *yaw, float *pitch) {
    (void)packet_len; //make compiler happy
    uint64_t tmp_entity_id;
    *pos += varint_read_u64(packet + *pos, packet_len - *pos, &tmp_entity_id);
    *entityId = (int32_t)tmp_entity_id;
    *entityType = read_u8(packet, pos);
    // minecraft has ts thing called network pos or something and it is pos * 32
    int32_t net_x = read_i32(packet, pos);
    int32_t net_y = read_i32(packet, pos);
    int32_t net_z = read_i32(packet, pos);
    double real_x = (double)net_x / 32.0;
    double real_y = (double)net_y / 32.0;
    double real_z = (double)net_z / 32.0;
    *x = real_x;
    *y = real_y;
    *z = real_z;
    //network thingy again
    int8_t net_yaw = read_i8(packet, pos);
    int8_t net_pitch = read_i8(packet, pos);
    *yaw = ((float)net_yaw / 256.0f) * 360.0f;
    *pitch = ((float)net_pitch / 256.0f) * 360.0f;
    // ignore metadata, we skip anyways in while loop
    return 1;
}

int read_rel_entity_move_packet(const unsigned char *packet, int packet_len, int *pos, int32_t *entityId, double *dx, double *dy, double *dz, uint8_t *onGround) {
    (void)packet_len; //make compiler happy
    uint64_t tmp_entity_id;
    *pos += varint_read_u64(packet + *pos, packet_len - *pos, &tmp_entity_id);
    *entityId = (int32_t)tmp_entity_id;
    int8_t net_dx = read_i8(packet, pos);
    int8_t net_dy = read_i8(packet, pos);
    int8_t net_dz = read_i8(packet, pos);
    double real_dx = (double)net_dx / 32.0;
    double real_dy = (double)net_dy / 32.0;
    double real_dz = (double)net_dz / 32.0;
    *dx = real_dx;
    *dy = real_dy;
    *dz = real_dz;
    *onGround = read_u8(packet, pos);
    return 1;
}

int read_entity_look_packet(const unsigned char *packet, int packet_len, int *pos, int32_t *entityId, float *yaw, float *pitch, uint8_t *onGround) {
    (void)packet_len; //make compiler happy
    uint64_t tmp_entity_id;
    *pos += varint_read_u64(packet + *pos, packet_len - *pos, &tmp_entity_id);
    *entityId = (int32_t)tmp_entity_id;
    int8_t net_yaw = read_i8(packet, pos);
    int8_t net_pitch = read_i8(packet, pos);
    *yaw = ((float)net_yaw / 256.0f) * 360.0f;
    *pitch = ((float)net_pitch / 256.0f) * 360.0f;
    *onGround = read_u8(packet, pos);
    return 1;
}

int read_entity_move_look_packet(const unsigned char *packet, int packet_len, int *pos, int32_t *entityId, double *dx, double *dy, double *dz, float *yaw, float *pitch, uint8_t *onGround) {
    (void)packet_len; //make compiler happy
    uint64_t tmp_entity_id;
    *pos += varint_read_u64(packet + *pos, packet_len - *pos, &tmp_entity_id);
    *entityId = (int32_t)tmp_entity_id;
    int8_t net_dx = read_i8(packet, pos);
    int8_t net_dy = read_i8(packet, pos);
    int8_t net_dz = read_i8(packet, pos);
    double real_dx = (double)net_dx / 32.0;
    double real_dy = (double)net_dy / 32.0;
    double real_dz = (double)net_dz / 32.0;
    *dx = real_dx;
    *dy = real_dy;
    *dz = real_dz;
    int8_t net_yaw = read_i8(packet, pos);
    int8_t net_pitch = read_i8(packet, pos);
    *yaw = ((float)net_yaw / 256.0f) * 360.0f;
    *pitch = ((float)net_pitch / 256.0f) * 360.0f;
    *onGround = read_u8(packet, pos);
    return 1;
}

int read_entity_teleport_packet(const unsigned char *packet, int packet_len, int *pos, int32_t *entityId, double *x, double *y, double *z, float *yaw, float *pitch, uint8_t *onGround) {
    (void)packet_len; //make compiler happy
    uint64_t tmp_entity_id;
    *pos += varint_read_u64(packet + *pos, packet_len - *pos, &tmp_entity_id);
    *entityId = (int32_t)tmp_entity_id;
    // minecraft has ts thing called network pos or something and it is pos * 32
    int32_t net_x = read_i32(packet, pos);
    int32_t net_y = read_i32(packet, pos);
    int32_t net_z = read_i32(packet, pos);
    double real_x = (double)net_x / 32.0;
    double real_y = (double)net_y / 32.0;
    double real_z = (double)net_z / 32.0;
    *x = real_x;
    *y = real_y;
    *z = real_z;
    //network thingy again
    int8_t net_yaw = read_i8(packet, pos);
    int8_t net_pitch = read_i8(packet, pos);
    *yaw = ((float)net_yaw / 256.0f) * 360.0f;
    *pitch = ((float)net_pitch / 256.0f) * 360.0f;
    *onGround = read_u8(packet, pos);
    return 1;
}

void read_and_handle_destroy_entities(const unsigned char *packet, int *pos) {
    uint64_t count;
    int len = varint_read_u64(packet + *pos, 10, &count);
    *pos += len;
    //printf("Destroying %llu entities\n", count);
    for (int i = 0; i < (int)count; i++) {
        uint64_t entity_id;
        len = varint_read_u64(packet + *pos, 10, &entity_id);
        *pos += len;
        destroy_entity((int32_t)entity_id);
        // Debug
    }
}

int read_abilities_packet(const unsigned char *packet, int packet_len, int *pos, int8_t *invulnerable, int8_t *flying, int8_t *allow_flying, int8_t *creative_mode, float *flySpeed, float *walkSpeed) {
    (void)packet_len; //make compiler happy
    uint8_t flags = read_i8(packet, pos);
    *flying = (flags & 0x02) != 0;
    *allow_flying = (flags & 0x04) != 0;
    *invulnerable = (flags & 0x01) != 0;
    *creative_mode = (flags & 0x08) != 0;
    // speeds
    *flySpeed = read_f32(packet, pos);
    *walkSpeed = read_f32(packet, pos);
    return 1;
}

int read_spawnloc_packet(const unsigned char *packet, int packet_len, int *pos, int *x, int *y, int *z) {
    (void)packet_len; //make compiler happy
    read_position(packet, pos, x, y, z); // pos is incremented inside
    return 1;
}

int read_loginconfirm_packet(const unsigned char *packet, int packet_len, int *pos, int32_t *entityId, uint8_t *gameMode, int8_t *dimension, uint8_t *difficulty, uint8_t *maxPlayers) {
    *entityId = read_i32(packet, pos);
    *gameMode = read_u8(packet, pos);
    *dimension = (int8_t)read_u8(packet, pos);
    *difficulty = read_u8(packet, pos);
    *maxPlayers = read_u8(packet, pos);
    char levelType[MAX_LEVEL_TYPE_LEN];
    int maxLevelTypeLen = sizeof(levelType);
    read_string(packet, packet_len, pos, levelType, maxLevelTypeLen);
    uint8_t reducedDebugInfo = read_u8(packet, pos); 
    (void)reducedDebugInfo; //make compiler happy
    return 1;
}

int read_compression_packet(const unsigned char *packet, int packet_len, int *pos, int *thres) {
    uint64_t threshold;

    int len = varint_read_u64(packet + *pos, packet_len - *pos, &threshold);
    *pos += len;

    printf("Set Compression Threshold: %llu\n", threshold);
    *thres = (int)threshold;
    return 1;
}

bool should_render_face(Block* myself, Block* neighbor){
    if (neighbor == NULL) return false; // No neighbor means we should not render the face -- finally gfot the chance to do this right
    if (block_defs[neighbor->id][neighbor->meta].model == MODEL_LEAVES && block_defs[myself->id][myself->meta].model == MODEL_LEAVES) return false;
    if (block_defs[neighbor->id][neighbor->meta].model == MODEL_LEAVES && block_defs[myself->id][myself->meta].model != MODEL_LEAVES && block_defs[neighbor->id][neighbor->meta].is_full_opaque == false) return true;
    return block_defs[neighbor->id][neighbor->meta].is_full_opaque == false;
}

uint8_t block_render_flags(Block* myself, Block* neighbor_n, Block* neighbor_s, Block* neighbor_e, Block* neighbor_w, Block* neighbor_up, Block* neighbor_down) {
    uint8_t flags = 0;
    if (should_render_face(myself, neighbor_n)) {
        flags |= RENDER_FACE_NORTH; 
    }// North
    if (should_render_face(myself, neighbor_s)) {
        flags |= RENDER_FACE_SOUTH;
    } // South
    if (should_render_face(myself, neighbor_e)) {
        flags |= RENDER_FACE_EAST;
    } // East
    if (should_render_face(myself, neighbor_w)) {
        flags |= RENDER_FACE_WEST; // West
    }
    if (should_render_face(myself, neighbor_up)) {
        flags |= RENDER_FACE_TOP; // Up
    }
    if (should_render_face(myself, neighbor_down)) {
        flags |= RENDER_FACE_BOTTOM; // Down
    }
    return flags;
}



void build_render_flags(ChunkData *c, ChunkData *neighbor_n, ChunkData *neighbor_s, ChunkData *neighbor_e, ChunkData *neighbor_w) {
    for (int y = 0; y < 256; y++) {
        for (int z = 0; z < 16; z++) {
            for (int x = 0; x < 16; x++) {
                Block *b = &c->data.blocks[y][z][x];
                if (b->id == 0) {
                    b->render_flags = RENDER_SKIP_ALL;
                    continue;
                }
                
                Block *block_n = NULL;
                if (z == 0) {
                    if (neighbor_n) block_n = &neighbor_n->data.blocks[y][15][x];
                } else {
                    block_n = &c->data.blocks[y][z-1][x];
                }
                
                Block *block_s = NULL;
                if (z == 15) {
                    if (neighbor_s) block_s = &neighbor_s->data.blocks[y][0][x];
                } else {
                    block_s = &c->data.blocks[y][z+1][x];
                }

                Block *block_e = NULL;
                if (x == 15) {
                    if (neighbor_e) block_e = &neighbor_e->data.blocks[y][z][0];
                } else {
                    block_e = &c->data.blocks[y][z][x+1];
                }

                Block *block_w = NULL;
                if (x == 0) {
                    if (neighbor_w) block_w = &neighbor_w->data.blocks[y][z][15];
                } else {
                    block_w = &c->data.blocks[y][z][x-1];
                }

                Block *block_top = NULL;
                if (y == 255) {
                    block_top = NULL;
                } else {
                    block_top = &c->data.blocks[y+1][z][x];
                }
                Block *block_bottom = NULL;
                if (y == 0) {
                    block_bottom = NULL;
                } else {                   
                    block_bottom = &c->data.blocks[y-1][z][x];
                }

                uint8_t flags = 0;
                flags = block_render_flags(b, block_n, block_s, block_e, block_w, block_top, block_bottom);


                /*
                // Top (y+1)
                if (y >= 255) flags |= RENDER_FACE_TOP;
                else if (!block_defs[c->data.blocks[y+1][z][x].id][c->data.blocks[y+1][z][x].meta].is_full_opaque) flags |= RENDER_FACE_TOP;

                // Bottom (y-1)
                if (y <= 0) flags |= RENDER_FACE_BOTTOM;
                else if (!block_defs[c->data.blocks[y-1][z][x].id][c->data.blocks[y-1][z][x].meta].is_full_opaque) flags |= RENDER_FACE_BOTTOM;

                // South (z+1)
                if (z >= 15) {
                    if (!neighbor_s || !block_defs[neighbor_s->data.blocks[y][0][x].id][neighbor_s->data.blocks[y][0][x].meta].is_full_opaque) flags |= RENDER_FACE_SOUTH;
                } else if (!block_defs[c->data.blocks[y][z+1][x].id][c->data.blocks[y][z+1][x].meta].is_full_opaque) flags |= RENDER_FACE_SOUTH;

                // North (z-1)
                if (z <= 0) {
                    if (!neighbor_n || !block_defs[neighbor_n->data.blocks[y][15][x].id][neighbor_n->data.blocks[y][15][x].meta].is_full_opaque) flags |= RENDER_FACE_NORTH;
                } else if (!block_defs[c->data.blocks[y][z-1][x].id][c->data.blocks[y][z-1][x].meta].is_full_opaque) flags |= RENDER_FACE_NORTH;

                // East (x+1)
                if (x >= 15) {
                    if (!neighbor_e || !block_defs[neighbor_e->data.blocks[y][z][0].id][neighbor_e->data.blocks[y][z][0].meta].is_full_opaque) flags |= RENDER_FACE_EAST;
                } else if (!block_defs[c->data.blocks[y][z][x+1].id][c->data.blocks[y][z][x+1].meta].is_full_opaque) flags |= RENDER_FACE_EAST;

                // West (x-1)
                if (x <= 0) {
                    if (!neighbor_w || !block_defs[neighbor_w->data.blocks[y][z][15].id][neighbor_w->data.blocks[y][z][15].meta].is_full_opaque) flags |= RENDER_FACE_WEST;
                } else if (!block_defs[c->data.blocks[y][z][x-1].id][c->data.blocks[y][z][x-1].meta].is_full_opaque) flags |= RENDER_FACE_WEST;*/
                
                c->data.blocks[y][z][x].render_flags = flags;
            }
        }
    }
}

void block_mesh_queue_add_block(int x, int y, int z) {
    for (int i = 0; i < BLOCK_MESH_QUEUE_SIZE; i++) {
        if (block_mesh_queue[i].active) {
            continue; // already in queue
        }
        block_mesh_queue[i].x = x;
        block_mesh_queue[i].y = y;
        block_mesh_queue[i].z = z;
        block_mesh_queue[i].active = 1;
        return;
    }
}

int read_and_apply_multi_block_change_packet_info(const unsigned char *packet, int packet_len, int *pos) {
    int32_t chunk_x = read_i32(packet, pos);
    int32_t chunk_z = read_i32(packet, pos);
    uint64_t record_count = 0;
    int len = varint_read_u64(packet + *pos, packet_len - *pos, &record_count);
    *pos += len;
    //printf("Multi Block Change: Chunk (%d,%d), Records=%llu\n", chunk_x, chunk_z, record_count);
    ChunkData *chunk = get_chunk(chunk_x, chunk_z);
    for (int i = 0; i < (int)record_count; i++) {
        uint16_t packed = (uint16_t)read_i16(packet, pos);
        int localX = (packed >> 12) & 0xF;
        int localZ = (packed >> 8) & 0xF;
        int y = packed & 0xFF;
        uint64_t bid_and_meta;
        int len = varint_read_u64(packet + *pos, packet_len - *pos, &bid_and_meta);
        *pos += len;
        int id = bid_and_meta >> 4;
        int meta = bid_and_meta & 0xF;
        chunk->data.blocks[y][localZ][localX] = (Block){id, meta, 0};
        block_mesh_queue_add_block(chunk_x * 16 + localX, y, chunk_z * 16 + localZ);
    }
    return 1;
}

int read_and_apply_single_block_change(const unsigned char *packet, int packet_len, int *pos) {
    int x,y,z;
    read_position(packet, pos, &x, &y, &z); 
    uint64_t bid_and_meta;
    int len = varint_read_u64(packet + *pos, packet_len - *pos, &bid_and_meta);
    *pos += len;
    int id = bid_and_meta >> 4;
    int meta = bid_and_meta & 0xF;
    set_block_xyz(x, y, z, id, meta);
    block_mesh_queue_add_block(x, y, z);
    return 1;
}

int read_and_apply_bulk_chunk_data_packet_info(const unsigned char *packet, int packet_len, int *pos) {
    int32_t completed_chunk_poses_x[WORLD_CHUNKS]; 
    int32_t completed_chunk_poses_z[WORLD_CHUNKS]; 

    uint8_t sky_light_sent = read_u8(packet, pos);
    uint64_t chunk_count = 0;
    
    int len = varint_read_u64(packet + *pos, packet_len - *pos, &chunk_count);
    *pos += len;

    ChunkMeta chunk_metas[WORLD_CHUNKS]; 

    printf("Map Chunk Bulk: %llu chunks, SkyLight=%d, Len=%d\n", chunk_count, sky_light_sent, packet_len - *pos);
    struct timeval stop, start;
    gettimeofday(&start, NULL);
    for (int i = 0; i < (int)chunk_count; i++) {
        int32_t x = read_i32(packet, pos);
        int32_t z = read_i32(packet, pos);
        uint16_t bitmap = (read_u8(packet, pos) << 8) | read_u8(packet, pos);
        //printf("  Chunk %d: X=%d, Z=%d, Sections=0x%04X\n", i, x, z, bitmap);
        ChunkData *chunk = allocate_chunk(x, z);
        if (!chunk) { printf("OOM!"); return 0; } 
        chunk->active = 1;
        chunk->initialized = 0;
        chunk->meta.x = x;
        chunk->meta.z = z;
        chunk->meta.bitmap = bitmap;
        chunk_metas[i] = chunk->meta;
    }
    
    for (int i = 0; i < (int)chunk_count; i++) {
        ChunkData *chunk = get_chunk(chunk_metas[i].x, chunk_metas[i].z);
        uint16_t mask = chunk_metas[i].bitmap;
        //printf("Parsing Chunk %d: X=%d, Z=%d, Sections=0x%04X\n", i, chunk_metas[i].x, chunk_metas[i].z, chunk_metas[i].bitmap);
        if (chunk == NULL) {
            printf("ERROR: No memory for chunk %d,%d\n", chunk_metas[i].x, chunk_metas[i].z);
            continue;
        }
        if (chunk_metas[i].bitmap == 0) { // unload chunk?
            clear_chunk_data(chunk);
            chunk->active = 0;
            chunk->initialized = 0;
            chunk->meshed = 0;
            //printf("Unloaded chunk at %d,%d\n", x, z);
            return 0;
        }
        chunk->initialized = 1;
        chunk->meshed = 0; 
        for (int s = 0; s < 16; s++) {
            if (mask & (1 << s)) {
                //clear_chunk_sector_data(chunk, s);
                // parse sec of chunks - y is divided every 16 blocks into 16x16x16 cubes
                const unsigned char *section_start = packet + *pos;
                for (int y = 0; y < 16; y++) {
                    for (int z = 0; z < 16; z++) {
                        for (int x = 0; x < 16; x++) {
                            int index = (y * 256) + (z * 16) + x;
                            unsigned char b0 = section_start[index * 2];
                            unsigned char b1 = section_start[index * 2 + 1];
                            uint16_t raw = b0 | (b1 << 8);
                            uint16_t block_id = raw >> 4;
                            uint8_t meta = raw & 0xF;
                            if (1) {
                                //int abs_x = (chunk_metas[i].x * 16) + x;
                                int abs_y = (s * 16) + y;
                                //int abs_z = (chunk_metas[i].z * 16) + z;
                                //(void)abs_x; (void)abs_z;
                                
                                // it would be faster if we built the chunk here
                                chunk->data.blocks[abs_y][z][x] = (Block){block_id, meta, 0}; // render flags will be built later
                            }
                        }
                    }
                }
                *pos += 8192; 
            }
        }
        
        // 2. Block Light
        for (int s = 0; s < 16; s++) {
            if (mask & (1 << s)) {
                *pos += 2048; // Skip light
            }
        }
        
        // 3. Sky Light (If Sent)
        if (sky_light_sent) {
            for (int s = 0; s < 16; s++) {
                if (mask & (1 << s)) {
                    *pos += 2048; // Skip skylight
                }
            }
        }
        
        // 4. Biomes (One 256 byte array per CHUNK, not per section)
        *pos += 256;
        
        
        //rintf("Parsed Chunk %d: X=%d, Z=%d, Sections=0x%04X\n", i, chunk_metas[i].x, chunk_metas[i].z, chunk_metas[i].bitmap);
    }
    // create chunk meshes - done in chunk mesh queue now
    /*for (int i = 0; i < (int)chunk_count; i++) {
        ChunkData *chunk = get_chunk(chunk_metas[i].x, chunk_metas[i].z);
        if (chunk && chunk->initialized) {
            ChunkData *neighbors[4];
            neighbors[0] = get_chunk(chunk->meta.x, chunk->meta.z - 1); // north
            neighbors[1] = get_chunk(chunk->meta.x + 1, chunk->meta.z); // east
            neighbors[2] = get_chunk(chunk->meta.x, chunk->meta.z + 1); // south
            neighbors[3] = get_chunk(chunk->meta.x - 1, chunk->meta.z); // west
            build_render_flags(chunk, neighbors[0],neighbors[2],neighbors[1],neighbors[3]);
        }
    }*/
    //refactor chunk meshes for surrounding chunks
    int8_t neighboring_directions[4][2] = {
        {0, -1}, // north
        {1, 0},  // east
        {0, 1},  // south
        {-1, 0}  // west
    };
    for (int i = 0; i < (int)chunk_count; i++) {
        //check each neighboring direction
        for (int d = 0; d < 4; d++) {
            int32_t neighbor_x = chunk_metas[i].x + neighboring_directions[d][0];
            int32_t neighbor_z = chunk_metas[i].z + neighboring_directions[d][1];
            //check if neighbor is in completed chunks
            int neighbor_index = -1;
            for (int j = 0; j < i; j++) {
                if (completed_chunk_poses_x[j] == neighbor_x && completed_chunk_poses_z[j] == neighbor_z) {
                    neighbor_index = j;
                    break;
                }
            }
            //check if neighbor chunk exists
            ChunkData *neighbor_chunk = get_chunk(neighbor_x, neighbor_z);
            if (neighbor_index != -1 && neighbor_chunk && neighbor_chunk->initialized) {
                //refactor mesh for neighbor chunk
                neighbor_chunk->meshed = 0; 
            }
        }
        completed_chunk_poses_x[i] = chunk_metas[i].x;
        completed_chunk_poses_z[i] = chunk_metas[i].z;
    }

    //printf("  Chunk data size: %d bytes\n", packet_len - *pos);
    gettimeofday(&stop, NULL);
    printf("Took %lu us to parse bulk chunk data packet\n", (stop.tv_sec - start.tv_sec) * 1000000 + stop.tv_usec - start.tv_usec);
    return 1;
}

int read_and_apply_map_chunk_packet_info(const unsigned char *packet, int packet_len, int *pos) {
    (void)packet_len; //make compiler happy
    int32_t x = read_i32(packet, pos);
    int32_t z = read_i32(packet, pos);
    uint8_t ground_up_continuous = read_u8(packet, pos);
    uint16_t primary_bitmap = (read_u8(packet, pos) << 8) | read_u8(packet, pos);

    ChunkData *chunk = get_chunk(x, z);
    if (!chunk) {
        chunk = allocate_chunk(x, z);
        if (!chunk) { 
            printf("OOM!"); 
            return 0; 
        }
    } else if (ground_up_continuous) {
        clear_chunk_data(chunk); // only clear old data if its a full chunk update
    }

    if (primary_bitmap == 0) { // Unload chunk
        chunk->active = 0;
        chunk->initialized = 0;
        chunk->meshed = 0;
        return 1;
    }

    chunk->active = 1;
    chunk->initialized = 0;
    chunk->meta.x = x;
    chunk->meta.z = z;
    chunk->meta.bitmap |= primary_bitmap;

    //print_contents_full(packet, packet_len);

    uint64_t data_size;
    *pos += varint_read_u64(packet + *pos, packet_len - *pos, &data_size);
    (void)data_size; //make compiler happy
    
    for (int s = 0; s < 16; s++) {
        if (primary_bitmap & (1 << s)) {
            const unsigned char *section_start = packet + *pos;
            //printf("Parsing section %d for chunk %d,%d\n", s, x, z);
            //printf("Section %d start pos: %d\n", s, *pos);
            for (int y = 0; y < 16; y++) {
                for (int z = 0; z < 16; z++) {
                    for (int x = 0; x < 16; x++) {
                        int index = (y * 256) + (z * 16) + x;
                        unsigned char b0 = section_start[index * 2];
                        unsigned char b1 = section_start[index * 2 + 1];
                        uint16_t raw = b0 | (b1 << 8);
                        uint16_t block_id = raw >> 4;
                        uint8_t meta = raw & 0xF;
                        
                        int abs_y = (s * 16) + y;
                        chunk->data.blocks[abs_y][z][x] = (Block){block_id, meta, 0};
                    }
                }
            }
            *pos += 8192; 
        }
    }
    
    for (int s = 0; s < 16; s++) {
        if (primary_bitmap & (1 << s)) {
            *pos += 2048; 
        }
    }
    
    if (ground_up_continuous) {
        for (int s = 0; s < 16; s++) {
            if (primary_bitmap & (1 << s)) {
                *pos += 2048; 
            }
        }
    }
    
    if (ground_up_continuous) {
        *pos += 256;
    }
    
    chunk->initialized = 1;
    chunk->meshed = 0;

    ChunkData *neighbors[4];
    neighbors[0] = get_chunk(chunk->meta.x, chunk->meta.z - 1); // north
    neighbors[1] = get_chunk(chunk->meta.x + 1, chunk->meta.z); // east
    neighbors[2] = get_chunk(chunk->meta.x, chunk->meta.z + 1); // south
    neighbors[3] = get_chunk(chunk->meta.x - 1, chunk->meta.z); // west
    
    for (int d = 0; d < 4; d++) {
        if (neighbors[d] && neighbors[d]->initialized) {
            neighbors[d]->meshed = 0; 
        }
    }
    
    return 1;
}

int read_playstate_packet(const unsigned char *packet, int packet_len, int *pos, char *uuid, uint64_t max_uuid_len) {
    uint64_t uuid_len;
    int len = varint_read_u64(packet + *pos, packet_len - *pos, &uuid_len);
    *pos += len;

    if (uuid_len < max_uuid_len) {
        memcpy(uuid, packet + *pos, uuid_len);
        uuid[uuid_len] = '\0'; 
        printf("My UUID: %s\n", uuid);
    }
    *pos += uuid_len;
    return 1;
}

int build_handshake_packet(unsigned char *final, const char *host, u_short port) { //final len should be 512 define like unsigned char final[512];
    unsigned char buf[512];
    int pos = 0;
    pos += varint_write_u64(buf + pos, 0x00); // 0x00 is ID of packet
    pos += varint_write_u64(buf + pos, 47); // Protocol version, 47 corresponds to 1.8

    int host_len = strlen(host);

    pos += varint_write_u64(buf + pos, host_len);
    memcpy(buf + pos, host, host_len);
    pos += host_len;

    uint16_t port_ = htons(port);
    memcpy(buf + pos, &port_, 2);
    pos += 2;

    pos += varint_write_u64(buf + pos, 2); // Next state 2 = login
    int final_pos = 0;

    final_pos += varint_write_u64(final + final_pos, pos); // why tf i gotta write final length
    memcpy(final + final_pos, buf, pos);
    final_pos += pos;
    return final_pos;
}

int build_login_packet(unsigned char *final, const char *name, int name_len) { //final len should be 512 define like unsigned char final[512];
    //name as string, type as string
    //const char *name = "test";
    //int name_len = strlen(name);
    unsigned char buf[512];
    int pos = 0;
    pos += varint_write_u64(buf + pos, 0x00); // 0x00 is ID of packet for login start

    //Name
    pos += varint_write_u64(buf + pos, name_len);
    memcpy(buf + pos, name, name_len);
    pos += name_len; 

    int final_pos = 0;

    final_pos += varint_write_u64(final + final_pos, pos); 
    memcpy(final + final_pos, buf, pos);
    final_pos += pos;
    return final_pos;
}

int build_player_pos_and_look_packet(unsigned char *final, double x, double y, double z, float yaw, float pitch, uint8_t on_ground) { //final len should be 512 define like unsigned char final[512];
    unsigned char buf[512];
    int pos = 0;
    if (compress_threshold >= 0) {
        pos += varint_write_u64(buf + pos, 0x00); // 0x00 is the length - no compression required here so that is why it is 0
    }
    pos += varint_write_u64(buf + pos, 0x06); // 0x06 is ID of packet for login start
    pos += write_f64(buf + pos, x);
    pos += write_f64(buf + pos, y);
    pos += write_f64(buf + pos, z);
    pos += write_f32(buf + pos, yaw);
    pos += write_f32(buf + pos, pitch);
    pos += write_u8(buf + pos, on_ground); // on ground true

    // write length cuz server stupid
    int final_pos = 0;
    final_pos += varint_write_u64(final + final_pos, pos); 
    memcpy(final + final_pos, buf, pos);
    final_pos += pos;
    return final_pos;
}

int build_settings_packet(unsigned char *final, int8_t view_distance) { //final len should be 512 define like unsigned char final[512];
    unsigned char buf[512];
    int pos = 0;
    if (compress_threshold >= 0) {
        pos += varint_write_u64(buf + pos, 0x00); // 0x00 is the length - no compression required here so that is why it is 0
    }
    pos += varint_write_u64(buf + pos, 0x15); // 0x15 is ID of packet for settings
    pos += write_string(buf + pos, "en_US"); // locale
    pos += write_i8(buf + pos, view_distance); // view distance
    pos += write_i8(buf + pos, 0); // chat flags - 0 means enabled
    pos += write_u8(buf + pos, 1); // chat colors - 1 means enabled
    pos += write_u8(buf + pos, 0x7F); // skin parts (all enabled)
    //pos += write_f64(buf + pos, x);

    // write length cuz server stupid
    int final_pos = 0;
    final_pos += varint_write_u64(final + final_pos, pos); 
    memcpy(final + final_pos, buf, pos);
    final_pos += pos;
    return final_pos;
}

int build_spawn_packet(unsigned char *final) { //final len should be 512 define like unsigned char final[512];
    unsigned char buf[512];
    int pos = 0;
    if (compress_threshold >= 0) {
        pos += varint_write_u64(buf + pos, 0x00); // 0x00 is the length - no compression required here so that is why it is 0
    }
    pos += varint_write_u64(buf + pos, 0x16); // 0x16 is ID of packet for spawn
    pos += varint_write_u64(buf + pos, 0); // 0 = spawn

    // write length cuz server stupid
    int final_pos = 0;
    final_pos += varint_write_u64(final + final_pos, pos); 
    memcpy(final + final_pos, buf, pos);
    final_pos += pos;
    return final_pos;
}

int build_block_place_packet(unsigned char *final, int32_t x, int32_t y, int32_t z, uint8_t direction, int16_t hand, float cursor_x, float cursor_y, float cursor_z) { //final len should be 512 define like unsigned char final[512];
    //save nbt raw bin for this shit
    //slot=id,count,dmg,nbt|i16,i8,i16
    unsigned char buf[512];
    int pos = 0;
    if (compress_threshold >= 0) {
        pos += varint_write_u64(buf + pos, 0x00); // 0x00 is the length - no compression required here so that is why it is 0
    }
    pos += varint_write_u64(buf + pos, 0x08); // 0x08 is ID of packet for block place
    write_position(buf, &pos, x, y, z);
    pos += write_u8(buf + pos, direction);
    pos += write_i16(buf + pos, hand);
    pos += write_f32(buf + pos, cursor_x);
    pos += write_f32(buf + pos, cursor_y);
    pos += write_f32(buf + pos, cursor_z);

    // write length cuz server stupid
    int final_pos = 0;
    final_pos += varint_write_u64(final + final_pos, pos); 
    memcpy(final + final_pos, buf, pos);
    final_pos += pos;
    return final_pos;
}

void skip_nbt(uint8_t type, const unsigned char *packet, int *pos) {
    // bullshit remover
    if (type == 0) return;
    switch (type) {
        case 1: read_u8(packet, pos); break;
        case 2: read_i16(packet, pos); break;
        case 3: read_i32(packet, pos); break;
        case 4: read_i64(packet, pos); break;
        case 5: read_f32(packet, pos); break;
        case 6: read_f64(packet, pos); break;
        case 7: read_i32(packet, pos); break;
        case 8: (uint16_t)read_i16(packet, pos); break;
        case 9: {
            uint8_t subType = read_u8(packet, pos);
            int32_t count = read_i32(packet, pos);
            for (int i = 0; i < count; i++) {
                skip_nbt(subType, packet, pos);
            }
            break;
        }
        case 10: {
            while (true) {
                uint8_t innerType = read_u8(packet, pos);
                if (innerType == 0) break;
                (uint16_t)read_i16(packet, pos);
                skip_nbt(innerType, packet, pos);
            }
            break;
        }
        case 11: (void)(read_i32(packet, pos) * 4); break; // void is to shut the compiler up about unused value, we just want to skip it
    }
}

int read_and_apply_set_slot_packet(const unsigned char *packet, int packet_len, int *pos) {
    (void)packet_len; //make compiler happy
    uint8_t window_id = read_u8(packet, pos);
    int16_t slot = read_i16(packet, pos);
    int16_t id = read_i16(packet, pos);
    if (id == -1) {
        if (window_id == 0) {
            me.inventory.inv[slot].id = 0;
            me.inventory.inv[slot].meta_or_dmg = 0;
            me.inventory.inv[slot].count = 0;
            //printf("Inventory Slot %d: Empty\n", slot);
        }
    } else {
        int8_t item_count = read_i8(packet, pos);
        int16_t damage_or_meta = read_i16(packet, pos);
        int8_t nbt_start_byte = read_i8(packet, pos);
        
        if (nbt_start_byte == 0x00) {
        } else {
            skip_nbt(nbt_start_byte, packet, pos); 
        }
        if (window_id == 0) {
            me.inventory.inv[slot].id = id;
            me.inventory.inv[slot].meta_or_dmg = damage_or_meta;
            me.inventory.inv[slot].count = item_count;
            printf("Inventory Slot %d: ID=%d, Count=%d, Meta/Dmg=%d\n", slot, id, item_count, damage_or_meta);
        }
    }
    return 1;
}

int read_and_apply_window_items_packet(const unsigned char *packet, int packet_len, int *pos) {
    (void)packet_len; //make compiler happy
    uint8_t window_id = read_u8(packet, pos);
    uint16_t count = (uint16_t)read_i16(packet, pos);
    for (int i = 0; i < count; i++) {
        int16_t id = read_i16(packet, pos);
        if (id == -1) {
            if (window_id == 0) {
                me.inventory.inv[i].id = 0;
                me.inventory.inv[i].meta_or_dmg = 0;
                me.inventory.inv[i].count = 0;
                //printf("Inventory Slot %d: Empty\n", i);
            }
        } else {
            int8_t item_count = read_i8(packet, pos);
            int16_t damage_or_meta = read_i16(packet, pos);
            int8_t nbt_start_byte = read_i8(packet, pos);
            
            if (nbt_start_byte == 0x00) {
            } else {
                skip_nbt(nbt_start_byte, packet, pos); 
            }
            if (window_id == 0) {
                // inventnory
                // 0-4 = crafting
                // 5-8 = armor
                // 9-35 = main inventory
                // 36-44 = hotbar
                me.inventory.inv[i].id = id;
                me.inventory.inv[i].meta_or_dmg = damage_or_meta;
                me.inventory.inv[i].count = item_count;
                printf("Inventory Slot %d: ID=%d, Count=%d, Meta/Dmg=%d\n", i, id, item_count, damage_or_meta);
            }
        }
    }
    return 1;
}


int handle_packet(const unsigned char *packet, int packet_len) {
    int pos = 0;

    static unsigned char dec_packet[RECV_BUF_SIZE];
    int dec_len = 0;
    int dec_pos = 0;
    
    if (compress_threshold >= 0) {
        uint64_t data_len;
        int data_len_bytes = varint_read_u64(packet + pos, packet_len - pos, &data_len);
        pos += data_len_bytes;

        if (data_len > 0) {
            // Compressed 
            mz_ulong dest_len = (mz_ulong)data_len;
            int ret = mz_uncompress(dec_packet, &dest_len, packet + pos, packet_len - pos);
            if (ret != MZ_OK) {
                printf("Decompression failed: %d\n", ret);
                return 0;
            }
            dec_len = (int)dest_len;
        } else {
            // Not compressed
            memcpy(dec_packet, packet + pos, packet_len - pos);
            dec_len = packet_len - pos;
        }
    } else {
        // No compression enabled yet
        memcpy(dec_packet, packet, packet_len);
        dec_len = packet_len;
    }

    uint64_t packet_id;
    int id_bytes = varint_read_u64(dec_packet + dec_pos, dec_len - dec_pos, &packet_id);
    if (id_bytes <= 0) return 0;
    dec_pos += id_bytes;

    //printf("Packet ID: 0x%" PRIX64 "\n", packet_id);

    //print_contents((unsigned char*)dec_packet, dec_len);

    if (current_state == STATE_LOGIN) {
        if (packet_id == 0x03) { // Set Compression
            read_compression_packet(dec_packet, dec_len, &dec_pos, &compress_threshold);
            printf("Compression threshold set to %d\n", compress_threshold);
        } else if (packet_id == 0x02) { // Play state
            read_playstate_packet(dec_packet, dec_len, &dec_pos, my_uuid, (uint64_t)sizeof(my_uuid));
            mc_uuid_t uuid = uuid_from_string(my_uuid);
            printf("Parsed UUID: ");
            print_uuid(uuid);
            me.base.base.uuid = uuid;
            current_state = STATE_PLAY;
            printf("Switched to PLAY state\n");
        }
    } else if (current_state == STATE_PLAY) {
        if (packet_id == 0x01) { // Login Confirm
            int32_t entityId;
            uint8_t gameMode;
            int8_t dimension;
            uint8_t difficulty;
            uint8_t maxPlayers;
            read_loginconfirm_packet(dec_packet, dec_len, &dec_pos, &entityId, &gameMode, &dimension, &difficulty, &maxPlayers);
            printf("Login Confirmed: EntityID=%d, GameMode=%d, Dimension=%d, Difficulty=%d, MaxPlayers=%d\n", entityId, gameMode, dimension, difficulty, maxPlayers);
            me.base.base.entity_id = entityId; // damnit why the bases
            me.base.base.dimension = dimension;
            me.base.gamemode = gameMode;
            //send settings packet
            
            int final_len = build_settings_packet(send_pk, packet_view_distance); 
            printf("Built Settings Packet with length %d\n", final_len);
            print_contents(send_pk, final_len);
            send(server, (char*)send_pk, final_len, 0);
            //send spawn packet
            final_len = build_spawn_packet(send_pk);
            printf("Built Spawn Packet with length %d\n", final_len);
            print_contents(send_pk, final_len);
            send(server, (char*)send_pk, final_len, 0);
        } else if (packet_id == 0x41) { // difficulty, can be ignored
        } else if (packet_id == 0x05) {
            int x, y, z;
            read_spawnloc_packet(dec_packet, dec_len, &dec_pos, &x, &y, &z);
            printf("World Spawn Location: X=%d, Y=%d, Z=%d\n", x, y, z); // this isn't player spawn location
        } else if (packet_id == 0x39) { // player abilities packet - speed / flyingspeed
            read_abilities_packet(dec_packet, dec_len, &dec_pos, &me.invulnerable, &me.flying, &me.allow_flying, &me.creative_mode, &me.fly_speed, &me.walk_speed);
            //printf("Player Abilities: Invulnerable=%d, Flying=%d, AllowFlying=%d, CreativeMode=%d, FlySpeed=%.2f, WalkSpeed=%.2f\n", me.invulnerable, me.flying, me.allow_flying, me.creative_mode, me.fly_speed, me.walk_speed);
        } else if (packet_id == 0x09) { // selected slot
            read_selected_slot_packet(dec_packet, dec_len, &dec_pos, &me.selected_slot);
        } else if (packet_id == 0x38) {
            // tab list packet, ignore for now
        } else if (packet_id == 0x08) { // position packet
            //printf("Received Player Position Packet\n");
            double tmp_x, tmp_y, tmp_z;
            float tmp_yaw, tmp_pitch;
            int8_t x_rel, y_rel, z_rel, yaw_rel, pitch_rel;
            read_player_position_packet(dec_packet, dec_len, &dec_pos, &tmp_x, &tmp_y, &tmp_z, &tmp_yaw, &tmp_pitch, &x_rel, &y_rel, &z_rel, &yaw_rel, &pitch_rel);
            if (1){ // temporary terst
                if (x_rel) me.base.base.x += tmp_x; else me.base.base.x = tmp_x;
                if (y_rel) me.base.base.y += tmp_y; else me.base.base.y = tmp_y;
                if (z_rel) me.base.base.z += tmp_z; else me.base.base.z = tmp_z;
                if (yaw_rel) me.base.base.yaw += tmp_yaw; else me.base.base.yaw = tmp_yaw;
                if (pitch_rel) me.base.base.pitch += tmp_pitch; else me.base.base.pitch = tmp_pitch;
                me.base.base.last_x = me.base.base.x;
                me.base.base.last_y = me.base.base.y;
                me.base.base.last_z = me.base.base.z;
                me.base.base.last_yaw = me.base.base.yaw;
                me.base.base.last_pitch = me.base.base.pitch;
                //printf("Player Position Updated: X=%.2f, Y=%.2f, Z=%.2f, Yaw=%.2f, Pitch=%.2f\n", me.base.base.x, me.base.base.y, me.base.base.z, me.base.base.yaw, me.base.base.pitch);
            }
            me.inited = true;
            //confirm new position
            //int final_len = build_player_pos_and_look_packet(send_pk, me.base.base.x, me.base.base.y, me.base.base.z, me.base.base.yaw, me.base.base.pitch, 1); // move into movement loop later
            //printf("Built Player Position and Look Packet with length %d\n", final_len);
            //print_contents(send_pk, final_len);
            //send(server, (char*)send_pk, final_len, 0);
        } else if (packet_id == 0x44) {
            // worldborder packet, maybe implement later but can be ignored for now
        } else if (packet_id == 0x03) {
            // time update packet, can be ignored
        } else if (packet_id == 0x30) {
            // THIS CAN ALSO BE USED FOR CHESTS, FURNACES, CTABLES, ETC!! THE SERVER WILL ASSIGN A UNIQUE ID TO EACH ONE
            read_and_apply_window_items_packet(dec_packet, dec_len, &dec_pos);
        } else if (packet_id == 0x2F) {
            // set slot, ignore for now since inventory not implemented
            // inventory is implemented now!!! time to make this
            read_and_apply_set_slot_packet(dec_packet, dec_len, &dec_pos);
        } else if (packet_id == 0x00) { // Keep Alive temporarry test
            uint64_t keep_alive_id;
            int len = varint_read_u64(dec_packet, dec_len, &keep_alive_id);
            (void)len;
            
            //printf("Keep Alive: %llu\n", keep_alive_id);
            unsigned char response[32];
            int rpos = 0;
            if (compress_threshold >= 0) {
                rpos += varint_write_u64(response + rpos, 0); 
            }
            rpos += varint_write_u64(response + rpos, 0x00); // ID 0x00
            rpos += varint_write_u64(response + rpos, keep_alive_id); 
            
            unsigned char final_frame[32];
            int final_pos = 0;
            final_pos += varint_write_u64(final_frame + final_pos, rpos);
            memcpy(final_frame + final_pos, response, rpos);
            final_pos += rpos;
            
            send(server, (char*)final_frame, final_pos, 0);
            //printf("Sent Keep Alive Response\n");
        } else if (packet_id == 0x26) { // Map Chunk Bulk
            puts("Received Bulk Chunk Data Packet");
            read_and_apply_bulk_chunk_data_packet_info(dec_packet, dec_len, &dec_pos);
        } else if (packet_id == 0x21) { // Map Chunk
            puts("Received Map Chunk Packet");
            read_and_apply_map_chunk_packet_info(dec_packet, dec_len, &dec_pos);
        } else if (packet_id == 0x22) { // multi block change
            puts("Received Multi Block Change Packet");
            read_and_apply_multi_block_change_packet_info(dec_packet, dec_len, &dec_pos);
        } else if (packet_id == 0x23) { // single block change
            read_and_apply_single_block_change(dec_packet, dec_len, &dec_pos);
        } else if (packet_id == 0x0C) {  // Player Spawn
            int32_t entityId;
            mc_uuid_t uuid;
            double x, y, z;
            float yaw, pitch;
            int16_t itemInHand;
            read_player_spawn_packet(dec_packet, dec_len, &dec_pos, &entityId, &uuid, &x, &y, &z, &yaw, &pitch, &itemInHand);
            //printf("Player Spawned: EntityID=%d, X=%.2f, Y=%.2f, Z=%.2f, Yaw=%.2f, Pitch=%.2f\n", entityId, x, y, z, yaw, pitch);
            //print_uuid(uuid);
            //printf("\n");
            (void)itemInHand; //make compiler happy
            GameEntity *e = allocate_entity(entityId, ENT_PLAYER);
            if (e) {
                e->u.base.x = x;
                e->u.base.y = y;
                e->u.base.z = z;
                e->u.base.yaw = yaw;
                e->u.base.pitch = pitch;
                e->u.base.uuid = uuid;
                e->u.base.entity_type = -1; // player has no entity type so i will use -1 internally
            }
        } else if (packet_id == 0x0F) {  // Mob Spawn
            int32_t entityId;
            uint8_t entityType;
            double x, y, z;
            float yaw, pitch;
            float headPitch;
            double vx, vy, vz;
            read_livingentity_spawn_packet(dec_packet, dec_len, &dec_pos, &entityId, &entityType, &x, &y, &z, &yaw, &pitch, &headPitch, &vx, &vy, &vz);
            //printf("Living Entity Spawned: EntityID=%d, EntityType=%d, X=%.2f, Y=%.2f, Z=%.2f, Yaw=%.2f, Pitch=%.2f, HeadPitch=%.2f, Velocity=(%.2f, %.2f, %.2f)\n", entityId, entityType, x, y, z, yaw, pitch, headPitch, vx, vy, vz);
            (void)headPitch; //make compiler happy
            (void)vx; (void)vy; (void)vz; //make compiler happy
            GameEntity *e = allocate_entity(entityId, ENT_MOB);
            if (e) {
                e->u.base.x = x;
                e->u.base.y = y;
                e->u.base.z = z;
                e->u.base.yaw = yaw;
                e->u.base.pitch = pitch;
                e->u.base.uuid = uuid_nil(); // mobs have no uuid in this protocol version
                e->u.base.entity_type = entityType;
                e->u.mob.head_pitch = headPitch;
            }
        } else if (packet_id == 0x0E) {  // Non-living Spawn
            int32_t entityId;
            uint8_t entityType;
            double x, y, z;
            float yaw, pitch;
            read_nonlivingentity_spawn_packet(dec_packet, dec_len, &dec_pos, &entityId, &entityType, &x, &y, &z, &yaw, &pitch);
            //printf("Non-living Entity Spawned: EntityID=%d, EntityType=%d, X=%.2f, Y=%.2f, Z=%.2f, Yaw=%.2f, Pitch=%.2f\n", entityId, entityType, x, y, z, yaw, pitch);
            GameEntity *e = allocate_entity(entityId, ENT_NONLIVING);
            if (e) {
                e->u.base.x = x;
                e->u.base.y = y;
                e->u.base.z = z;
                e->u.base.yaw = yaw;
                e->u.base.pitch = pitch;
                e->u.base.uuid = uuid_nil(); // mobs have no uuid in this protocol version
                e->u.base.entity_type = entityType + 65536; // offset to avoid clashing with living entity types
            }
        } else if (packet_id == 0x15) {  // Entity Relative Move
            int32_t entityId;
            double dx, dy, dz;
            uint8_t onGround;
            read_rel_entity_move_packet(dec_packet, dec_len, &dec_pos, &entityId, &dx, &dy, &dz, &onGround);
            (void)onGround; //make compiler happy
            GameEntity *e = get_entity(entityId);
            if (e) {
                e->u.base.x += dx;
                e->u.base.y += dy;
                e->u.base.z += dz;
                //printf("Entity %d moved by (%.2f, %.2f, %.2f) to new position (%.2f, %.2f, %.2f)\n", entityId, dx, dy, dz, e->u.base.x, e->u.base.y, e->u.base.z);
            } else {
                //printf("Entity Relative Move for unknown entity ID %d\n", entityId);
            }
        } else if (packet_id == 0x16) {  // Entity Look
            int32_t entityId;
            float yaw, pitch;
            uint8_t onGround;
            read_entity_look_packet(dec_packet, dec_len, &dec_pos, &entityId, &yaw, &pitch, &onGround);
            (void)onGround; //make compiler happy
            GameEntity *e = get_entity(entityId);
            if (e) {
                e->u.base.yaw = yaw;
                e->u.base.pitch = pitch;
                //printf("Entity %d looked to Yaw=%.2f, Pitch=%.2f\n", entityId, yaw, pitch);
            } else {
                //printf("Entity Look for unknown entity ID %d\n", entityId);
            }
        } else if (packet_id == 0x17) {  // Entity Move and Look
            int32_t entityId;
            double dx, dy, dz;
            float yaw, pitch;
            uint8_t onGround;
            read_entity_move_look_packet(dec_packet, dec_len, &dec_pos, &entityId, &dx, &dy, &dz, &yaw, &pitch, &onGround);
            (void)onGround; //make compiler happy
            GameEntity *e = get_entity(entityId);
            if (e) {
                e->u.base.x += dx;
                e->u.base.y += dy;
                e->u.base.z += dz;
                e->u.base.yaw = yaw;
                e->u.base.pitch = pitch;
                //printf("Entity %d moved by (%.2f, %.2f, %.2f) to new position (%.2f, %.2f, %.2f) and looked to Yaw=%.2f, Pitch=%.2f\n", entityId, dx, dy, dz, e->u.base.x, e->u.base.y, e->u.base.z, yaw, pitch);
            } else {
                //printf("Entity Move and Look for unknown entity ID %d\n", entityId);
            }
        } else if (packet_id == 0x18) {  // Entity Teleport
            int32_t entityId;
            double x, y, z;
            float yaw, pitch;
            uint8_t onGround;
            read_entity_teleport_packet(dec_packet, dec_len, &dec_pos, &entityId, &x, &y, &z, &yaw, &pitch, &onGround);
            (void)onGround; //make compiler happy
            GameEntity *e = get_entity(entityId);
            if (e) {
                e->u.base.x = x;
                e->u.base.y = y;
                e->u.base.z = z;
                e->u.base.yaw = yaw;
                e->u.base.pitch = pitch;
                //printf("Entity %d teleported to position (%.2f, %.2f, %.2f) and looked to Yaw=%.2f, Pitch=%.2f\n", entityId, x, y, z, yaw, pitch);
            } else {
                //printf("Entity Teleport for unknown entity ID %d\n", entityId);
            }
        } else if (packet_id == 0x19) {
            // entity head look, can be ignored for now
        } else if (packet_id == 0x1C) {
            // entity metadata, implement later
        } else if (packet_id == 0x13) { // Destroy Entities
            read_and_handle_destroy_entities(dec_packet, &dec_pos);
        }
    }

    return 1;
}

float get_player_camera_yoff(){
    return (me.shift_key_down ? 1.27f : 1.62f);
}

void render_block(Block *block, ChunkData *chunk, int x, int y, int z, const mat4 *vp, int *facecount) {
    if (block->id != 0) {
        // only render a few block types for now
        //printf("Block ID: %d\n", block.id);
        uint32_t color =  block_defs[block->id]->solid_color;// default white
        const BlockTextures *bt = get_block_textures(block->id, block->meta);
        
        float block_x = (chunk->meta.x * 16) + x;
        float block_y = y;
        float block_z = (chunk->meta.z * 16) + z;
        float vertices[8][3] = {
            {block_x, block_y, block_z},
            {block_x + 1, block_y, block_z},
            {block_x, block_y + 1, block_z},
            {block_x + 1, block_y + 1, block_z},
            {block_x, block_y, block_z + 1},
            {block_x + 1, block_y, block_z + 1},
            {block_x, block_y + 1, block_z + 1},
            {block_x + 1, block_y + 1, block_z + 1}
        };

        int sx[8], sy[8], skip[8];
        int skip_face[6] = {1,1,1,1,1,1}; 
        float depth[8];
        for(int i = 0; i < 8; i++){
            if (get_coords_from_3d(&sx[i], &sy[i], &depth[i], vertices[i][0], vertices[i][1], vertices[i][2], *vp) > 0) {
                skip[i] = 0; 
            } else {
                skip[i] = 1;
            }
        }

        int flags = block->render_flags;
        if (flags & RENDER_FACE_TOP) {
            skip_face[2] = 0;
            facecount += 1;
        }
        if (flags & RENDER_FACE_BOTTOM) {
            skip_face[3] = 0;
            facecount += 1;
        }
        if (flags & RENDER_FACE_WEST) {
            skip_face[5] = 0;
            facecount += 1;
        }
        if (flags & RENDER_FACE_EAST) {
            skip_face[4] = 0;
            facecount += 1;
        }
        if (flags & RENDER_FACE_NORTH) {
            skip_face[0] = 0;
            facecount += 1;
        }
        if (flags & RENDER_FACE_SOUTH) {
            skip_face[1] = 0;
            facecount += 1;
        }

        //distance from player (me variable)
        float distancefromplayer = sqrtf((block_x - me.base.base.x) * (block_x - me.base.base.x) + (block_y - me.base.base.y) * (block_y - me.base.base.y) + (block_z - me.base.base.z) * (block_z - me.base.base.z));
        /**/
        //draw faces
        if (color != 0x0 && 0) {
            // Front Face: 0, 2, 3, 1 (CW)
            if (!skip[0] && !skip[2] && !skip[3] && !skip[1] && !skip_face[0]) {
                draw_quad_z(sx[0], sy[0], depth[0], sx[2], sy[2], depth[2], sx[3], sy[3], depth[3], sx[1], sy[1], depth[1], color); 
            }
            // Back Face: 5, 7, 6, 4 (CW)
            if(!skip[5] && !skip[7] && !skip[6] && !skip[4] && !skip_face[1]) {
                draw_quad_z(sx[5], sy[5], depth[5], sx[7], sy[7], depth[7], sx[6], sy[6], depth[6], sx[4], sy[4], depth[4], color); 
            }
            // Top Face: 2, 6, 7, 3 (CW)
            if(!skip[2] && !skip[6] && !skip[7] && !skip[3] && !skip_face[2]) {
                draw_quad_z(sx[2], sy[2], depth[2], sx[6], sy[6], depth[6], sx[7], sy[7], depth[7], sx[3], sy[3], depth[3], color); 
            }
            // Bottom Face: 0, 1, 5, 4 (CW)
            if(!skip[0] && !skip[1] && !skip[5] && !skip[4] && !skip_face[3]) {
                draw_quad_z(sx[0], sy[0], depth[0], sx[1], sy[1], depth[1], sx[5], sy[5], depth[5], sx[4], sy[4], depth[4], color); 
            }
            // Right Face: 1, 3, 7, 5 (Already CW)
            if(!skip[1] && !skip[3] && !skip[7] && !skip[5] && !skip_face[4]) {
                draw_quad_z(sx[1], sy[1], depth[1], sx[3], sy[3], depth[3], sx[7], sy[7], depth[7], sx[5], sy[5], depth[5], color); 
            }
            // Left Face: 4, 6, 2, 0 (CW)
            if(!skip[4] && !skip[6] && !skip[2] && !skip[0] && !skip_face[5]) {
                draw_quad_z(sx[4], sy[4], depth[4], sx[6], sy[6], depth[6], sx[2], sy[2], depth[2], sx[0], sy[0], depth[0], color); 
            }
        } else if (bt && 1) {
            if (block_defs[block->id]->model == MODEL_CUBE || block_defs[block->id]->model == MODEL_LEAVES || block_defs[block->id]->model == MODEL_SLIME || block_defs[block->id]->model == MODEL_GRASS) {
                // Front
                if (distancefromplayer < dist_before_solid_color){
                    if (!skip[0] && !skip[2] && !skip[3] && !skip[1] && !skip_face[0]) {
                        draw_quad_tex_z(sx[0], sy[0], depth[0], 0.0f, 1.0f, sx[2], sy[2], depth[2], 0.0f, 0.0f, sx[3], sy[3], depth[3], 1.0f, 0.0f, sx[1], sy[1], depth[1], 1.0f, 1.0f, bt->south); 
                    }
                    // Back Face: 5, 7, 6, 4 (CW)
                    if(!skip[5] && !skip[7] && !skip[6] && !skip[4] && !skip_face[1]) {
                        draw_quad_tex_z(sx[5], sy[5], depth[5], 0.0f, 1.0f, sx[7], sy[7], depth[7], 0.0f, 0.0f, sx[6], sy[6], depth[6], 1.0f, 0.0f, sx[4], sy[4], depth[4], 1.0f, 1.0f, bt->north); 
                    }
                    // Top Face: 2, 6, 7, 3 (CW)
                    if(!skip[2] && !skip[6] && !skip[7] && !skip[3] && !skip_face[2]) {
                        draw_quad_tex_z(sx[2], sy[2], depth[2], 0.0f, 0.0f, sx[6], sy[6], depth[6], 1.0f, 0.0f, sx[7], sy[7], depth[7], 1.0f, 1.0f, sx[3], sy[3], depth[3], 0.0f, 1.0f, bt->top); 
                    }
                    // Bottom Face: 0, 1, 5, 4 (CW)
                    if(!skip[0] && !skip[1] && !skip[5] && !skip[4] && !skip_face[3]) {
                        draw_quad_tex_z(sx[0], sy[0], depth[0], 0.0f, 0.0f, sx[1], sy[1], depth[1], 1.0f, 0.0f, sx[5], sy[5], depth[5], 1.0f, 1.0f, sx[4], sy[4], depth[4], 0.0f, 1.0f, bt->bottom); 
                    }
                    // Right Face: 1, 3, 7, 5 (Already CW)
                    if(!skip[1] && !skip[3] && !skip[7] && !skip[5] && !skip_face[4]) {
                        draw_quad_tex_z(sx[1], sy[1], depth[1], 0.0f, 1.0f, sx[3], sy[3], depth[3], 0.0f, 0.0f, sx[7], sy[7], depth[7], 1.0f, 0.0f, sx[5], sy[5], depth[5], 1.0f, 1.0f, bt->east); 
                    }
                    // Left Face: 4, 6, 2, 0 (CW)
                    if(!skip[4] && !skip[6] && !skip[2] && !skip[0] && !skip_face[5]) {
                        draw_quad_tex_z(sx[4], sy[4], depth[4], 0.0f, 1.0f, sx[6], sy[6], depth[6], 0.0f, 0.0f, sx[2], sy[2], depth[2], 1.0f, 0.0f, sx[0], sy[0], depth[0], 1.0f, 1.0f, bt->west); 
                    }
                } else if (distancefromplayer < dist_before_wireframe) {
                    uint32_t colorofb = block_defs[block->id][block->meta].solid_color;
                    if (!skip[0] && !skip[2] && !skip[3] && !skip[1] && !skip_face[0]) {
                        draw_quad_z_optimized(sx[0], sy[0], depth[0], sx[2], sy[2], depth[2], sx[3], sy[3], depth[3], sx[1], sy[1], depth[1], colorofb); 
                    }
                    // Back Face: 5, 7, 6, 4 (CW)
                    if(!skip[5] && !skip[7] && !skip[6] && !skip[4] && !skip_face[1]) {
                        draw_quad_z_optimized(sx[5], sy[5], depth[5], sx[7], sy[7], depth[7], sx[6], sy[6], depth[6], sx[4], sy[4], depth[4], colorofb); 
                    }
                    // Top Face: 2, 6, 7, 3 (CW)
                    if(!skip[2] && !skip[6] && !skip[7] && !skip[3] && !skip_face[2]) {
                        draw_quad_z_optimized(sx[2], sy[2], depth[2], sx[6], sy[6], depth[6], sx[7], sy[7], depth[7], sx[3], sy[3], depth[3], colorofb); 
                    }
                    // Bottom Face: 0, 1, 5, 4 (CW)
                    if(!skip[0] && !skip[1] && !skip[5] && !skip[4] && !skip_face[3]) {
                        draw_quad_z_optimized(sx[0], sy[0], depth[0], sx[1], sy[1], depth[1], sx[5], sy[5], depth[5], sx[4], sy[4], depth[4], colorofb); 
                    }
                    // Right Face: 1, 3, 7, 5 (Already CW)
                    if(!skip[1] && !skip[3] && !skip[7] && !skip[5] && !skip_face[4]) {
                        draw_quad_z_optimized(sx[1], sy[1], depth[1], sx[3], sy[3], depth[3], sx[7], sy[7], depth[7], sx[5], sy[5], depth[5], colorofb); 
                    }
                    // Left Face: 4, 6, 2, 0 (CW)
                    if(!skip[4] && !skip[6] && !skip[2] && !skip[0] && !skip_face[5]) {
                        draw_quad_z_optimized(sx[4], sy[4], depth[4], sx[6], sy[6], depth[6], sx[2], sy[2], depth[2], sx[0], sy[0], depth[0], colorofb); 
                    }
                } else {
                    uint32_t colorofb = block_defs[block->id][block->meta].solid_color;
                    if (!skip[0] && !skip[2] && !skip[3] && !skip[1] && !skip_face[0]) {
                        draw_quad_z_wire(sx[0], sy[0], depth[0], sx[2], sy[2], depth[2], sx[3], sy[3], depth[3], sx[1], sy[1], depth[1], colorofb); 
                    }
                    // Back Face: 5, 7, 6, 4 (CW)
                    if(!skip[5] && !skip[7] && !skip[6] && !skip[4] && !skip_face[1]) {
                        draw_quad_z_wire(sx[5], sy[5], depth[5], sx[7], sy[7], depth[7], sx[6], sy[6], depth[6], sx[4], sy[4], depth[4], colorofb); 
                    }
                    // Top Face: 2, 6, 7, 3 (CW)
                    if(!skip[2] && !skip[6] && !skip[7] && !skip[3] && !skip_face[2]) {
                        draw_quad_z_wire(sx[2], sy[2], depth[2], sx[6], sy[6], depth[6], sx[7], sy[7], depth[7], sx[3], sy[3], depth[3], colorofb); 
                    }
                    // Bottom Face: 0, 1, 5, 4 (CW)
                    if(!skip[0] && !skip[1] && !skip[5] && !skip[4] && !skip_face[3]) {
                        draw_quad_z_wire(sx[0], sy[0], depth[0], sx[1], sy[1], depth[1], sx[5], sy[5], depth[5], sx[4], sy[4], depth[4], colorofb); 
                    }
                    // Right Face: 1, 3, 7, 5 (Already CW)
                    if(!skip[1] && !skip[3] && !skip[7] && !skip[5] && !skip_face[4]) {
                        draw_quad_z_wire(sx[1], sy[1], depth[1], sx[3], sy[3], depth[3], sx[7], sy[7], depth[7], sx[5], sy[5], depth[5], colorofb); 
                    }
                    // Left Face: 4, 6, 2, 0 (CW)
                    if(!skip[4] && !skip[6] && !skip[2] && !skip[0] && !skip_face[5]) {
                        draw_quad_z_wire(sx[4], sy[4], depth[4], sx[6], sy[6], depth[6], sx[2], sy[2], depth[2], sx[0], sy[0], depth[0], colorofb); 
                    }
                }
            } else if (block_defs[block->id]->model == MODEL_CROSS || block_defs[block->id]->model == MODEL_TALLGRASS || block_defs[block->id]->model == MODEL_CROP) {
                if (distancefromplayer < dist_before_solid_color){
                    // First Quad: 0, 6, 7, 1 <- this is incorrect, it actually should be 0, 2, 7, 5
                    if(!skip[0] && !skip[2] && !skip[7] && !skip[5]) {
                        draw_quad_tex_z(sx[0], sy[0], depth[0], 0.0f, 1.0f, sx[2], sy[2], depth[2], 0.0f, 0.0f, sx[7], sy[7], depth[7], 1.0f, 0.0f, sx[5], sy[5], depth[5], 1.0f, 1.0f, bt->north); 
                    }
                    // Second Quad: 4, 2, 3, 5 (CW) <- also incorrect, should be 1, 3, 6, 4
                    if(!skip[1] && !skip[3] && !skip[6] && !skip[4]) {
                        draw_quad_tex_z(sx[1], sy[1], depth[1], 0.0f, 1.0f, sx[3], sy[3], depth[3], 0.0f, 0.0f, sx[6], sy[6], depth[6], 1.0f, 0.0f, sx[4], sy[4], depth[4], 1.0f, 1.0f, bt->north); 
                    }
                } else if (distancefromplayer < dist_before_wireframe) {
                    /*
                    uint32_t colorofb = block_defs[block->id][block->meta].solid_color;
                    // First Quad: 0, 6, 7, 1 <- this is incorrect, it actually should be 0, 2, 7, 5
                    if(!skip[0] && !skip[2] && !skip[7] && !skip[5]) {
                        draw_quad_z_optimized(sx[0], sy[0], depth[0], sx[2], sy[2], depth[2], sx[7], sy[7], depth[7], sx[5], sy[5], depth[5], colorofb); 
                    }
                    // Second Quad: 4, 2, 3, 5 (CW) <- also incorrect, should be 1, 3, 6, 4
                    if(!skip[1] && !skip[3] && !skip[6] && !skip[4]) {
                        draw_quad_z_optimized(sx[1], sy[1], depth[1], sx[3], sy[3], depth[3], sx[6], sy[6], depth[6], sx[4], sy[4], depth[4], colorofb); 
                    }*/
                } else {
                    /*
                    uint32_t colorofb = block_defs[block->id][block->meta].solid_color;
                    // First Quad: 0, 6, 7, 1 <- this is incorrect, it actually should be 0, 2, 7, 5
                    if(!skip[0] && !skip[2] && !skip[7] && !skip[5]) {
                        draw_quad_z_wire(sx[0], sy[0], depth[0], sx[2], sy[2], depth[2], sx[7], sy[7], depth[7], sx[5], sy[5], depth[5], colorofb); 
                    }
                    // Second Quad: 4, 2, 3, 5 (CW) <- also incorrect, should be 1, 3, 6, 4
                    if(!skip[1] && !skip[3] && !skip[6] && !skip[4]) {
                        draw_quad_z_wire(sx[1], sy[1], depth[1], sx[3], sy[3], depth[3], sx[6], sy[6], depth[6], sx[4], sy[4], depth[4], colorofb); 
                    }*/
                }
            }
        }
    }
}

void render_inventory_slot(int x, int y, uint32_t *pxbuf, int8_t szmultiplier) {
    //draw the slot background
    uint32_t color = 0xffcccccc;
    const uint32_t *texture = item_tex_slot;
    for (int j = 0; j < 18; j++) {
        for (int i = 0; i < 18; i++) {
            for (int xoff = 0; xoff < szmultiplier; xoff++) {
                for (int yoff = 0; yoff < szmultiplier; yoff++) {
                    int px2 = x + i*szmultiplier + xoff;
                    int py2 = y + j*szmultiplier + yoff;
                    if (px2 >= 0 && px2 < RENDER_W && py2 >= 0 && py2 < RENDER_H) {
                        if (texture) { 
                            if (texture[j * 18 + i] & 0x00FFFFFF) { 
                                pxbuf[py2 * RENDER_W + px2] = texture[j * 18 + i];
                            } else {
                                // transparent pixel, don't draw
                            }
                        } else {
                            pxbuf[py2 * RENDER_W + px2] = color;
                        }
                    }
                }
            }
            
        }
    }
}
void render_inventory_sel(int x, int y, uint32_t *pxbuf, int8_t szmultiplier) {
    //draw the slot background
    uint32_t color = 0xffcccccc;
    const uint32_t *texture = item_tex_slotsel;
    for (int j = 0; j < 22; j++) {
        for (int i = 0; i < 22; i++) {
            for (int xoff = 0; xoff < szmultiplier; xoff++) {
                for (int yoff = 0; yoff < szmultiplier; yoff++) {
                    int px2 = x + i*szmultiplier + xoff;
                    int py2 = y + j*szmultiplier + yoff;
                    if (px2 >= 0 && px2 < RENDER_W && py2 >= 0 && py2 < RENDER_H) {
                        if (texture) { 
                            if (texture[j * 22 + i] & 0x00FFFFFF) { 
                                pxbuf[py2 * RENDER_W + px2] = texture[j * 22 + i];
                            } else {
                                // transparent pixel, don't draw
                            }
                        } else {
                            pxbuf[py2 * RENDER_W + px2] = color;
                        }
                    }
                }
            }
            
        }
    }
}


void render_inventory_item(SlotItem item, int x, int y, uint32_t *pxbuf, int8_t szmultiplier) {
    if (item.id != 0 && item_defs[item.id][0].texture != NULL) {
        uint32_t color = item_defs[item.id][0].solid_color;
        const uint32_t *texture = item_defs[item.id][0].texture;
        for (int j = 0; j < 16; j++) {
            for (int i = 0; i < 16; i++) {
                for (int xoff = 0; xoff < szmultiplier; xoff++) {
                    for (int yoff = 0; yoff < szmultiplier; yoff++) {
                        int px2 = x + i*szmultiplier + xoff;
                        int py2 = y + j*szmultiplier + yoff;
                        if (px2 >= 0 && px2 < RENDER_W && py2 >= 0 && py2 < RENDER_H) {
                            if (texture) { 
                                if (texture[j * 16 + i] & 0x00FFFFFF) { 
                                    pxbuf[py2 * RENDER_W + px2] = texture[j * 16 + i];
                                } else {
                                    // transparent pixel, don't draw
                                }
                            } else {
                                pxbuf[py2 * RENDER_W + px2] = color;
                            }
                        }
                    }
                }
                
            }
        }
    }
}

int render_frame(uint32_t *pxbuf, int width, int height) {
    //struct timeval stop, start;
    //gettimeofday(&start, NULL);

    //background
    for (int i = 0; i < width * height; i++) {
        pxbuf[i] = 0x0077ff; 
        zbuffer_new[i] = 0; //clear zbuffer - optimised one now requires init to be 0
    }
    pixels_rendered = 0;
    //perspective matrix
    float yaw_rad = me.base.base.interp_yaw * (M_PI / 180.0f);
    float pitch_rad = me.base.base.interp_pitch * (M_PI / 180.0f);
    float look_x = -sinf(yaw_rad) * cosf(pitch_rad);
    float look_y = -sinf(pitch_rad);
    float look_z = cosf(yaw_rad) * cosf(pitch_rad);
    float eye[3] = { (float)me.base.base.interp_x, (float)me.base.base.interp_y + get_player_camera_yoff(), (float)me.base.base.interp_z }; // +1.62 for eye height
    float center[3] = { eye[0] + look_x, eye[1] + look_y, eye[2] + look_z };
    float up[3] = { 0.0f, 1.0f, 0.0f };
    mat4 view; 
    view = mat4_lookAt(view, eye, center, up);
    mat4 proj;
    proj = mat4_perspective(proj, 70.0f * (M_PI/180.0f), (float)RENDER_W/(float)RENDER_H, 0.1f, 1000.0f);
    mat4 vp = mat4_mul(proj, view);
    int facecount = 0;
    //draw cubes
    if (dist_mode == 0){ // the "stupid" mode
        for (int ox = -1; ox <= 1; ox++) {
            for (int oz = -1; oz <= 1; oz++) {
                ChunkData *chunk = get_chunk(((int)me.base.base.interp_x >> 4) + ox, ((int)me.base.base.interp_z >> 4) + oz);
                if (chunk && chunk->initialized) {
                    for (int y = 0; y < 256; y++) {
                        for (int z = 0; z < 16; z++) {
                            for (int x = 0; x < 16; x++) {
                                Block block = chunk->data.blocks[y][z][x];
                                if (block.id == 0 || block.render_flags == RENDER_SKIP_ALL) {
                                    continue; 
                                }
                                render_block(&block, chunk, x, y, z, &vp, &facecount);
                            }
                        }
                    }
                }
            }
        }
    } else if (dist_mode == 1) { // blocks mode (renders around player in a circle)
        int player_min_chunk_x = floor(((me.base.base.interp_x - renderer_distance)) / 16);
        int player_max_chunk_x = ceil(((me.base.base.interp_x + renderer_distance)) / 16);
        int player_min_chunk_z = floor(((me.base.base.interp_z - renderer_distance)) / 16);
        int player_max_chunk_z = ceil(((me.base.base.interp_z + renderer_distance)) / 16);
        int miny = max(0, (int)floor(me.base.base.interp_y - renderer_distance));
        int maxy = min(255, (int)ceil(me.base.base.interp_y + renderer_distance));
        for (int cx = player_min_chunk_x; cx <= player_max_chunk_x; cx++) {
            for (int cz = player_min_chunk_z; cz <= player_max_chunk_z; cz++) {
                ChunkData *chunk = get_chunk(cx, cz);
                if (chunk && chunk->initialized) {
                    for (int y = miny; y <= maxy; y++) {
                        for (int z = 0; z < 16; z++) {
                            for (int x = 0; x < 16; x++) {
                                Block block = chunk->data.blocks[y][z][x];
                                if (block.id == 0 || block.render_flags == RENDER_SKIP_ALL) {
                                    continue; 
                                }
                                int world_x = (chunk->meta.x * 16) + x;
                                int world_z = (chunk->meta.z * 16) + z;
                                int dx = (int)(world_x - me.base.base.interp_x);
                                int dz = (int)(world_z - me.base.base.interp_z);
                                if (dx*dx + dz*dz <= renderer_distance * renderer_distance) {
                                    render_block(&block, chunk, x, y, z, &vp, &facecount);
                                }
                            }
                        }
                    }
                }
            }
        }
    } // 2 not implemented yet

    // render cursor
    
    //puts("Highlighting block under cursor");
    //printf("Highlighting block at (%d, %d, %d)\n", world_x, y, world_z);
    //highlight block
    uint32_t highlight_color = 0xFFFFFF; // white
    //inflate bound by 0.01 to prevent z-fighting
    float block_x = (float)me.cursor_on_block_x;
    float block_y = (float)me.cursor_on_block_y;
    float block_z = (float)me.cursor_on_block_z;
    float vertices[8][3] = {
        {block_x - 0.01f, block_y - 0.01f, block_z - 0.01f},
        {block_x + 1.01f, block_y - 0.01f, block_z - 0.01f},
        {block_x - 0.01f, block_y + 1.01f, block_z - 0.01f},
        {block_x + 1.01f, block_y + 1.01f, block_z - 0.01f},
        {block_x - 0.01f, block_y - 0.01f, block_z + 1.01f},
        {block_x + 1.01f, block_y - 0.01f, block_z + 1.01f},
        {block_x - 0.01f, block_y + 1.01f, block_z + 1.01f},
        {block_x + 1.01f, block_y + 1.01f, block_z + 1.01f}
    };
    int sx[8], sy[8], skip[8];
    float depth[8];
    for(int i = 0; i < 8; i++){
        if (get_coords_from_3d(&sx[i], &sy[i], &depth[i], vertices[i][0], vertices[i][1], vertices[i][2], vp) > 0) {
            skip[i] = 0;
        } else {
            skip[i] = 1;
        }
    }
    if (!skip[0] && !skip[1] && !skip[2] && !skip[3]) {
        // Front Face: 0, 2, 3, 1 (CW)
        draw_quad_z_wire(sx[0], sy[0], depth[0], sx[2], sy[2], depth[2], sx[3], sy[3], depth[3], sx[1], sy[1], depth[1], highlight_color); 
    }
    // Back Face: 5, 7, 6, 4 (CW)
    if (!skip[5] && !skip[7] && !skip[6] && !skip[4]) {
        draw_quad_z_wire(sx[5], sy[5], depth[5], sx[7], sy[7], depth[7], sx[6], sy[6], depth[6], sx[4], sy[4], depth[4], highlight_color); 
    }
    if (!skip[2] && !skip[6] && !skip[7] && !skip[3]) {
        // Top Face: 2, 6, 7, 3 (CW)
        draw_quad_z_wire(sx[2], sy[2], depth[2], sx[6], sy[6], depth[6], sx[7], sy[7], depth[7], sx[3], sy[3], depth[3], highlight_color);
    }
    if (!skip[0] && !skip[1] && !skip[5] && !skip[4]) {
        // Bottom Face: 0, 1, 5, 4 (CW)
        draw_quad_z_wire(sx[0], sy[0], depth[0], sx[1], sy[1], depth[1], sx[5], sy[5], depth[5], sx[4], sy[4], depth[4], highlight_color); 
    }
    if (!skip[1] && !skip[3] && !skip[7] && !skip[5]) {
        // Right Face: 1, 3, 7, 5 (Already CW)
        draw_quad_z_wire(sx[1], sy[1], depth[1], sx[3], sy[3], depth[3], sx[7], sy[7], depth[7], sx[5], sy[5], depth[5], highlight_color); 
    }
    if (!skip[4] && !skip[6] && !skip[2] && !skip[0]) {
        // Left Face: 4, 6, 2, 0 (CW)
        draw_quad_z_wire(sx[4], sy[4], depth[4], sx[6], sy[6], depth[6], sx[2], sy[2], depth[2], sx[0], sy[0], depth[0], highlight_color);
    }
                                    

    //gettimeofday(&stop, NULL);
    //double elapsed = (stop.tv_sec - start.tv_sec) * 1000.0 + (stop.tv_usec - start.tv_usec) / 1000.0;
    //printf("Rendered frame in %.2f ms (%d faces)\n", elapsed, facecount);
    if (display_info_type > 0){
        draw_text6x8_scaled(pxbuf, RENDER_W, RENDER_H, 3, 3, debug_menu_buf, 1, 0xFFFFFF);
    }
    //printf("Pixels rendered: %d\n", pixels_rendered);
    return 1;
}

static inline int u32_to_str(char *out, unsigned int x) {
    char buf[10];
    int i = 0;

    do {
        buf[i++] = '0' + (x % 10);
        x /= 10;
    } while (x);

    for (int j = 0; j < i; j++)
        out[j] = buf[i - j - 1];

    out[i] = '\0';  
    return i;
}


Vector3 pitch_yaw_to_forward(float pitch, float yaw) {
    Vector3 forward;
    
    forward.x = cos(pitch) * sin(yaw);
    forward.y = sin(pitch);
    forward.z = cos(pitch) * cos(yaw);
    
    return forward;
}

Vector3 pitch_yaw_to_forward_degrees(float pitch_deg, float yaw_deg) {
    float pitch = pitch_deg * M_PI / 180.0f;
    float yaw = yaw_deg * M_PI / 180.0f;
    return pitch_yaw_to_forward(pitch * -1, yaw * -1);
}

Vector3 pitch_yaw_to_forward_no_y(float pitch, float yaw) {
    Vector3 forward;
    
    forward.x = cos(pitch) * sin(yaw);
    forward.z = cos(pitch) * cos(yaw);
    
    return forward;
}

Vector3 pitch_yaw_to_forward_degrees_no_y(float pitch_deg, float yaw_deg) {
    float pitch = pitch_deg * M_PI / 180.0f;
    float yaw = yaw_deg * M_PI / 180.0f;
    return pitch_yaw_to_forward_no_y(pitch, yaw * -1); // Invert yaw for no fucking reason it just works better
}


int render_gui_and_inventory(uint32_t *pxbuf, int width, int height) {
    if (1) { // horbar
        // inventnory
        // 0-4 = crafting
        // 5-8 = armor
        // 9-35 = main inventory
        // 36-44 = hotbar
        //draw hotbar items

        //printf("Selected slot: %d\n", me.selected_slot);
        //item_tex_slotsel 
        for (int slot = 36; slot < 45; slot++) {
            int slot_x = ((width / 2 / gui_size_mp) - 72 + (slot-36)*18) * gui_size_mp;
            int slot_y = height - 20 * gui_size_mp;
            render_inventory_slot(slot_x, slot_y, pxbuf, gui_size_mp); // slot background
            if (me.inventory.inv[slot].id != 0) {
                render_inventory_item(me.inventory.inv[slot], slot_x + 1 * gui_size_mp, slot_y + 1 * gui_size_mp, pxbuf, gui_size_mp); // item
                char count_str[10];
                int count_len = u32_to_str(count_str, me.inventory.inv[slot].count);
                //printf("Count string for slot %d: %s\n", slot, count_str);  
                if (count_len > 1) {
                    draw_num5x8_scaled(pxbuf, width, height, slot_x + 5 * gui_size_mp, slot_y + 10 * gui_size_mp, count_str, gui_size_mp, 0xFFFFFF); // item count
                } else {
                    draw_num5x8_scaled(pxbuf, width, height, slot_x + 10 * gui_size_mp, slot_y + 10 * gui_size_mp, count_str, gui_size_mp, 0xFFFFFF); // item count
                }
            }
        }
        int slot_x2 = ((width / 2 / gui_size_mp) - 72 + (me.selected_slot)*18) * gui_size_mp;
        int slot_y2 = height - 20 * gui_size_mp;
        render_inventory_sel(slot_x2 - 2 * gui_size_mp, slot_y2 - 2 * gui_size_mp, pxbuf, gui_size_mp);
    }
    if (0){//render camera ray
        Vector3 forward = pitch_yaw_to_forward_degrees(me.base.base.pitch, me.base.base.yaw); //test without me.base.base.pitch for now
        Vector3 ray_pos = {me.base.base.x, me.base.base.y + 1.6f, me.base.base.z}; // eye position
        Vector3 ray_dest = {ray_pos.x + forward.x * 5.0f, ray_pos.y + forward.y * 5.0f, ray_pos.z + forward.z * 5.0f}; // extend the ray out to 5 blocks distance
        Vector3 ray_start = {ray_pos.x + forward.x * 1.0f, ray_pos.y + forward.y * 1.0f, ray_pos.z + forward.z * 1.0f};
        int sx, sy;
        float depth;
        int sx2, sy2;
        float depth2;
        float yaw_rad = me.base.base.interp_yaw * (M_PI / 180.0f);
        float pitch_rad = me.base.base.interp_pitch * (M_PI / 180.0f);
        float look_x = -sinf(yaw_rad) * cosf(pitch_rad);
        float look_y = -sinf(pitch_rad);
        float look_z = cosf(yaw_rad) * cosf(pitch_rad);
        float eye[3] = { (float)me.base.base.interp_x, (float)me.base.base.interp_y + 1.62f, (float)me.base.base.interp_z }; // +1.62 for eye height
        float center[3] = { eye[0] + look_x, eye[1] + look_y, eye[2] + look_z };
        float up[3] = { 0.0f, 1.0f, 0.0f };
        mat4 view; 
        view = mat4_lookAt(view, eye, center, up);
        mat4 proj;
        proj = mat4_perspective(proj, 70.0f * (M_PI/180.0f), (float)RENDER_W/(float)RENDER_H, 0.1f, 1000.0f);
        mat4 vp = mat4_mul(proj, view);
        get_coords_from_3d(&sx, &sy, &depth, ray_dest.x, ray_dest.y, ray_dest.z, vp);
        get_coords_from_3d(&sx2, &sy2, &depth2, ray_start.x, ray_start.y, ray_start.z, vp);
        draw_line_z_fast(sx, sy, depth, sx2, sy2, depth2, 0xFF0000); // red line for the ray
    }
    return 1;
}

void GetClientAreaSize(HWND hWnd, int *width, int *height) {
    RECT clientRect;
    if (GetClientRect(hWnd, &clientRect)) {
        *width = clientRect.right - clientRect.left;
        *height = clientRect.bottom - clientRect.top;
    } else {
        *width = 0;
        *height = 0;
        // std::cerr << "GetClientRect failed!" << std::endl;
    }
}

int paint_window(HWND hwnd, uint32_t *pxbuf, int width, int height) {
    HDC hdc = GetDC(hwnd);
    StretchDIBits(hdc, 0, 0, width, height, 0, 0, RENDER_W, RENDER_H, pxbuf, &bmi, DIB_RGB_COLORS, SRCCOPY);
    ReleaseDC(hwnd, hdc);
    return 1;
}


LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_CLOSE:
        case WM_DESTROY:
            running = false;
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}
/*
typedef struct {
    uint16_t id;
    bool is_full_opaque; // For culling optimization
    bool has_transparency; // For rendering order
    BlockModelType model;
    BlockTextures text; // Up, Down, N, S, E, W
    int solid_color; // Used if texture_id is -1
    CollisionShape collision_shape; // refer to collision_shapes.h for details
    // collision later
} BlockDef;
*/
int createblockdefs() {
    for (int i = 0; i < 256; i++) {
        for (int m = 0; m < 16; m++) {
            block_defs[i][m].is_full_opaque = false;
            block_defs[i][m].text = get_block_textures(i, 0); //0 because I don't have block state textures yet
            block_defs[i][m].collision_shape_id = get_collision_shape_id(i, m);
            if (block_defs[i][m].text != NULL && block_defs[i][m].text->top != NULL){
                block_defs[i][m].solid_color = (block_defs[i][m].text->top)[0*16 + 0]; // y * 16 + x
            } else {
                block_defs[i][m].solid_color = 0xFFFFFF; // default white
            }
            block_defs[i][m].id = i;
            block_defs[i][m].model = get_block_model_type(i);
            block_defs[i][m].is_full_opaque = get_is_full_opaque(i);
            if (block_defs[i][m].model == MODEL_CROSS || block_defs[i][m].model == MODEL_TALLGRASS || block_defs[i][m].model == MODEL_CROP) {
                block_defs[i][m].is_full_opaque = false;
            }
            block_defs[i][m].has_transparency = false; // not used
            block_defs[i][m].mineable = get_block_minable(i);
            block_defs[i][m].hardness = get_block_hardness(i);
            block_defs[i][m].stackSize = get_block_stack_size(i);
        }
    }
    for (int i = 0; i < 512; i++) {
        for (int m = 0; m < 1; m++) {
            //printf("Creating item def for ID %d\n", i);
            //puts("Set ID");
            item_defs[i][m].id = i;
            if (i < 256) {
                //puts("Texture 1");
                if (get_block_textures(i, 0) != NULL && get_block_textures(i, 0)->north != NULL) {
                    item_defs[i][m].texture = get_block_textures(i, 0)->north;
                }
                item_defs[i][m].solid_color = 0x00000000; // not implemented yet
            } else {
                //puts("Texture 2");
                item_defs[i][m].texture = get_item_texture(i);
                item_defs[i][m].solid_color = 0x00000000; // not implemented yet
            }
            //food? - foods.json

            //puts("Stack size");
            item_defs[i][m].stackSize = stack_size(i);
        }
    }
    return 1;
}

void LockCursorToWindow(HWND hwnd)
{
    while (ShowCursor(FALSE) >= 0);

    RECT rect;
    GetClientRect(hwnd, &rect);

    POINT ul = { rect.left, rect.top };
    POINT lr = { rect.right, rect.bottom };

    ClientToScreen(hwnd, &ul);
    ClientToScreen(hwnd, &lr);

    RECT clip = { ul.x, ul.y, lr.x, lr.y };
    ClipCursor(&clip);
}

void UnlockCursor()
{
    ClipCursor(NULL);
    while (ShowCursor(TRUE) < 0);
}

void RegisterRawMouse(HWND hwnd) {
    RAWINPUTDEVICE rid;
    rid.usUsagePage = 0x01; // Generic desktop controls
    rid.usUsage     = 0x02; // Mouse
    rid.dwFlags     = RIDEV_INPUTSINK; // Receive even when not focused (optional)
    rid.hwndTarget  = hwnd;

    RegisterRawInputDevices(&rid, 1, sizeof(rid));
}

void add_clientplayer_input_event(uint8_t event_type, uint8_t keycode, uint8_t action) {
    for (int i = 0; i < MAX_INPUT_EVENTS; i++) {
        if (me.input_events[i].type == 0) { 
            me.input_events[i].type = event_type;
            me.input_events[i].code = keycode;
            me.input_events[i].action = action; // shou;ld always be 0
            break;
        }
    }
}


LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam){
    switch (msg){
        case WM_INPUT:
        {
            UINT size = 0;
            GetRawInputData((HRAWINPUT)lParam, RID_INPUT, NULL, &size, sizeof(RAWINPUTHEADER));

            BYTE buffer[sizeof(RAWINPUT)];
            if (size > sizeof(buffer)) break;

            if (GetRawInputData((HRAWINPUT)lParam, RID_INPUT, buffer, &size, sizeof(RAWINPUTHEADER)) != size)
                break;

            RAWINPUT* raw = (RAWINPUT*)buffer;

            if (raw->header.dwType == RIM_TYPEMOUSE && mouse_locked) {
                LONG dx = raw->data.mouse.lLastX;
                LONG dy = raw->data.mouse.lLastY;
                mouse_dx += dx*sensitivity;
                mouse_dy += dy*sensitivity;

            }
            break;
        }

        case WM_SETFOCUS:
            LockCursorToWindow(hwnd);
            break;

        case WM_KILLFOCUS:
            UnlockCursor();
            break;

        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        case WM_KEYDOWN:
            {
                if (mouse_locked){
                    if (wParam == VK_ESCAPE){
                        UnlockCursor();
                        mouse_locked = false;
                    }
                    if (wParam == 'P'){
                        dist_before_wireframe = 0;
                        dist_before_solid_color = 0;
                    }
                    if (wParam == 'O'){
                        dist_before_wireframe = 127;
                        dist_before_solid_color = 0;
                    }
                    if (wParam == 'I'){
                        dist_before_wireframe = 127;
                        dist_before_solid_color = 127;
                    }
                    if (wParam == VK_F3){
                        display_info_type += 1;
                        if (display_info_type > 3) display_info_type = 0;
                    }
                }
                break;
            }
        case WM_KEYUP: {
            /*switch (wParam) {
                case VK_RETURN:
                    break;
                case VK_SHIFT:
                    break;
            }*/
            break;
        }
        case WM_LBUTTONDOWN: {
            if (!mouse_locked) {
                LockCursorToWindow(hwnd);
                mouse_locked = true;
            }
            me.lmb_down = true;
            me.tick_at_lmb_down = physics_tick_count;
            add_clientplayer_input_event(2, 0, 0); //2=mouse button, 0=left button, 0=press
            break;
        }
        case WM_LBUTTONUP: {
            me.lmb_down = false;
            break;
        }
        case WM_RBUTTONDOWN: {
            if (!mouse_locked) {
                LockCursorToWindow(hwnd);
                mouse_locked = true;
            }
            me.rmb_down = true;
            me.tick_at_rmb_down = physics_tick_count;
            add_clientplayer_input_event(2, 1, 0); //2=mouse button, 1=right button, 0=press
            break;
        }
        case WM_RBUTTONUP: {
            me.rmb_down = false;
            break;
        }
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}

/// Physics structs

typedef struct {
    float x, y, z;
} Vec3;

typedef struct {
    bool intersects;
    bool is_directly_on_top;
} CollisionResult;

/// Collition helpers 
double clamp(double d, double min, double max) {
  const double t = d < min ? min : d;
  return t > max ? max : t;
}
bool boxes_intersect(AABB a, AABB b) {
    return (a.min[0] < b.max[0] && a.max[0] > b.min[0]) &&
           (a.min[1] < b.max[1] && a.max[1] > b.min[1]) &&
           (a.min[2] < b.max[2] && a.max[2] > b.min[2]);
}

bool is_on_top(AABB a, AABB b, float epsilon) {
    bool y_touching = fabsf(a.min[1] - b.max[1]) < epsilon;
    bool xz_overlap = (a.min[0] < b.max[0] && a.max[0] > b.min[0]) && (a.min[2] < b.max[2] && a.max[2] > b.min[2]);
    return y_touching && xz_overlap;
}

AABB translate_box(const AABB* box, Vec3 pos) {
    AABB result;
    result.min[0] = box->min[0] + pos.x;
    result.min[1] = box->min[1] + pos.y;
    result.min[2] = box->min[2] + pos.z;
    result.max[0] = box->max[0] + pos.x;
    result.max[1] = box->max[1] + pos.y;
    result.max[2] = box->max[2] + pos.z;
    return result;
}

CollisionResult check_collision(CollisionShape shape1, Vec3 pos1, bool offset1bypt5, CollisionShape shape2, Vec3 pos2, bool offset2bypt5) {
    CollisionResult result = {false, false};
    const float EPSILON = 0.015f; // Tolerance
    for (int i = 0; i < shape1.num_boxes; i++) {
        AABB box1 = translate_box(&shape1.boxes[i], pos1);
        if (offset1bypt5) {
            box1 = translate_box(&box1, (Vec3){-0.5f, 0.0f, -0.5f});
        }
        for (int j = 0; j < shape2.num_boxes; j++) {
            AABB box2 = translate_box(&shape2.boxes[j], pos2);
            if (offset2bypt5) {
                box2 = translate_box(&box2, (Vec3){-0.5f, 0.0f, -0.5f});
            }
            if (boxes_intersect(box1, box2)) {
                result.intersects = true;
            }
            if (is_on_top(box1, box2, EPSILON)) {
                result.is_directly_on_top = true;
            }
        }
    }
    return result;
}

CollisionShape player_standing = {
    .num_boxes = 1,
    .boxes = {
        {{-0.3f, 0.0f, -0.3f}, {0.3f, 1.8f, 0.3f}},
    }
};

CollisionShape player_crouching = {
    .num_boxes = 1,
    .boxes = {
        {{-0.3f, 0.0f, -0.3f}, {0.3f, 1.5f, 0.3f}},
    }
};

///

CollisionResult check_client_player_block_collision(ClientPlayer client_player, CollisionShape current_shape) {
    CollisionResult result = {false, false};
    for (int x_offset = -1; x_offset <= 1; x_offset++) {
        for (int z_offset = -1; z_offset <= 1; z_offset++) {
            for (int y_offset = -1; y_offset <= 2; y_offset++) {
                Block* block = get_block_xyz(round(client_player.base.base.x) + x_offset, round(client_player.base.base.y) + y_offset, round(client_player.base.base.z) + z_offset);
                if (block && block->id != 0) {
                    CollisionResult res = check_collision(current_shape, (Vec3){client_player.base.base.x, client_player.base.base.y, client_player.base.base.z}, false, collision_shapes[block_defs[block->id][block->meta].collision_shape_id], (Vec3){round(client_player.base.base.x) + x_offset, round(client_player.base.base.y) + y_offset, round(client_player.base.base.z) + z_offset}, false);
                    if (res.intersects) {
                        result.intersects = true;
                    }
                    if (res.is_directly_on_top) {
                        result.is_directly_on_top = true;
                    }
                }
            }
        }
    }
    return result;
}

void check_delete_chunks(){
    /*
    for (int i = 0; i < WORLD_CHUNKS; i++) {
        ChunkData *chunk = &world[i];
        if (chunk->initialized) {
            float dx = chunk->meta.x * 16 + 8 - me.base.base.x;
            float dz = chunk->meta.z * 16 + 8 - me.base.base.z;
            float distance = sqrtf(dx*dx + dz*dz);
            if (distance > ((float)packet_view_distance * 16.0f + 24.0f)) { // 24 blocks for whatever reason
                chunk->initialized = 0;
                chunk->active = 0;
                
                printf("Unloaded chunk at (%d, %d) (player at (%.2f, %.2f))\n", chunk->meta.x, chunk->meta.z, me.base.base.x, me.base.base.z);
            }
        }
    }*/
    //no longer in used in favor of server unloading
}


void mesh_queue(){
    for (int i = 0; i < WORLD_CHUNKS; i++) {
        ChunkData *chunk = &world[i];
        if (chunk->initialized && chunk->active && chunk->meshed == 0) {
            if (chunk->meta.x >= me.base.base.x / 16 - meshing_distance && chunk->meta.x <= me.base.base.x / 16 + meshing_distance && chunk->meta.z >= me.base.base.z / 16 - meshing_distance && chunk->meta.z <= me.base.base.z / 16 + meshing_distance) {
                build_render_flags(chunk, get_chunk(chunk->meta.x, chunk->meta.z - 1), get_chunk(chunk->meta.x, chunk->meta.z + 1), get_chunk(chunk->meta.x + 1, chunk->meta.z), get_chunk(chunk->meta.x - 1, chunk->meta.z));
                chunk->meshed = 1;
                break; //only mesh one chunk per tick to avoid frame drops
            }
        }
    }
    for (int i = 0; i < BLOCK_MESH_QUEUE_SIZE; i++) {
        if (block_mesh_queue[i].active) {
            int chunk_x = block_mesh_queue[i].x >> 4;
            int chunk_z = block_mesh_queue[i].z >> 4;
            ChunkData *chunk = get_chunk(chunk_x, chunk_z);
            if (chunk && chunk->initialized) {
                Block* block = chunk->active ? &chunk->data.blocks[block_mesh_queue[i].y][block_mesh_queue[i].z & 15][block_mesh_queue[i].x & 15] : NULL;
                ChunkData *neighbor_chunk_north = get_chunk(chunk_x, chunk_z - 1);
                ChunkData *neighbor_chunk_south = get_chunk(chunk_x, chunk_z + 1);
                ChunkData *neighbor_chunk_east = get_chunk(chunk_x + 1, chunk_z);
                ChunkData *neighbor_chunk_west = get_chunk(chunk_x - 1, chunk_z);
                if (chunk->meshed) {
                    int neighbor_north_x = block_mesh_queue[i].x;
                    int neighbor_north_y = block_mesh_queue[i].y;
                    int neighbor_north_z = block_mesh_queue[i].z - 1;

                    int neighbor_south_x = block_mesh_queue[i].x;
                    int neighbor_south_y = block_mesh_queue[i].y;
                    int neighbor_south_z = block_mesh_queue[i].z + 1;

                    int neighbor_east_x = block_mesh_queue[i].x + 1;
                    int neighbor_east_y = block_mesh_queue[i].y;
                    int neighbor_east_z = block_mesh_queue[i].z;

                    int neighbor_west_x = block_mesh_queue[i].x - 1;
                    int neighbor_west_y = block_mesh_queue[i].y;
                    int neighbor_west_z = block_mesh_queue[i].z;

                    int neighbor_up_x = block_mesh_queue[i].x;
                    int neighbor_up_y = block_mesh_queue[i].y + 1;
                    int neighbor_up_z = block_mesh_queue[i].z;

                    int neighbor_down_x = block_mesh_queue[i].x;
                    int neighbor_down_y = block_mesh_queue[i].y - 1;
                    int neighbor_down_z = block_mesh_queue[i].z;

                    Block* neighbor_north; // don't use get block xyz because it has to check every chunk
                    if ((neighbor_north_z - chunk_z*16) < 0) {
                        if (neighbor_chunk_north && neighbor_chunk_north->initialized) {
                            neighbor_north = &neighbor_chunk_north->data.blocks[neighbor_north_y][neighbor_north_z & 15][neighbor_north_x & 15];
                        } else {
                            neighbor_north = NULL;
                        }
                    } else {
                        neighbor_north = &chunk->data.blocks[neighbor_north_y][neighbor_north_z & 15][neighbor_north_x & 15];
                    }
                    Block* neighbor_south;
                    if ((neighbor_south_z - chunk_z*16) >= 16) {
                        if (neighbor_chunk_south && neighbor_chunk_south->initialized) {
                            neighbor_south = &neighbor_chunk_south->data.blocks[neighbor_south_y][neighbor_south_z & 15][neighbor_south_x & 15];
                        } else {
                            neighbor_south = NULL;
                        }
                    } else {
                        neighbor_south = &chunk->data.blocks[neighbor_south_y][neighbor_south_z & 15][neighbor_south_x & 15];
                    }
                    Block* neighbor_east;
                    if ((neighbor_east_x - chunk_x*16) >= 16) {
                        if (neighbor_chunk_east && neighbor_chunk_east->initialized) {
                            neighbor_east = &neighbor_chunk_east->data.blocks[neighbor_east_y][neighbor_east_z & 15][neighbor_east_x & 15];
                        } else {
                            neighbor_east = NULL;
                        }
                    } else {
                        neighbor_east = &chunk->data.blocks[neighbor_east_y][neighbor_east_z & 15][neighbor_east_x & 15];
                    }
                    Block* neighbor_west;
                    if ((neighbor_west_x - chunk_x*16) < 0) {
                        if (neighbor_chunk_west && neighbor_chunk_west->initialized) {
                            neighbor_west = &neighbor_chunk_west->data.blocks[neighbor_west_y][neighbor_west_z & 15][neighbor_west_x & 15];
                        } else {
                            neighbor_west = NULL;
                        }
                    } else {
                        neighbor_west = &chunk->data.blocks[neighbor_west_y][neighbor_west_z & 15][neighbor_west_x & 15];
                    }
                    Block* neighbor_up = (neighbor_up_y > 255) ? NULL : &chunk->data.blocks[neighbor_up_y][neighbor_up_z & 15][neighbor_up_x & 15];
                    Block* neighbor_down = (neighbor_down_y < 0) ? NULL : &chunk->data.blocks[neighbor_down_y][neighbor_down_z & 15][neighbor_down_x & 15];

                    block->render_flags = block_render_flags(block, neighbor_north, neighbor_south, neighbor_east, neighbor_west, neighbor_up, neighbor_down);
                    

                    if (neighbor_north){
                        if (!should_render_face(neighbor_north, block)) {
                            //hide
                            neighbor_north->render_flags &= ~RENDER_FACE_SOUTH;
                        } else {
                            //show
                            neighbor_north->render_flags |= RENDER_FACE_SOUTH;
                        }
                    }

                    if (neighbor_south){
                        if (!should_render_face(neighbor_south, block)) {
                            //hide
                            neighbor_south->render_flags &= ~RENDER_FACE_NORTH;
                        } else {
                            //show
                            neighbor_south->render_flags |= RENDER_FACE_NORTH;
                        }
                    }

                    if (neighbor_east){
                        if (!should_render_face(neighbor_east, block)) {
                            //hide
                            neighbor_east->render_flags &= ~RENDER_FACE_WEST;
                        } else {
                            //show
                            neighbor_east->render_flags |= RENDER_FACE_WEST;
                        }

                    }

                    if (neighbor_west){
                        if (!should_render_face(neighbor_west, block)) {
                            //hide
                            neighbor_west->render_flags &= ~RENDER_FACE_EAST;
                        } else {
                            //show
                            neighbor_west->render_flags |= RENDER_FACE_EAST;
                        }
                    }

                    if (neighbor_up){
                        if (!should_render_face(neighbor_up, block)) {
                            //hide
                            neighbor_up->render_flags &= ~RENDER_FACE_BOTTOM;
                        } else {
                            //show
                            neighbor_up->render_flags |= RENDER_FACE_BOTTOM;
                        }
                    }

                    if (neighbor_down){
                        if (!should_render_face(neighbor_down, block)) {
                            //hide
                            neighbor_down->render_flags &= ~RENDER_FACE_TOP;
                        } else {
                            //show
                            neighbor_down->render_flags |= RENDER_FACE_TOP;
                        }
                    }
                }
            }
            block_mesh_queue[i].active = false;
        }
    }
}

void update_cursor_on_block(){
    // raycast from player position in direction of camera until it hits a block or reaches max distance
    float max_distance = 4.5f; // for breaking/placing, not pvp
    Vector3 forward = pitch_yaw_to_forward_degrees(me.base.base.pitch, me.base.base.yaw); //test without me.base.base.pitch for now
    Vector3 ray_pos = {me.base.base.x,me.base.base.y + get_player_camera_yoff(), me.base.base.z}; // eye position
    for (float t = 0; t < max_distance; t += 0.005f) {
        Vector3 check_pos = {ray_pos.x + forward.x * t, ray_pos.y + forward.y * t, ray_pos.z + forward.z * t};
        Block* block = get_block_xyz((int)floorf(check_pos.x), (int)floorf(check_pos.y), (int)floorf(check_pos.z));
        //check if cursor touches block hitbox 
        
        if (block && block->id != 0) {
            /*
            me.cursor_on_block_x = round(check_pos.x);
            me.cursor_on_block_y = round(check_pos.y);
            me.cursor_on_block_z = round(check_pos.z);
            me.is_cursor_on_block = true;
            return;*/
            //use 0.001x0.001x0.001 box at check_pos to check collision with block hitbox, if collides then set cursor on block
            CollisionShape test_shape_0_001 = {
                .num_boxes = 1,
                .boxes = {
                    {{-0.0005f, -0.0005f, -0.0005f}, {0.0005f, 0.0005f, 0.0005f}},
                }
            };
            CollisionResult res = check_collision(test_shape_0_001, (Vec3){check_pos.x, check_pos.y, check_pos.z}, false, get_bounding_box_shape(block->id, block->meta), (Vec3){floorf(check_pos.x), floorf(check_pos.y), floorf(check_pos.z)}, false);
            
            if (res.intersects) {
                me.cursor_on_block_x = (int)floorf(check_pos.x);
                me.cursor_on_block_y = (int)floorf(check_pos.y);
                me.cursor_on_block_z = (int)floorf(check_pos.z);
                me.is_cursor_on_block = true;
                return;
            }
        }
    }
    me.cursor_on_block_x = -1;
    me.cursor_on_block_y = -1;
    me.cursor_on_block_z = -1;
    me.is_cursor_on_block = false;
}


void physics_tick(int tick_count){
    me.w_key_down = GetAsyncKeyState('W') & 0x8000;
    me.a_key_down = GetAsyncKeyState('A') & 0x8000;
    me.s_key_down = GetAsyncKeyState('S') & 0x8000;
    me.d_key_down = GetAsyncKeyState('D') & 0x8000;
    me.space_key_down = GetAsyncKeyState(VK_SPACE) & 0x8000;
    me.shift_key_down = GetAsyncKeyState(VK_SHIFT) & 0x8000;
    me.ctrl_key_down = GetAsyncKeyState(VK_CONTROL) & 0x8000;
    CollisionShape current_shape = me.shift_key_down ? player_crouching : player_standing;
    
    me.base.base.yaw += mouse_dx;
    me.base.base.pitch += mouse_dy;
    
    me.base.base.pitch = (float)clamp((double)me.base.base.pitch, (double)-89.9f, (double)89.9f); 
    me.base.base.yaw = fmodf(me.base.base.yaw, 360.0f);

    float fpm = 1.0f;
    float spm = 1.0f;
    if (me.shift_key_down) {
        fpm *= me.crouch_multiplier;
        spm *= me.crouch_multiplier;
    }
    if (me.ctrl_key_down) {
        fpm *= me.sprint_multiplier;
    }

    if (me.w_key_down){
        Vector3 forward = pitch_yaw_to_forward_degrees_no_y(0, me.base.base.yaw);
        me.x_vel += forward.x * me.base_move_speed*fpm;
        me.z_vel += forward.z * me.base_move_speed*fpm;
    }
    if (me.s_key_down){
        Vector3 forward = pitch_yaw_to_forward_degrees_no_y(0, me.base.base.yaw);
        me.x_vel -= forward.x * me.base_move_speed*spm;
        me.z_vel -= forward.z * me.base_move_speed*spm;
    }
    if (me.a_key_down){
        Vector3 forward = pitch_yaw_to_forward_degrees_no_y(0, me.base.base.yaw);
        me.x_vel += forward.z * me.base_move_speed*spm;
        me.z_vel -= forward.x * me.base_move_speed*spm;
    }
    if (me.d_key_down){
        Vector3 forward = pitch_yaw_to_forward_degrees_no_y(0, me.base.base.yaw);
        me.x_vel -= forward.z * me.base_move_speed*spm;
        me.z_vel += forward.x * me.base_move_speed*spm;
    }

    me.x_vel *= me.friction_multiplier;
    me.z_vel *= me.friction_multiplier;

    if (me.y_vel < -3.92f) {
        me.y_vel = -3.92f; // terminal velocity
    }


    bool is_already_colliding = false;

    CollisionResult res = check_client_player_block_collision(me, current_shape);
    if (res.intersects) {
        is_already_colliding = true;
    }

    if (res.is_directly_on_top && me.space_key_down) {
        me.y_vel = me.jump_vel;
    }

    if (res.is_directly_on_top) {
        me.on_ground = true;
    } else {
        me.on_ground = false;
    }

    if (!is_already_colliding) {
    } else {
        me.x_vel = 0;
        me.z_vel = 0;
        if (me.y_vel < 0) {
            me.y_vel = 0;
        }
    }

    //gravity
    me.y_vel += me.gravity;

    if (1){
        me.base.base.x += me.x_vel;
        // if stuck in block, move back until not colliding
        if (check_client_player_block_collision(me, current_shape).intersects) {
            for (int i = 0; i < ceil(fabs(me.x_vel) * 100); i += 1) {
                me.base.base.x -= (me.x_vel > 0 ? 0.01f : -0.01f);
                if (!check_client_player_block_collision(me, current_shape).intersects) {
                    break;
                }
            }
            me.x_vel = 0;
        }
        me.base.base.z += me.z_vel;
        // if stuck in block, move back until not colliding
        if (check_client_player_block_collision(me, current_shape).intersects) {
            for (int i = 0; i < ceil(fabs(me.z_vel) * 100); i += 1) {
                me.base.base.z -= (me.z_vel > 0 ? 0.01f : -0.01f);
                if (!check_client_player_block_collision(me, current_shape).intersects) {
                    break;
                }
            }
            me.z_vel = 0;
        }
        me.base.base.y += me.y_vel;
        // if stuck in block, move back until not colliding
        if (check_client_player_block_collision(me, current_shape).intersects) {
            for (int i = 0; i < ceil(fabs(me.y_vel) * 100); i += 1) {
                me.base.base.y -= (me.y_vel > 0 ? 0.01f : -0.01f);
                if (!check_client_player_block_collision(me, current_shape).intersects) {
                    break;
                }
            }
            me.y_vel = 0;
        }
    }

    mouse_dx = 0.0f;
    mouse_dy = 0.0f;

    check_delete_chunks();

    update_cursor_on_block();

    for (int i = 0; i < MAX_INPUT_EVENTS; i++) {
        if (me.input_events[i].type != 0) {
            if (me.input_events[i].type == 1) {
                //keyboard event
            } else if (me.input_events[i].type == 2) {
                //mouse button event
                if (me.input_events[i].code == 0) {
                    //left button
                } else {
                    //right button
                    if (me.input_events[i].action == 0) {
                    }
                }
            }
        }
    }
    
}

void interpolation_physics(uint64_t last_tick_us, uint64_t next_tick_us) {
    uint64_t current_time = get_time_us();
    float t = (float)(current_time - last_tick_us) / (float)(next_tick_us - last_tick_us);
    //printf("t: %.2f\n", t);
    if (t > 1.0f) t = 1.0f;
    if (t < 0.0f) t = 0.0f;

    // interpolate me
    me.base.base.interp_x = me.base.base.last_x + (me.base.base.x - me.base.base.last_x) * t;
    me.base.base.interp_y = me.base.base.last_y + (me.base.base.y - me.base.base.last_y) * t;
    me.base.base.interp_z = me.base.base.last_z + (me.base.base.z - me.base.base.last_z) * t;
    me.base.base.interp_yaw = me.base.base.last_yaw + (me.base.base.yaw - me.base.base.last_yaw) * t;
    me.base.base.interp_pitch = me.base.base.last_pitch + (me.base.base.pitch - me.base.base.last_pitch) * t;
    ///
    //printf("%.2f %.2f %.2f\n", me.base.base.interp_yaw, me.base.base.last_yaw, me.base.base.yaw);

}

void send_periodic_packets() {
    if (me.base.base.x != me.last_pk_x || me.base.base.y != me.last_pk_y || me.base.base.z != me.last_pk_z || me.base.base.yaw != me.last_pk_yaw || me.base.base.pitch != me.last_pk_pitch) {
        int final_len = build_player_pos_and_look_packet(send_pk_periodic, me.base.base.x, me.base.base.y, me.base.base.z, me.base.base.yaw, me.base.base.pitch, me.on_ground); // move into movement loop later
        send(server, (char*)send_pk_periodic, final_len, 0);
        me.last_pk_x = me.base.base.x;
        me.last_pk_y = me.base.base.y;
        me.last_pk_z = me.base.base.z;
        me.last_pk_yaw = me.base.base.yaw;
        me.last_pk_pitch = me.base.base.pitch;
    }
}

int main(void) {
    me.inited = false;
    me.base_move_speed = 0.16f;
    me.sprint_multiplier = 1.3f;
    me.crouch_multiplier = 0.3f;
    me.friction_multiplier = 0.6f;
    me.jump_vel = 0.6f;
    me.gravity = -0.08f;

    start_physics_update = get_time_us();

    puts("creating block definitions...");

    createblockdefs();

    puts("created block definitions successfully");

    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = RENDER_W;
    bmi.bmiHeader.biHeight = -RENDER_H; // negative for top-down
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    WNDCLASS wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = "TinyMC_Class";
    RegisterClass(&wc);
    HWND window = CreateWindowEx(0, "TinyMC_Class", "tinymc client", WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT, 640, 480, NULL, NULL, GetModuleHandle(NULL), NULL);
    
    RegisterRawMouse(window);
    LockCursorToWindow(window);

    WSADATA wsa;
	SOCKADDR_IN addr;

    if (WSAStartup(MAKEWORD(2,2), &wsa) != 0) {
        puts("WSAStartup failed");
        return 1;
    }

    server = socket(AF_INET, SOCK_STREAM, 0);

    if (server == INVALID_SOCKET) {
        puts("socket() failed");
        return 1;
    }

	InetPton(AF_INET, ipname, &addr.sin_addr.s_addr);

	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);

    puts("Connecting...");

    if (connect(server, (SOCKADDR*)&addr, sizeof(addr)) == SOCKET_ERROR) {
        printf("connect failed: %d\n", WSAGetLastError());
        return 1;
    }
    
    puts("Connected, sending handshake");
	
    //send handshake
    unsigned char final[512];
    int final_len = build_handshake_packet(final, (char*)ipname, port);
    printf("Build handshake packet with length %d\n", final_len);
    print_contents(final, final_len);
    send(server, (char*)final, final_len, 0);
    puts("Handshake sent successfully");

    //clear final buffer
    memset(final, '\0', sizeof(final));
    current_state = STATE_LOGIN;

    //login start
    final_len = build_login_packet(final, usrname, strlen(usrname));
    printf("Build login packet with length %d\n", final_len);
    print_contents(final, final_len);
    send(server, (char*)final, final_len, 0);
    puts("Login start sent successfully");

    u_long mode = 1; // non blocking mode
    ioctlsocket(server, FIONBIO, &mode);

    uint64_t bench_start, bench_end;

    while (running) {
        //window
        bench_start = get_time_us();
        MSG msg;
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        bench_end = get_time_us();
        double window_elapsed = (bench_end - bench_start);
        //

        //network
        bench_start = get_time_us();
        int n = recv(server, (char*)recv_buf + recv_len, RECV_BUF_SIZE - recv_len, 0);
        if (n > 0) {
            recv_len += n;
            int offset = 0;
            while (1) {
                uint64_t packet_len_64;
                int len_bytes = (int)varint_read_u64(recv_buf + offset, recv_len - offset, &packet_len_64);
                int packet_len = (int)packet_len_64;
                
                if (len_bytes <= 0) break;
                if (recv_len - offset < len_bytes + packet_len) break;

                unsigned char *packet = recv_buf + offset + len_bytes;
                handle_packet(packet, packet_len);

                offset += len_bytes + packet_len;
            }

            if (offset > 0) {
                memmove(recv_buf, recv_buf + offset, recv_len - offset);
                recv_len -= offset;
            }
        } else if (n == SOCKET_ERROR) {
            int err = WSAGetLastError();
            if (err == WSAEWOULDBLOCK) {
                // no data
            } else {
                // error
                printf("Socket Error: %d\n", err);
                running = false;
            }
        } else if (n == 0) {
            puts("Server closed connection");
            running = false;
        }
        bench_end = get_time_us();
        double net_elapsed = (bench_end - bench_start);
        
        bench_start = get_time_us();
        uint64_t current_time = get_time_us();

        int tickstoexecute = (int)(((current_time - start_physics_update) / TICK_TIME) - physics_tick_count); // 0.05 second per tick 

        ChunkData *chunkatplayer = get_chunk((int)(round(me.base.base.x)) >> 4, (int)(round(me.base.base.z)) >> 4);

        if (me.inited && chunkatplayer && chunkatplayer->initialized) {
            if (tickstoexecute > 0) {
                me.base.base.last_x = me.base.base.x;
                me.base.base.last_y = me.base.base.y;
                me.base.base.last_z = me.base.base.z;
                me.base.base.last_yaw = me.base.base.yaw;
                me.base.base.last_pitch = me.base.base.pitch;
            }
            for (int i = 0; i < tickstoexecute; i++) {
                physics_tick(physics_tick_count);
                send_periodic_packets();
                last_physics_update = get_time_us();
                next_physics_update = last_physics_update + TICK_TIME;
                physics_tick_count++;
            }
            interpolation_physics(last_physics_update, next_physics_update);
        } else {
            // interpolation will get messed up and crash 
            start_physics_update = get_time_us();
            last_physics_update = start_physics_update;
            next_physics_update = last_physics_update + TICK_TIME;
        }

        bench_end = get_time_us();
        double physics_elapsed = (bench_end - bench_start);
        bench_start = get_time_us();
        //puts("Meshing chunks...");
        mesh_queue();
        //puts("Finished meshing chunks");
        bench_end = get_time_us();
        double mesh_elapsed = (bench_end - bench_start);
        bench_start = get_time_us();
        render_frame(pxbuf, RENDER_W, RENDER_H);
        bench_end = get_time_us();
        double render_elapsed = (bench_end - bench_start);
        bench_start = get_time_us();
        render_gui_and_inventory(pxbuf, RENDER_W, RENDER_H);
        bench_end = get_time_us();
        double render_2nd_layer_elapsed = (bench_end - bench_start);
        bench_start = get_time_us();
        int width, height;
        GetClientAreaSize(window, &width, &height);
        paint_window(window, pxbuf, width, height);
        bench_end = get_time_us();
        double paint_elapsed = (bench_end - bench_start);

        memset(debug_menu_buf, '\0', sizeof(debug_menu_buf));
        if (display_info_type == 1) {
            snprintf(debug_menu_buf, sizeof(debug_menu_buf), "Player: (%.2f, %.2f, %.2f)\nPlayer Cursor Block: (%i, %i, %i)\nYaw: %.2f Pitch: %.2f\n", me.base.base.interp_x, me.base.base.interp_y, me.base.base.interp_z, me.cursor_on_block_x, me.cursor_on_block_y, me.cursor_on_block_z, me.base.base.interp_yaw, me.base.base.interp_pitch);
        } else if (display_info_type == 3) {
            //packet feed not implemented yet
        } else if (display_info_type == 2) {
            int active_chunks = 0;
            int meshed_chunks = 0;
            for (int i = 0; i < WORLD_CHUNKS; i++) {
                ChunkData *chunk = &world[i];
                if (chunk->active) {
                    if (chunk->initialized) active_chunks++;
                    if (chunk->meshed) meshed_chunks++;
                }
            }
            snprintf(debug_menu_buf, sizeof(debug_menu_buf), "Window: %.2f ms | Network: %.2f ms | Physics: %.2f ms | Mesh Queue: %.2f ms | Render: %.2f ms | 2nd layer Render: %.2f ms | Paint: %.2f ms\n\nTotal chunks: %d, Loaded chunks: %d, Meshed chunks: %d\n", window_elapsed / 1000.0, net_elapsed / 1000.0, physics_elapsed / 1000.0, mesh_elapsed / 1000.0, render_elapsed / 1000.0, render_2nd_layer_elapsed / 1000.0, paint_elapsed / 1000.0, WORLD_CHUNKS, active_chunks, meshed_chunks);
        }
    }

	closesocket(server);
	WSACleanup();

    return 0;
    
}