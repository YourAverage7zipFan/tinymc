#include "items_generated.h"

uint8_t stack_size(uint16_t id) {
    switch(id) {
        case 256: // iron_shovel
            return 1;
        case 257: // iron_pickaxe
            return 1;
        case 258: // iron_axe
            return 1;
        case 259: // flint_and_steel
            return 1;
        case 261: // bow
            return 1;
        case 267: // iron_sword
            return 1;
        case 268: // wooden_sword
            return 1;
        case 269: // wooden_shovel
            return 1;
        case 270: // wooden_pickaxe
            return 1;
        case 271: // wooden_axe
            return 1;
        case 272: // stone_sword
            return 1;
        case 273: // stone_shovel
            return 1;
        case 274: // stone_pickaxe
            return 1;
        case 275: // stone_axe
            return 1;
        case 276: // diamond_sword
            return 1;
        case 277: // diamond_shovel
            return 1;
        case 278: // diamond_pickaxe
            return 1;
        case 279: // diamond_axe
            return 1;
        case 282: // mushroom_stew
            return 1;
        case 283: // golden_sword
            return 1;
        case 284: // golden_shovel
            return 1;
        case 285: // golden_pickaxe
            return 1;
        case 286: // golden_axe
            return 1;
        case 290: // wooden_hoe
            return 1;
        case 291: // stone_hoe
            return 1;
        case 292: // iron_hoe
            return 1;
        case 293: // diamond_hoe
            return 1;
        case 294: // golden_hoe
            return 1;
        case 298: // leather_helmet
            return 1;
        case 299: // leather_chestplate
            return 1;
        case 300: // leather_leggings
            return 1;
        case 301: // leather_boots
            return 1;
        case 302: // chainmail_helmet
            return 1;
        case 303: // chainmail_chestplate
            return 1;
        case 304: // chainmail_leggings
            return 1;
        case 305: // chainmail_boots
            return 1;
        case 306: // iron_helmet
            return 1;
        case 307: // iron_chestplate
            return 1;
        case 308: // iron_leggings
            return 1;
        case 309: // iron_boots
            return 1;
        case 310: // diamond_helmet
            return 1;
        case 311: // diamond_chestplate
            return 1;
        case 312: // diamond_leggings
            return 1;
        case 313: // diamond_boots
            return 1;
        case 314: // golden_helmet
            return 1;
        case 315: // golden_chestplate
            return 1;
        case 316: // golden_leggings
            return 1;
        case 317: // golden_boots
            return 1;
        case 323: // sign
            return 16;
        case 325: // bucket
            return 16;
        case 326: // water_bucket
            return 1;
        case 327: // lava_bucket
            return 1;
        case 328: // minecart
            return 1;
        case 329: // saddle
            return 1;
        case 332: // snowball
            return 16;
        case 333: // boat
            return 1;
        case 335: // milk_bucket
            return 1;
        case 342: // chest_minecart
            return 1;
        case 343: // furnace_minecart
            return 1;
        case 344: // egg
            return 16;
        case 346: // fishing_rod
            return 1;
        case 354: // cake
            return 1;
        case 355: // bed
            return 1;
        case 359: // shears
            return 1;
        case 368: // ender_pearl
            return 16;
        case 373: // potion
            return 1;
        case 386: // writable_book
            return 1;
        case 387: // written_book
            return 16;
        case 398: // carrot_on_a_stick
            return 1;
        case 403: // enchanted_book
            return 1;
        case 407: // tnt_minecart
            return 1;
        case 408: // hopper_minecart
            return 1;
        case 413: // rabbit_stew
            return 1;
        case 416: // armor_stand
            return 16;
        case 417: // iron_horse_armor
            return 1;
        case 418: // golden_horse_armor
            return 1;
        case 419: // diamond_horse_armor
            return 1;
        case 422: // command_block_minecart
            return 1;
        case 425: // banner
            return 16;
        case 2256: // record_13
            return 1;
        case 2257: // record_cat
            return 1;
        case 2258: // record_blocks
            return 1;
        case 2259: // record_chirp
            return 1;
        case 2260: // record_far
            return 1;
        case 2261: // record_mall
            return 1;
        case 2262: // record_mellohi
            return 1;
        case 2263: // record_stal
            return 1;
        case 2264: // record_strad
            return 1;
        case 2265: // record_ward
            return 1;
        case 2266: // record_11
            return 1;
        case 2267: // record_wait
            return 1;
        default: return 64;
    }
}

const uint32_t* get_item_texture(uint16_t id) {
    switch(id) {
    case 256: // iron_shovel
        return item_tex_iron_shovel;
    case 257: // iron_pickaxe
        return item_tex_iron_pickaxe;
    case 258: // iron_axe
        return item_tex_iron_axe;
    case 259: // flint_and_steel
        return item_tex_flint_and_steel;
    case 260: // apple
        return item_tex_apple;
    case 261: // bow
        return item_tex_bow_standby;
    case 262: // arrow
        return item_tex_arrow;
    case 263: // coal
        return item_tex_coal;
    case 264: // diamond
        return item_tex_diamond;
    case 265: // iron_ingot
        return item_tex_iron_ingot;
    case 266: // gold_ingot
        return item_tex_gold_ingot;
    case 267: // iron_sword
        return item_tex_iron_sword;
    case 268: // wooden_sword
        return item_tex_wood_sword;
    case 269: // wooden_shovel
        return item_tex_wood_shovel;
    case 270: // wooden_pickaxe
        return item_tex_wood_pickaxe;
    case 271: // wooden_axe
        return item_tex_wood_axe;
    case 272: // stone_sword
        return item_tex_stone_sword;
    case 273: // stone_shovel
        return item_tex_stone_shovel;
    case 274: // stone_pickaxe
        return item_tex_stone_pickaxe;
    case 275: // stone_axe
        return item_tex_stone_axe;
    case 276: // diamond_sword
        return item_tex_diamond_sword;
    case 277: // diamond_shovel
        return item_tex_diamond_shovel;
    case 278: // diamond_pickaxe
        return item_tex_diamond_pickaxe;
    case 279: // diamond_axe
        return item_tex_diamond_axe;
    case 280: // stick
        return item_tex_stick;
    case 281: // bowl
        return item_tex_bowl;
    case 282: // mushroom_stew
        return item_tex_mushroom_stew;
    case 283: // golden_sword
        return item_tex_gold_sword;
    case 284: // golden_shovel
        return item_tex_gold_shovel;
    case 285: // golden_pickaxe
        return item_tex_gold_pickaxe;
    case 286: // golden_axe
        return item_tex_gold_axe;
    case 287: // string
        return item_tex_string;
    case 288: // feather
        return item_tex_feather;
    case 289: // gunpowder
        return item_tex_gunpowder;
    case 290: // wooden_hoe
        return item_tex_wood_hoe;
    case 291: // stone_hoe
        return item_tex_stone_hoe;
    case 292: // iron_hoe
        return item_tex_iron_hoe;
    case 293: // diamond_hoe
        return item_tex_diamond_hoe;
    case 294: // golden_hoe
        return item_tex_gold_hoe;
    case 295: // wheat_seeds
        return item_tex_seeds_wheat;
    case 296: // wheat
        return item_tex_wheat;
    case 297: // bread
        return item_tex_bread;
    case 298: // leather_helmet
        return item_tex_leather_helmet;
    case 299: // leather_chestplate
        return item_tex_leather_chestplate;
    case 300: // leather_leggings
        return item_tex_leather_leggings;
    case 301: // leather_boots
        return item_tex_leather_boots;
    case 302: // chainmail_helmet
        return item_tex_chainmail_helmet;
    case 303: // chainmail_chestplate
        return item_tex_chainmail_chestplate;
    case 304: // chainmail_leggings
        return item_tex_chainmail_leggings;
    case 305: // chainmail_boots
        return item_tex_chainmail_boots;
    case 306: // iron_helmet
        return item_tex_iron_helmet;
    case 307: // iron_chestplate
        return item_tex_iron_chestplate;
    case 308: // iron_leggings
        return item_tex_iron_leggings;
    case 309: // iron_boots
        return item_tex_iron_boots;
    case 310: // diamond_helmet
        return item_tex_diamond_helmet;
    case 311: // diamond_chestplate
        return item_tex_diamond_chestplate;
    case 312: // diamond_leggings
        return item_tex_diamond_leggings;
    case 313: // diamond_boots
        return item_tex_diamond_boots;
    case 314: // golden_helmet
        return item_tex_gold_helmet;
    case 315: // golden_chestplate
        return item_tex_gold_chestplate;
    case 316: // golden_leggings
        return item_tex_gold_leggings;
    case 317: // golden_boots
        return item_tex_gold_boots;
    case 318: // flint
        return item_tex_flint;
    case 319: // porkchop
        return item_tex_porkchop_raw;
    case 320: // cooked_porkchop
        return item_tex_porkchop_cooked;
    case 321: // painting
        return item_tex_painting;
    case 322: // golden_apple
        return item_tex_apple_golden;
    case 323: // sign
        return item_tex_sign;
    case 324: // wooden_door
        return item_tex_door_wood;
    case 325: // bucket
        return item_tex_bucket_empty;
    case 326: // water_bucket
        return item_tex_bucket_water;
    case 327: // lava_bucket
        return item_tex_bucket_lava;
    case 328: // minecart
        return item_tex_minecart_normal;
    case 329: // saddle
        return item_tex_saddle;
    case 330: // iron_door
        return item_tex_door_iron;
    case 331: // redstone
        return item_tex_redstone_dust;
    case 332: // snowball
        return item_tex_snowball;
    case 333: // boat
        return item_tex_boat;
    case 334: // leather
        return item_tex_leather;
    case 335: // milk_bucket
        return item_tex_bucket_milk;
    case 336: // brick
        return item_tex_brick;
    case 337: // clay_ball
        return item_tex_clay_ball;
    case 338: // reeds
        return item_tex_reeds;
    case 339: // paper
        return item_tex_paper;
    case 340: // book
        return item_tex_book_normal;
    case 341: // slime_ball
        return item_tex_slimeball;
    case 342: // chest_minecart
        return item_tex_minecart_chest;
    case 343: // furnace_minecart
        return item_tex_minecart_furnace;
    case 344: // egg
        return item_tex_egg;
    case 345: // compass
        return item_tex_compass;
    case 346: // fishing_rod
        return item_tex_fishing_rod_uncast;
    case 347: // clock
        return item_tex_clock;
    case 348: // glowstone_dust
        return item_tex_glowstone_dust;
    case 349: // fish
        return item_tex_fish_raw;
    case 350: // cooked_fish
        return item_tex_fish_salmon_cooked;
    case 351: // dye
        return item_tex_dye_powder_black;
    case 352: // bone
        return item_tex_bone;
    case 353: // sugar
        return item_tex_sugar;
    case 354: // cake
        return item_tex_cake;
    case 355: // bed
        return item_tex_bed;
    case 356: // repeater
        return item_tex_repeater;
    case 357: // cookie
        return item_tex_cookie;
    case 358: // filled_map
        return item_tex_map_filled;
    case 359: // shears
        return item_tex_shears;
    case 360: // melon
        return item_tex_melon;
    case 361: // pumpkin_seeds
        return item_tex_seeds_pumpkin;
    case 362: // melon_seeds
        return item_tex_seeds_melon;
    case 363: // beef
        return item_tex_beef_raw;
    case 364: // cooked_beef
        return item_tex_beef_cooked;
    case 365: // chicken
        return item_tex_chicken_raw;
    case 366: // cooked_chicken
        return item_tex_chicken_cooked;
    case 367: // rotten_flesh
        return item_tex_rotten_flesh;
    case 368: // ender_pearl
        return item_tex_ender_pearl;
    case 369: // blaze_rod
        return item_tex_blaze_rod;
    case 370: // ghast_tear
        return item_tex_ghast_tear;
    case 371: // gold_nugget
        return item_tex_gold_nugget;
    case 372: // nether_wart
        return item_tex_nether_wart;
    case 373: // potion
        return item_tex_potion_overlay;
    case 374: // glass_bottle
        return item_tex_potion_bottle_empty;
    case 375: // spider_eye
        return item_tex_spider_eye;
    case 376: // fermented_spider_eye
        return item_tex_spider_eye_fermented;
    case 377: // blaze_powder
        return item_tex_blaze_powder;
    case 378: // magma_cream
        return item_tex_magma_cream;
    case 379: // brewing_stand
        return item_tex_brewing_stand;
    case 380: // cauldron
        return item_tex_cauldron;
    case 381: // ender_eye
        return item_tex_ender_eye;
    case 382: // speckled_melon
        return item_tex_melon_speckled;
    case 383: // spawn_egg
        return item_tex_spawn_egg;
    case 384: // experience_bottle
        return item_tex_experience_bottle;
    case 385: // fire_charge
        return item_tex_fireball;
    case 386: // writable_book
        return item_tex_book_writable;
    case 387: // written_book
        return item_tex_book_written;
    case 388: // emerald
        return item_tex_emerald;
    case 389: // item_frame
        return item_tex_item_frame;
    case 390: // flower_pot
        return item_tex_flower_pot;
    case 391: // carrot
        return item_tex_carrot;
    case 392: // potato
        return item_tex_potato;
    case 393: // baked_potato
        return item_tex_potato_baked;
    case 394: // poisonous_potato
        return item_tex_potato_poisonous;
    case 395: // map
        return item_tex_map_empty;
    case 396: // golden_carrot
        return item_tex_carrot_golden;
    case 398: // carrot_on_a_stick
        return item_tex_carrot_on_a_stick;
    case 399: // nether_star
        return item_tex_nether_star;
    case 400: // pumpkin_pie
        return item_tex_pumpkin_pie;
    case 401: // fireworks
        return item_tex_fireworks;
    case 402: // firework_charge
        return item_tex_fireworks_charge;
    case 403: // enchanted_book
        return item_tex_book_enchanted;
    case 404: // comparator
        return item_tex_comparator;
    case 405: // netherbrick
        return item_tex_netherbrick;
    case 406: // quartz
        return item_tex_quartz;
    case 407: // tnt_minecart
        return item_tex_minecart_tnt;
    case 408: // hopper_minecart
        return item_tex_minecart_hopper;
    case 409: // prismarine_shard
        return item_tex_prismarine_shard;
    case 410: // prismarine_crystals
        return item_tex_prismarine_crystals;
    case 411: // rabbit
        return item_tex_rabbit_raw;
    case 412: // cooked_rabbit
        return item_tex_rabbit_cooked;
    case 413: // rabbit_stew
        return item_tex_rabbit_stew;
    case 414: // rabbit_foot
        return item_tex_rabbit_foot;
    case 415: // rabbit_hide
        return item_tex_rabbit_hide;
    case 416: // armor_stand
        return item_tex_wooden_armorstand;
    case 417: // iron_horse_armor
        return item_tex_iron_horse_armor;
    case 418: // golden_horse_armor
        return item_tex_gold_horse_armor;
    case 419: // diamond_horse_armor
        return item_tex_diamond_horse_armor;
    case 420: // lead
        return item_tex_lead;
    case 421: // name_tag
        return item_tex_name_tag;
    case 422: // command_block_minecart
        return item_tex_minecart_command_block;
    case 423: // mutton
        return item_tex_mutton_raw;
    case 424: // cooked_mutton
        return item_tex_mutton_cooked;
    case 427: // spruce_door
        return item_tex_door_spruce;
    case 428: // birch_door
        return item_tex_door_birch;
    case 429: // jungle_door
        return item_tex_door_jungle;
    case 430: // acacia_door
        return item_tex_door_acacia;
    case 431: // dark_oak_door
        return item_tex_door_dark_oak;
    case 2256: // record_13
        return item_tex_record_13;
    case 2257: // record_cat
        return item_tex_record_cat;
    case 2258: // record_blocks
        return item_tex_record_blocks;
    case 2259: // record_chirp
        return item_tex_record_chirp;
    case 2260: // record_far
        return item_tex_record_far;
    case 2261: // record_mall
        return item_tex_record_mall;
    case 2262: // record_mellohi
        return item_tex_record_mellohi;
    case 2263: // record_stal
        return item_tex_record_stal;
    case 2264: // record_strad
        return item_tex_record_strad;
    case 2265: // record_ward
        return item_tex_record_ward;
    case 2266: // record_11
        return item_tex_record_11;
    case 2267: // record_wait
        return item_tex_record_wait;
    default: return NULL;
    }
}