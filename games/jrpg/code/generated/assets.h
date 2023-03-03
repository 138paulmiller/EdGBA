/*** Generated by EdGBA ***/
#ifndef __ASSETS_H__
#define __ASSETS_H__

typedef struct Map
{
	const char mode : 2;
	const char bg0_priority : 2;
	const char bg0_size_flag : 2;
	const short bg0_scroll_x;
	const short bg0_scroll_y;
	const unsigned short* bg0_tiles;
	const struct Tileset* bg0_tileset;
	const char bg1_priority : 2;
	const char bg1_size_flag : 2;
	const short bg1_scroll_x;
	const short bg1_scroll_y;
	const unsigned short* bg1_tiles;
	const struct Tileset* bg1_tileset;
	const char bg2_priority : 2;
	const char bg2_size_flag : 2;
	const short bg2_scroll_x;
	const short bg2_scroll_y;
	const unsigned short* bg2_tiles;
	const struct Tileset* bg2_tileset;
	const char bg3_priority : 2;
	const char bg3_size_flag : 2;
	const short bg3_scroll_x;
	const short bg3_scroll_y;
	const unsigned short* bg3_tiles;
	const struct Tileset* bg3_tileset;
} Map;

extern struct Map Map_Emerald_0;
extern struct Map Map_Emerald_1;

typedef struct Palette
{
	unsigned short size;
	const unsigned short* colors;
} Palette;

extern struct Palette Palette_Sprite;
extern struct Palette Palette_Tileset;

typedef struct SpriteAnim
{
	unsigned short hflip;
	unsigned short vflip;
	unsigned short frame_duration;
	unsigned char frame_count;
	const unsigned char* frames;
} SpriteAnim;

extern struct SpriteAnim Anim_PlayerDown;
extern struct SpriteAnim Anim_PlayerLeft;
extern struct SpriteAnim Anim_PlayerRight;
extern struct SpriteAnim Anim_PlayerUp;
extern struct SpriteAnim Anim_PlayerWalkDown;
extern struct SpriteAnim Anim_PlayerWalkLeft;
extern struct SpriteAnim Anim_PlayerWalkRight;
extern struct SpriteAnim Anim_PlayerWalkUp;

typedef struct SpriteSheet
{
	unsigned short width;
	unsigned short height;
	const unsigned char* pixels;
	unsigned short sprite_size;
} SpriteSheet;

extern struct SpriteSheet SpriteSheet_Emerald_Brendan;

typedef struct Tileset
{
	unsigned short width;
	unsigned short height;
	const unsigned char* pixels;
} Tileset;

extern struct Tileset Tileset_Emerald_00;
extern struct Tileset Tileset_UI;

#endif //__ASSETS_H__