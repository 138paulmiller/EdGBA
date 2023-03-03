#ifndef PLAYER_H
#define PLAYER_H

#include "gba.h"
#include <assets.h>

typedef enum PlayerState
{
	PLAYER_IDLE_DOWN = 0, 
	PLAYER_IDLE_UP, 
	PLAYER_IDLE_LEFT, 
	PLAYER_IDLE_RIGHT,
	PLAYER_WALK_DOWN, 
	PLAYER_WALK_UP, 
	PLAYER_WALK_LEFT, 
	PLAYER_WALK_RIGHT,
	PLAYER_STATE_COUNT
} PlayerState;

void player_spawn(short x, short y);
void player_input();
void player_update();

// Setup walk and scroll boundaries
void player_init_walk(short min_x, short min_y, short max_x, short max_y);
void player_init_anim(PlayerState state, SpriteAnim* anim);
void player_init_collision(short min_x, short min_y, short max_x, short max_y);

// Get the players position, in pixels
short player_x();
short player_y();

short player_tx();
short player_ty();

// Get the map scroll offset according to player walk position
short player_scroll_x();
short player_scroll_y();

#endif //PLAYER_H