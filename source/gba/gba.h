#ifndef GBA_H
#define GBA_H

//TODO: determine max support via hardware. these are approximates

#define GBA_TILED_MODE0 0x00
#define GBA_TILED_MODE1 0x01
#define GBA_TILED_MODE2 0x02
#define GBA_TILED_MODE_COUNT 0x03

// Note reverse order!
#define GBA_PRIORITY_LAST 0
#define GBA_PRIORITY_THIRD 1
#define GBA_PRIORITY_SECOND 2
#define GBA_PRIORITY_FIRST 3
#define GBA_PRIORITY_COUNT 4

// BG indices
#define GBA_BG0 0x00
#define GBA_BG1 0x01
#define GBA_BG2 0x02
#define GBA_BG3 0x03
#define GBA_BG_COUNT 0x04

#define GBA_TILE_SIZE       8
#define GBA_PALETTE_COUNT   256

#define GBA_TILESET_WIDTH  128
#define GBA_TILESET_HEIGHT 256

#define GBA_TILESET_MAX_SIZE (GBA_TILESET_WIDTH * GBA_TILESET_HEIGHT )
#define GBA_TILE_MAX        (GBA_TILESET_MAX_SIZE / (GBA_TILE_SIZE * GBA_TILE_SIZE))

#define GBA_SPRITE_SIZE_8x8   0x00
#define GBA_SPRITE_SIZE_16x16 0x10
#define GBA_SPRITE_SIZE_32x32 0x20
#define GBA_SPRITE_SIZE_64x64 0x30
#define GBA_SPRITE_SIZE_16x8  0x01
#define GBA_SPRITE_SIZE_32x8  0x11
#define GBA_SPRITE_SIZE_32x16 0x21
#define GBA_SPRITE_SIZE_64x32 0x31
#define GBA_SPRITE_SIZE_8x16  0x02
#define GBA_SPRITE_SIZE_8x32  0x12
#define GBA_SPRITE_SIZE_16x32 0x22
#define GBA_SPRITE_SIZE_32x64 0x32
#define GBA_SPRITE_SIZE_COUNT 12

#define GBA_MAP_SIZE_32x32 0
#define GBA_MAP_SIZE_32x64 1
#define GBA_MAP_SIZE_64x32 2
#define GBA_MAP_SIZE_64x64 3
#define GBA_MAP_SIZE_16x16_AFFINE 4
#define GBA_MAP_SIZE_32x32_AFFINE 5
#define GBA_MAP_SIZE_64x64_AFFINE 6
#define GBA_MAP_SIZE_128x128_AFFINE 7
#define GBA_MAP_SIZE_COUNT 8

#define GBA_TILE_HFLIP_BIT 10
#define GBA_TILE_VFLIP_BIT 11

#define GBA_SPRITESHEET_WIDTH  128
#define GBA_SPRITESHEET_HEIGHT 128

// "blank color"
#define GBA_COLORKEY_RGB 0x00FF00FF
#define GBA_COLORKEY_15BIT 0x7C1F

#define GBA_SHARED_SPRITE_PALETTE_NAME "Palette_Sprite"
#define GBA_SHARED_TILESET_PALETTE_NAME "Palette_Tileset"

// Used for local data ids
#define GBA_COLORS_SUFFIX "_colors"
#define GBA_PALETTE_SUFFIX "_palette"
#define GBA_PIXELS_SUFFIX "_pixels"
#define GBA_TILES_SUFFIX "_tiles"

#define GBA_ASSETS_HEADER     "assets.h"

#define GBA_CODE_PATH     "code/"
#define GBA_GENERATED_PATH   "code/generated/"

// All paths under the Asset dir
#define GBA_IMAGES_PATH   "images/"
#define GBA_MAPS_PATH         "maps/"
#define GBA_PALETTES_PATH "palettes/"
#define GBA_TILESETS_PATH     "tilesets/"
#define GBA_SPRITEANIMS_PATH      "spriteanims/"
#define GBA_SPRITESHEETS_PATH "spritesheets/"

#define GBA_PALETTE_TYPE     "Palette"
#define GBA_LAYER_TYPE     "Palette"
#define GBA_MAP_TYPE     "Map"
#define GBA_IMAGE_TYPE   "Image"
#define GBA_TILESET_TYPE "Tileset"
#define GBA_SPRITEANIM_TYPE  "SpriteAnim"
#define GBA_SPRITEFRAME_TYPE  "SpriteFrame"
#define GBA_SPRITESHEET_TYPE "SpriteSheet"

#define GBA_DEFAULT_GAME_NAME    "Game"
#define GBA_DEFAULT_MAP_NAME     "Map"
#define GBA_DEFAULT_PALETTE_NAME "Palette"
#define GBA_DEFAULT_IMAGE_NAME   "Image"
#define GBA_DEFAULT_TILESET_NAME "Tileset"
#define GBA_DEFAULT_SPRITEANIM_NAME  "SpriteAnim"
#define GBA_DEFAULT_SPRITESHEET_NAME "SpriteSheet"

#define EMPTY_TILESET_NAME "[EMPTY]"

#endif // GBA_H
