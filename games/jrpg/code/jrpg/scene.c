#include "scene.h"

static Scene* current_scene = NULL;
static Scene* pending_scene = NULL;
static SceneParams  pending_params;
static uint update_rate = 10;

void scene_process()
{
	gba_vsync();
	// TODO: use interupts for handle vsync and input
	if(pending_scene != NULL)
	{
		if(current_scene && current_scene->exit)
		{
			current_scene->exit();
		}
		current_scene = pending_scene;
		pending_scene = NULL;
		current_scene->enter(pending_params);
	}	

	//gba_wait(update_rate, current_scene->input);
	current_scene->input();
	current_scene->update();
}

void scene_change(Scene* new_scene, SceneParams params)
{
	pending_scene = new_scene;
	pending_params = params;
}

void scene_set_update_rate(uint cycles)
{
	update_rate = cycles;
}
