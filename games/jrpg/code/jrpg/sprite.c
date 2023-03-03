#include "sprite.h"

// TODO: allow for more sprites than objs. Cull sprites into obj according to visibility

typedef struct SpriteInstance 
{
    // sprite object  index
    int object_index;
	
	float x, y; 
    	
	SpriteAnim* anim;

	char animated;	

    short counter;
	short frame;	

} SpriteInstance;

struct SpriteInstance sprites[GBA_OBJ_COUNT];
uint sprite_index = 0;

// current sprite size
uint sprite_size_flags = 0;
uint sprite_priority = GBA_PRIORITY_FIRST; //Before FG and UI

inline SpriteInstance* sprite_get(SpriteId spriteid)
{
	return &sprites[spriteid];
}

void sprite_sheet_init(SpriteSheet* sheet)
{
	sprite_size_flags = sheet->sprite_size;
	gba_obj_image(sheet->pixels, sheet->width, sheet->height);
}

void sprite_start_anim(SpriteId spriteid, SpriteAnim* anim)
{
	SpriteInstance* sprite = sprite_get(spriteid);
	if(sprite->anim != anim)
	{
		sprite_reset_anim(spriteid);
	}
	sprite->anim = anim;
	sprite->animated = 1;
}

void sprite_stop_anim(SpriteId spriteid)
{
	SpriteInstance* sprite = sprite_get(spriteid);
	sprite->animated = 0;
}

void sprite_reset_anim(SpriteId spriteid)
{
	SpriteInstance* sprite = sprite_get(spriteid);
	sprite->counter = 0;  
	sprite->frame = 0;  
}

SpriteId sprite_new()
{
	int obj_index = gba_obj_new(sprite_size_flags, sprite_priority);
	if(obj_index == GBA_OBJ_INVALID)
	{
		return -1;
	}
	
	SpriteId spriteid = sprite_index;
	sprite_index++;

	SpriteInstance* sprite = sprite_get(spriteid);

	sprite->object_index = obj_index;
	sprite->x = sprite->y =0;
	sprite->anim = NULL;

	sprite_reset_anim(spriteid);

	return spriteid;
}

void sprite_get_size(SpriteId spriteid, short *w, short *h)
{
	SpriteInstance* sprite = sprite_get(spriteid);
	*w = gba_obj_width(sprite->object_index);
	*h = gba_obj_height(sprite->object_index);
}

void sprite_set_pos(SpriteId spriteid, short x, short y) 
{
	SpriteInstance* sprite = sprite_get(spriteid);
	gba_obj_set_pos(sprite->object_index, x, y);
}

void sprite_get_pos(SpriteId spriteid, short *x, short *y)
{	
	SpriteInstance* sprite = sprite_get(spriteid);
	gba_obj_get_pos(sprite->object_index, x, y);
}

void sprite_set_frame(SpriteId spriteid, short frame) 
{
	SpriteInstance* sprite = sprite_get(spriteid);


	uint offset = frame * gba_obj_width(sprite->object_index);
	//uint offset = frame * GBA_TILE_WIDTH;
	gba_obj_set_offset(sprite->object_index, offset);
}

void sprite_move_by(SpriteId spriteid, float dx, float dy) 
{
	SpriteInstance* sprite = sprite_get(spriteid);
	sprite->x += dx;
 	sprite->y += dy;
	sprite_set_pos(spriteid, sprite->x, sprite->y);
}

void sprite_flip(SpriteId spriteid, char hflip, char vflip)
{
	SpriteInstance* sprite = sprite_get(spriteid);
	gba_obj_flip(sprite->object_index, hflip, vflip);
}

void sprite_reset_all() 
{
	sprite_index = 0;
	gba_obj_reset_all();
}

void sprite_update(SpriteId spriteid)
{
	SpriteInstance* sprite = sprite_get(spriteid);
	
	SpriteAnim* anim = sprite->anim;
	if(anim == NULL)	
		return;

	if(sprite->animated && anim->frame_count > 1) 
	{
        sprite->counter+=16;
        if (sprite->counter > anim->frame_duration)
        {
           	sprite->frame = (sprite->frame+1) % anim->frame_count;
          	sprite->counter = 0;
        }
    }

	uint frame = 0;
	if(sprite->frame < anim->frame_count)
		frame  = anim->frames[sprite->frame];
    sprite_set_frame(spriteid, frame);
    sprite_flip(spriteid, anim->hflip, anim->vflip);
}

void sprite_update_all()
{
	for(uint id = 0; id < sprite_index; ++id)
	{
		sprite_update(id);
	}
	gba_obj_update_all();
}
