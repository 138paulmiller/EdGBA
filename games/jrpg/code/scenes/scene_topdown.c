#include "scenes.h"
#include <stdio.h>

#define LAYER_BG 0
#define LAYER_UI 1

void scene_topdown_enter(SceneParams params);
void scene_topdown_exit();
void scene_topdown_input();
void scene_topdown_update();


Scene scene_topdown =
{
	scene_topdown_enter,
	scene_topdown_exit,
	scene_topdown_input,
	scene_topdown_update
};

SceneParams scene_params;
static char debug_log[254] = {0};	
static int zone_swapping = 0;

void scene_topdown_enter(SceneParams params)
{
	scene_params = params;
	gba_bg_palette(params.map_palette->colors);
	gba_obj_palette(params.sprite_palette->colors);

	// Create a boiler player in the scene for loading map, and sprites.
	map_init(params.map);

	if(zone_swapping)
		return;

	// setup sprites
	gba_obj_reset_all();
	sprite_sheet_init(params.spritesheet);


	player_spawn(params.portal_x, params.portal_y);
	player_init_anim(PLAYER_IDLE_DOWN, &Anim_PlayerDown);
	player_init_anim(PLAYER_IDLE_UP, &Anim_PlayerUp);
	player_init_anim(PLAYER_IDLE_LEFT, &Anim_PlayerLeft);
	player_init_anim(PLAYER_IDLE_RIGHT, &Anim_PlayerRight);
	player_init_anim(PLAYER_WALK_DOWN, &Anim_PlayerWalkDown);
	player_init_anim(PLAYER_WALK_UP, &Anim_PlayerWalkUp);
	player_init_anim(PLAYER_WALK_LEFT, &Anim_PlayerWalkLeft);
	player_init_anim(PLAYER_WALK_RIGHT, &Anim_PlayerWalkRight);

	//TODO: define this using a rect obj
	//The sprite does not fill the entire 16x32 rect. This defines the collideable region
	short col_min_x = 0;
	short col_min_y = 0;
	short col_max_x = 16;
	short col_max_y = 24;

	short map_min_x, map_min_y, map_max_x, map_max_y;
	map_rect(&map_min_x, &map_min_y, &map_max_x, &map_max_y);
	player_init_collision(col_min_x, col_min_y, col_max_x, col_max_y);
	player_init_walk(map_min_x, map_min_y, map_max_x, map_max_y);
}

void scene_topdown_exit()
{
}

static void inline update_map()
{
	const short scroll_x = player_scroll_x();
	const short scroll_y = player_scroll_y();

	map_set_scroll(LAYER_BG, scroll_x, scroll_y);

	sprite_update_all();
}

static void update_debug()
{
	char* msg = debug_log;
	msg += sprintf(msg, "xy:%d %d\n", player_x(), player_y());
	//msg += sprintf(msg, "Player State :%d\n", player.state);
	*msg = '\0';

	// clear text
	ui_text("           ", 1,1);
	ui_text(debug_log, 1, 1);
}

void scene_topdown_input()
{	
	player_input();
	
	static char can_swap = 1;
	if(can_swap && gba_button_down(GBA_BUTTON_START))
	{
		static char toggle = 0;
		if(toggle%2 == 1)	
			scene_params.map = &Map_Emerald_0;
		else
			scene_params.map = &Map_Emerald_1;
		++toggle;
		scene_params.portal_x = player_x();
		scene_params.portal_y = player_y();
		scene_change(&scene_topdown, scene_params);
		zone_swapping = 1;
	}
	can_swap = !gba_button_down(GBA_BUTTON_START);
}

void scene_topdown_update()
{
	player_update();

	update_map();
	update_debug();
}