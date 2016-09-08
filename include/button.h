/*
 *  Woomïnstaller GX2 - Homebrew package installer for Wii U
 *
 *  Copyright (C) 2016          SALT
 *  Copyright (C) 2016          Max Thomas (Shiny Quagsire) <mtinc2@gmail.com>
 *
 *  This code is licensed under the terms of the GNU LGPL, version 2.1
 *  see file LICENSE.md or https://www.gnu.org/licenses/lgpl-2.1.txt
 */

#ifndef BUTTON_H
#define BUTTON_H

#include <wut.h>
#include <memory.h>
#include <gx2/texture.h>

#include "draw.h"
#include "font.h"
#include "gui.h"

typedef struct button_texture
{
    GX2Texture *texture;
    float x_shift;
    float y_shift;
    float width;
    float height;
    float x_corner_size;
    float top_y_corner_size;
    float bottom_y_corner_size;
    bool x_flip;
    bool y_flip;
    float scale;
} button_texture;

typedef struct button_text
{
    float x_shift;
    float y_shift;
    int height;
    int color;
    char *text;
} button_text;

typedef struct button_t
{
    float x_pos;
    float y_pos;
    float width;
    float height;
    float scale;
    float scale_pressed;
    int button_mask;
    bool selected;
    bool touching;
    bool touch_down;
    bool inflated;
    bool x_center_scale;
    bool y_center_scale;
    float touch_x;
    float touch_y;
    int num_textures;
    button_texture** textures;
    int num_texts;
    button_text** texts;
    int num_start_events;
    void **start_events;
    int num_inflate_events;
    void **inflate_events;
    int num_inflate_release_events;
    void **inflate_release_events;
    int num_deflate_events;
    void **deflate_events;
    void *extra_data;
    void *extra_data_2;
} button_t;

bool in_rect(float tpXPos, float tpYPos, float x_pos, float y_pos, float width, float height);

button_t *button_instantiate(float x_pos, float y_pos, float scale_pressed);
void button_add_texture(button_t *button, GX2Texture *texture, float x_shift, float y_shift, float width, float height, bool x_flip, bool y_flip);
void button_add_texture_nostretch(button_t *button, GX2Texture *texture, float x_shift, float y_shift, float width, float height, float x_corner_size, float top_y_corner_size, float bottom_y_corner_size, bool x_flip, bool y_flip);
void button_add_text(button_t *button, float x_shift, float y_shift, int height, int color, char *text);
void button_update(button_t *button, float tpXPos, float tpYPos, bool tpIsTouched);
void button_draw(button_t *button);
void button_destroy(button_t *button);

void button_add_start_event(button_t *button, void *callback);
void button_add_inflate_event(button_t *button, void *callback);
void button_add_inflate_release_event(button_t *button, void *callback);
void button_add_deflate_event(button_t *button, void *callback);

void button_add_button_map(button_t *button, u32 mask);
void button_remove_button_map(button_t *button, u32 mask);

#endif
