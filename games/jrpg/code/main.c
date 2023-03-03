#include <stdio.h>
#include <math.h>

#include <scenes/scenes.h>

int main(int argc, char** argv ) 
{ 
	// TODO: use gbfs to load backgrounds, save data
	SceneParams scene_default_params = 
	{ 
		&Map_Emerald_0,
		&SpriteSheet_Emerald_Brendan,
		&Palette_Tileset,
		&Palette_Sprite,
		128, 128
	};

	scene_change(&SCENE_ENTRY, scene_default_params);
	while(1)
	{
		scene_process();
	}
}
