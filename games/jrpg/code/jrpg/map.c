#include "map.h"
#include <math.h>
typedef struct BgState
{
	char char_block;
	char screen_block;
	const unsigned char* image_data;
	const unsigned short* tile_data;
} BgState;

BgState _bg_states[GBA_BG_COUNT];
Map* _map = NULL;
char _mode = -1;

inline ushort _get_affine(ushort mode, ushort bg_index)
{
	switch(mode)
	{
	case 1: return bg_index == 2;
	case 2: return bg_index == 2 ||  bg_index == 3;
	}
	return 0;
}

inline ushort _get_size_flag_width(ushort mode, ushort bg_index, ushort size_flag)
{
	if(_get_affine(mode, bg_index))
	{
		switch(size_flag)
		{
			case GBA_BG_16x16_AFFINE: return 16;
			case GBA_BG_32x32_AFFINE: return 32;
			case GBA_BG_64x64_AFFINE: return 64;
			case GBA_BG_128x128_AFFINE: return 128;
		}
	}
	else
	{	
		switch(size_flag)
		{
			case GBA_BG_32x32: return 32;
			case GBA_BG_64x32: return 64;
			case GBA_BG_32x64: return 32;
			case GBA_BG_64x64: return 64;
		}
	}
	return 0;
}

inline ushort _get_size_flag_height(ushort mode, ushort bg_index, ushort size_flag)
{
	if(_get_affine(mode, bg_index))
	{
		switch(size_flag)
		{
			case GBA_BG_16x16_AFFINE: return 16;
			case GBA_BG_32x32_AFFINE: return 32;
			case GBA_BG_64x64_AFFINE: return 64;
			case GBA_BG_128x128_AFFINE: return 128;
		}
	}
	else
	{	
		switch(size_flag)
		{
			case GBA_BG_32x32: return 32;
			case GBA_BG_64x32: return 32;
			case GBA_BG_32x64: return 64;
			case GBA_BG_64x64: return 64;
		}
	}
	return 0;
}

inline void _set_screenblock(short bg_index, char screen_block_n)
{
	_bg_states[bg_index].screen_block = screen_block_n;
}

inline uchar _get_screenblock(short bg_index)
{
	return _bg_states[bg_index].screen_block;
}

inline void _set_charblock(short bg_index, char char_block_n)
{	
	_bg_states[bg_index].char_block = char_block_n;
}

inline char _get_charblock(short bg_index)
{
	return _bg_states[bg_index].char_block;
}

inline const uchar* _get_image_data(short bg_index)
{
	return _bg_states[bg_index].image_data;
}

inline void _set_image_data(short bg_index, const uchar* image_data)
{
	_bg_states[bg_index].image_data = image_data;
}

inline const ushort* _get_tile_data(short bg_index)
{
	return _bg_states[bg_index].tile_data;
}

inline void _set_tile_data(short bg_index, const ushort* tile_data)
{
	_bg_states[bg_index].tile_data = tile_data;
}

inline char _get_priority(short bg_index)
{
	switch(bg_index)
	{
		case 0: return _map->bg0_priority;
		case 1: return _map->bg1_priority;
		case 2: return _map->bg2_priority;
		case 3: return _map->bg3_priority;
	}
	return GBA_PRIORITY_FIRST;
}

inline short _get_scroll_x(short bg_index)
{
	switch(bg_index)
	{
		case 0: return _map->bg0_scroll_x;
		case 1: return _map->bg1_scroll_x;
		case 2: return _map->bg2_scroll_x;
		case 3: return _map->bg3_scroll_x;
	}
	return 0;
}

inline short _get_scroll_y(short bg_index)
{
	switch(bg_index)
	{
		case 0: return _map->bg0_scroll_y;
		case 1: return _map->bg1_scroll_y;
		case 2: return _map->bg2_scroll_y;
		case 3: return _map->bg3_scroll_y;
	}
	return 0;
}

inline const Tileset* _get_tileset(short bg_index)
{
	switch(bg_index)
	{
		case 0: return _map->bg0_tileset;
		case 1: return _map->bg1_tileset;
		case 2: return _map->bg2_tileset;
		case 3: return _map->bg3_tileset;
	}
	return NULL;
}

inline const ushort* _get_tiles(short bg_index)
{
	switch(bg_index)
	{
		case 0: return _map->bg0_tiles;
		case 1: return _map->bg1_tiles;
		case 2: return _map->bg2_tiles;
		case 3: return _map->bg3_tiles;
	}
	return NULL;
}

inline ushort _get_size_flag(short bg_index)
{
	switch(bg_index)
	{
		case 0: return _map->bg0_size_flag;
		case 1: return _map->bg1_size_flag;
		case 2: return _map->bg2_size_flag;
		case 3: return _map->bg3_size_flag;
	}
	return 0;
}

static inline ushort _get_width(short bg_index)
{
	return _get_size_flag_width(_mode, bg_index, _get_size_flag(bg_index));
}

static inline ushort _get_height(short bg_index)
{
	return _get_size_flag_height(_mode, bg_index, _get_size_flag(bg_index));
}

inline char _is_enabled(ushort bg_index)
{
	return _get_tileset(bg_index) && _get_tiles(bg_index);
}

void _setup_bgs()
{
	const Tileset* bg_to_tileset[GBA_BG_COUNT];
	for(short bg_index = 0; bg_index < GBA_BG_COUNT; ++bg_index)
	{
		bg_to_tileset[bg_index] = NULL;
	}
	
	//Setup layer charblocks. One per tileset
	int charblock_n = 0;
	for(short bg_index = 0; bg_index < GBA_BG_COUNT; ++bg_index)
	{
		if(!_is_enabled(bg_index))
			continue;

		const Tileset* tileset = _get_tileset(bg_index);

		short bg_charblock_n = charblock_n; 
		for(short other_bg_index = 0; other_bg_index< GBA_BG_COUNT; ++other_bg_index)
		{
			if(bg_to_tileset[other_bg_index] == tileset)
			{
				bg_charblock_n = _get_charblock(other_bg_index);
				break;
			}
		}
		
		_set_charblock(bg_index, bg_charblock_n);

		bg_to_tileset[bg_index] = tileset;
		if(bg_charblock_n == charblock_n)
		{
			charblock_n += ceil(((float)tileset->width * tileset->height) / (int)GBA_CHAR_BLOCK_SIZE);
		}
	}

	// Setup layer screen blocks
	int screen_block_n = charblock_n * (GBA_SCREEN_BLOCK_COUNT / GBA_CHAR_BLOCK_COUNT);
	for(short bg_index = 0; bg_index < GBA_BG_COUNT; ++bg_index)
	{
		if(!_is_enabled(bg_index))
			continue;

		_set_screenblock(bg_index, screen_block_n);
		// Multiply by 2, since the tile data is a short
		screen_block_n += 2 * ceil((float)(_get_width(bg_index) * _get_height(bg_index)) / (int)GBA_SCREEN_BLOCK_SIZE);
	}
}

void map_init(Map* map)
{
	_map = map;

	// setup the gba bg
	_setup_bgs(map);

	//Only reinitialize the GBA mode changes. TODO: 
	if(_mode != _map->mode)
	{
		const char sprite_2d = 0;
		_mode = _map->mode;
		gba_init(_map->mode, sprite_2d);
	}

	for(short bg_index = 0; bg_index < GBA_BG_COUNT; ++bg_index)
	{
		//if layer is not valid, do not load the data 
		if(!_is_enabled(bg_index))
		{
			gba_bg_disable(bg_index);
			continue;
		}
		ushort width = _get_width(bg_index);
		ushort height = _get_height(bg_index);
		short scroll_x = _get_scroll_x(bg_index);
		short scroll_y = _get_scroll_y(bg_index);

		const uchar wrap = 0;  //only available if affine
		const uchar priority = _get_priority(bg_index);
		const uchar char_block_n = _get_charblock(bg_index);
		const uchar screen_block_n = _get_screenblock(bg_index);
		const uchar size_flags = _get_size_flag(bg_index);

		gba_bg_enable(bg_index, char_block_n, screen_block_n, size_flags, priority, wrap);
		
		// Load the BG tileset, if not already loaded
		const Tileset* tileset = _get_tileset(bg_index);
		if(tileset->pixels != _get_image_data(bg_index))
			gba_bg_image(char_block_n, tileset->pixels, tileset->width, tileset->height);	   
		
		// Load the tilemap, if not already loaded
		const ushort* tiles = _get_tiles(bg_index);
		if(tiles != _get_tile_data(bg_index))
			gba_bg_tilemap(screen_block_n, tiles, width, height);
	
		// Adjust to default scroll position
		gba_bg_set_scroll(bg_index, scroll_x, scroll_y);

		//cache current loaded state
		_set_image_data(bg_index, tileset->pixels);
		_set_tile_data(bg_index, tiles);
	}
}

void map_rect(short* min_x, short* min_y, short* max_x, short* max_y)
{
	*min_x = 32767;
	*min_y = 32767;
	*max_x = -32768;
	*max_y = -32768;

	for(ushort bg_index = 0; bg_index < GBA_BG_COUNT; ++bg_index)
	{	
		if(!_is_enabled(bg_index))
			continue;
		
		short bg_min_x, bg_min_y, bg_max_x, bg_max_y;
		
		const ushort width = _get_width(bg_index);
		const ushort height = _get_height(bg_index);
		const short scroll_x = _get_scroll_x(bg_index);
		const short scroll_y = _get_scroll_y(bg_index);

		bg_min_x = scroll_x;
		bg_min_y = scroll_y;
		bg_max_x = scroll_x + width * GBA_TILE_WIDTH;
		bg_max_y = scroll_y + height * GBA_TILE_HEIGHT;

		if(bg_min_x < *min_x)
			*min_x = bg_min_x;

		if(bg_min_y < *min_y)
			*min_y = bg_min_y;

		if(bg_max_x > *max_x)
			*max_x = bg_max_x;

		if(bg_max_y > *max_y)
			*max_y = bg_max_y;
	}
}

void map_scroll_by(short bg_index, short offset_dx, short offset_dy)
{	
	if(!_is_enabled(bg_index))
		return;

	gba_bg_scroll_by(bg_index, offset_dx, offset_dy);
}

void map_set_scroll(short bg_index, short scroll_x, short scroll_y)
{
	if(!_is_enabled(bg_index))
		return;

	gba_bg_set_scroll(bg_index, scroll_x, scroll_y);
}

void map_get_scroll(short bg_index, short* scroll_x, short* scroll_y)
{
	if(!_is_enabled(bg_index))
		return;

	gba_bg_get_scroll(bg_index, scroll_x, scroll_y);
}

ushort map_tile(short bg_index, int x, int y) 
{
	if(!_is_enabled(bg_index))
		return -1;

	const ushort* tiles = _get_tiles(bg_index);
	if(tiles == NULL)
		return -1;

	const ushort width = _get_width(bg_index);
	const short scroll_x = _get_scroll_x(bg_index);
	const short scroll_y = _get_scroll_y(bg_index);

	//offset by scroll
	x += scroll_x;
	y += scroll_y;

	// convert from screen coord to tile map index coord 
	// each tile 8x8 so divide screen by 8 to get 1x1 index
	x >>= 3;
	y >>= 3;

	// return the tile at the 2d->1d transformed index
	return tiles[y * width + x];
}

volatile ushort* map_tiles(short bg_index, int x, int y)
{
	if(!_is_enabled(bg_index))
		return NULL;

	const ushort width = _get_width(bg_index);
	if(width == 0)
		return NULL;

	char screen_block_n = _get_screenblock(bg_index);
   	volatile ushort* screen_block = gba_screen_block(screen_block_n);

	const int index = y * width + x;
	return &screen_block[index];
}

ushort map_tile_count(short bg_index)
{
	return _get_width(bg_index) * _get_height(bg_index);
}

void map_clear(short bg_index)
{
	if(!_is_enabled(bg_index))
		return;

	/* clear the map to be all blank tiles */
	char screen_block_n = _get_screenblock(bg_index);
   	volatile ushort* screen_block = gba_screen_block(screen_block_n);
	for (int i = 0; i < map_tile_count(bg_index); i++) 
	{
		screen_block[i] = 0;
	}
}
