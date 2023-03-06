#include "scene.h"

static Scene* current_scene = NULL;
static Scene* pending_scene = NULL;
static SceneParams  pending_params;

void scene_process()
{
	//gba_vsync();
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

void scene_run(Scene* new_scene, SceneParams params)
{
	current_scene = new_scene;
	current_scene->enter(params);

	//gba_vblank_callback(&scene_process);
	while(1)
	{
		gba_vsync();
		scene_process();
		// TODO: support for additional scene processing here
	}
}

void scene_change(Scene* new_scene, SceneParams params)
{
	pending_scene = new_scene;
	pending_params = params;
}
