#ifndef SCENE_H
#define SCENE_H

#include "gba.h"
#include <assets.h>

typedef struct SceneParams
{
	Map* map;
	SpriteSheet* spritesheet;
	Palette* map_palette;
	Palette* sprite_palette;
	int portal_x, portal_y; // this should be changed to a portal index
} SceneParams;

typedef void(*SceneEnterFunc)(SceneParams /*params*/);
typedef void(*SceneExitFunc)();
typedef void(*SceneInputFunc)();
typedef void(*SceneUpdateFunc)();
                
typedef struct Scene
{
	SceneEnterFunc enter;
	SceneExitFunc exit;
	SceneInputFunc input;
	SceneUpdateFunc update;
} Scene;

void scene_change(Scene* new_scene, SceneParams params);
void scene_process();

#endif //SCENE_H