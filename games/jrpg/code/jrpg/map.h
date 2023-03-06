
#ifndef MAP_H
#define MAP_H

#include "gba.h"
#include <assets.h>

/*
	Stream zones. Must have shared tilesets
*/
//
void map_init(Map* map);

// In pixels, returns to total size of the map taking into account the scroll  
void map_rect(short* min_x, short* min_y, short* max_x, short* max_y);

// Get Layer offsets
void map_scroll_by(short bg_index, short offset_dx, short offset_dy);
void map_set_scroll(short bg_index, short scroll_x, short scroll_y);
void map_get_scroll(short bg_index, short* scroll_x, short* scroll_y);

//Current tiles on screen
uchar map_affine(short bg_index);
ushort map_tile(short bg_index, int x, int y);
volatile ushort* map_tiles(short bg_index, int x, int y);
ushort map_tile_count(short bg_index);
void map_clear(short bg_index);

#endif