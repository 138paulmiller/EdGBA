#ifndef SPRITE_H
#define SPRITE_H

#include "gba.h"
#include <assets.h>

typedef int SpriteId;

//Loads sprite image into vram. 
void sprite_sheet_init(SpriteSheet* sheet);

SpriteId sprite_new();

void sprite_get_size(SpriteId spriteid, short *w, short *h);

void sprite_set_pos(SpriteId spriteid, short x, short y);
void sprite_get_pos(SpriteId spriteid, short *x, short *y);
void sprite_move_by(SpriteId spriteid, float dx, float dy);

void sprite_set_frame(SpriteId spriteid, short frame);
void sprite_flip(SpriteId spriteid, char hflip, char vflip);

void sprite_start_anim(SpriteId spriteid, SpriteAnim* anim);
void sprite_stop_anim(SpriteId spriteid);
void sprite_reset_anim(SpriteId spriteid);

void sprite_clear_all();
void sprite_update_all();

#endif
