#include "player.h"
#include "sprite.h"

#define DEFAULT_SPEED 0.7
#define INPUT_TIMEOUT 15
#define WALK_CELL_SIZE 16

typedef struct Player
{
	SpriteId sprite;
	PlayerState state;

	float x, y;
	short w, h;
	short col_min_x, col_min_y, col_max_x, col_max_y; //collision rect
	short tx, ty; //target x/y
	short scroll_x, scroll_y;

	float speed;
	short input_timer;
	char target_reached;
} Player;

static Player player;
static SpriteAnim* state_anims[PLAYER_STATE_COUNT];

static short scroll_min_x, scroll_min_y;
static short scroll_max_x, scroll_max_y;
static short walk_min_x, walk_min_y;
static short walk_max_x, walk_max_y;

void player_spawn(short x, short y)
{
	player.sprite = sprite_new();
	player.state = PLAYER_IDLE_DOWN;
	player.speed = DEFAULT_SPEED;
	player.input_timer = INPUT_TIMEOUT;
	player.x = x;
	player.y = y;
	player.tx = x;
	player.ty = y;
	player.target_reached = 1;
	player.input_timer = INPUT_TIMEOUT;

	player.scroll_x = player.scroll_y = 0;
	sprite_get_size(player.sprite, &player.w, &player.h);
	player_init_collision(x,y, player.w, player.h);
}

void player_init_collision(short min_x, short min_y, short max_x, short max_y)
{
	player.col_min_x = min_x;
	player.col_min_y = min_y;
	player.col_max_x = max_x;
	player.col_max_y = max_y;
}

void player_init_walk(short min_x, short min_y, short max_x, short max_y)
{
	scroll_min_x = min_x;
	scroll_min_y = min_y;
	scroll_max_x = max_x - GBA_SCREEN_WIDTH;
	scroll_max_y = max_y - GBA_SCREEN_HEIGHT;

	walk_min_x = min_x;
	walk_min_y = min_y;
	walk_max_x = max_x;
	walk_max_y = max_y;
}

void player_init_anim(PlayerState state, SpriteAnim* anim)
{
	state_anims[state] = anim;
}

short player_x()
{
	return (short)player.x;
}

short player_y()
{
	return (short)player.y;
}

short player_tx()
{
	return (short)player.tx;
}

short player_ty()
{
	return (short)player.ty;
}

short player_scroll_x()
{
	return player.scroll_x;
}

short player_scroll_y()
{
	return player.scroll_y;
}

static void _process_input(PlayerState idle_state, PlayerState walk_state, short dx, short dy)
{
	if(player.state != walk_state && player.state != idle_state)
	{
		player.state = idle_state;
		player.input_timer = INPUT_TIMEOUT;
	}
	else
	{
		player.state = walk_state;
		player.tx = player.x + dx * WALK_CELL_SIZE;
		player.ty = player.y + dy * WALK_CELL_SIZE;
		player.target_reached = 0;
		player.input_timer = INPUT_TIMEOUT;
	}

	// Clamp target to walk rect, taking into account the collision rect
	if(player.tx< walk_min_x - player.col_min_x) 
		player.tx = walk_min_x  - player.col_min_x;	
	else if(player.tx > walk_max_x - player.col_max_x)
		player.tx = walk_max_x  - player.col_max_x;
	if(player.ty < walk_min_y - player.col_min_y) 
		player.ty = walk_min_x - player.col_min_y;			
	else if(player.ty > walk_max_y - player.col_max_y)
		player.ty = walk_max_y - player.col_max_y;
}

void player_input()
{	
	player.input_timer -= 1;
	if(player.input_timer > 0)
		return;
	else 
		player.input_timer -= 1;
	if(!player.target_reached) return;

	if(gba_button_down(GBA_BUTTON_LEFT))
	{	
		_process_input(PLAYER_IDLE_LEFT, PLAYER_WALK_LEFT, -1, 0);
	}
	else if(gba_button_down(GBA_BUTTON_RIGHT))
	{
		_process_input(PLAYER_IDLE_RIGHT, PLAYER_WALK_RIGHT, 1, 0);
	}
	else if(gba_button_down(GBA_BUTTON_UP))
	{
		_process_input(PLAYER_IDLE_UP, PLAYER_WALK_UP, 0, -1);
	}
	else if(gba_button_down(GBA_BUTTON_DOWN))
	{
		_process_input(PLAYER_IDLE_DOWN, PLAYER_WALK_DOWN, 0, 1);
	}		
	else
	{
		switch(player.state)
		{
			case PLAYER_WALK_LEFT:  player.state = PLAYER_IDLE_LEFT;  break;
			case PLAYER_WALK_RIGHT: player.state = PLAYER_IDLE_RIGHT; break;
			case PLAYER_WALK_UP:    player.state = PLAYER_IDLE_UP;    break;
			case PLAYER_WALK_DOWN:  player.state = PLAYER_IDLE_DOWN;  break;
			default:
				break;
		}
	}
}

static void inline _update_movement()
{
	switch(player.state)
	{
		case PLAYER_WALK_LEFT: 	
			player.x -= player.speed;
			if(player.x <= player.tx)
			{
				player.x = player.tx;
				player.target_reached = 1;
			}
			break;
		case PLAYER_WALK_RIGHT: 	
			player.x += player.speed;
			if(player.x >= player.tx)
			{
				player.x = player.tx;
				player.target_reached = 1;
			}
			break;
		case PLAYER_WALK_UP: 		
			player.y -= player.speed;
			if(player.y <= player.ty)
			{
				player.y = player.ty;
				player.target_reached = 1;
			}
			break;
		case PLAYER_WALK_DOWN: 
			player.y += player.speed;
			if((int)player.y >= player.ty)
			{
				player.y = player.ty;
				player.target_reached = 1;
			}
			break;
		default:
			break;
	}
}

static void inline _update_scroll()
{
	player.scroll_x = player.x - GBA_SCREEN_WIDTH / 2;
	player.scroll_y = player.y - GBA_SCREEN_HEIGHT / 2;

	if(player.scroll_x < scroll_min_x)
	{
		player.scroll_x = scroll_min_x;
	}
	else if(player.scroll_x > scroll_max_x)
	{
		player.scroll_x = scroll_max_x;
	}
	if(player.scroll_y < scroll_min_y)
	{
		player.scroll_y = scroll_min_y;
	}
	else if(player.scroll_y > scroll_max_y)
	{
		player.scroll_y = scroll_max_y;
	}
}

static void inline _update_sprite()
{
	int sprite_x = GBA_SCREEN_WIDTH / 2;
	int sprite_y = GBA_SCREEN_HEIGHT / 2;

	const int sprite_offset_x = player.x - GBA_SCREEN_WIDTH / 2;
	const int sprite_offset_y = player.y - GBA_SCREEN_HEIGHT / 2;

	if(player.scroll_x == scroll_min_x)
	{
		sprite_x += sprite_offset_x - scroll_min_x;
	}
	else if(player.scroll_x == scroll_max_x)
	{
		sprite_x += sprite_offset_x - scroll_max_x;
	}
	if(player.scroll_y == scroll_min_y)
	{
		sprite_y += sprite_offset_y - scroll_min_y;
	}
	else if(player.scroll_y == scroll_max_y)
	{
		sprite_y += sprite_offset_y - scroll_max_y;
	}

	sprite_start_anim(player.sprite, state_anims[player.state]); 
	sprite_set_pos(player.sprite, sprite_x, sprite_y);
}

void player_update()
{
	_update_movement();
	_update_scroll();
	_update_sprite();
}