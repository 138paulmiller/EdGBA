#include "ui.h"
#include "map.h"

/*
	Draw text on the background at (x, y) with tilemap width w. 
	This is done by first loading text image as a bakground image.
	Each character is a tile that is mapped to the screen.
	Function works similar to the tilemap load function.
*/

#define UI_BG_INDEX 1

inline void ui_text(char* str, short x, short y) 
{
	//assuming text image is in ASCII order and ignores control and escape codes 0-32
	int ignore = 32; 

	volatile ushort* tiles = map_tiles(UI_BG_INDEX, x, y);

	//iterate through each char and map ASCII to image index
	while (*str != '\0') 
	{
		if(*str == '\n')
		{
			++y;
			++str;

			tiles = map_tiles(UI_BG_INDEX, x, y);
			continue;	
		}
		//set char as index to tilemap
		*tiles = *str-ignore;
		++str;
		++tiles;
	}   
}

void ui_clear()
{
	map_clear(UI_BG_INDEX);
}