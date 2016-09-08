/*
 *  Woomïnstaller GX2 - Homebrew package installer for Wii U
 *
 *  Copyright (C) 2016          SALT
 *  Copyright (C) 2016          Max Thomas (Shiny Quagsire) <mtinc2@gmail.com>
 *
 *  This code is licensed under the terms of the GNU LGPL, version 2.1
 *  see file LICENSE.md or https://www.gnu.org/licenses/lgpl-2.1.txt
 */

#include "button.h"

bool in_rect(float tpXPos, float tpYPos, float x_pos, float y_pos, float width, float height)
{
    if(tpXPos > x_pos && tpYPos > y_pos && tpXPos < x_pos + width && tpYPos < y_pos + height)
        return true;
    return false;
}

button_t *button_instantiate(float x_pos, float y_pos, float scale_pressed)
{
    volatile button_t *button = memalign(0x100, sizeof(button_t));
    
    button->x_pos = x_pos;
    button->y_pos = y_pos;
    button->width = 0.0f;
    button->height = 0.0f;
    button->scale = 1.0f;
    button->scale_pressed = scale_pressed;
    button->button_mask = 0;
    button->selected = false;
    button->touching = false;
    button->touch_down = false;
    button->inflated = false;
    button->x_center_scale = true;
    button->y_center_scale = true;
    button->touch_y = 0.0f;
    button->touch_x = 0.0f;
    
    button->num_textures = 0;
    button->textures = 0;
    button->num_texts = 0;
    button->texts = 0;
    
    button->num_start_events = 0;
    button->start_events = 0;
    button->num_inflate_events = 0;
    button->inflate_events = 0;
    button->num_inflate_release_events = 0;
    button->inflate_release_events = 0;
    button->num_deflate_events = 0;
    button->deflate_events = 0;
    
    button->extra_data = 0;
    
    return button;
}

void button_add_texture(button_t *button, GX2Texture *texture, float x_shift, float y_shift, float width, float height, bool x_flip, bool y_flip)
{
    button_add_texture_nostretch(button, texture, x_shift, y_shift, width, height, 0.0f, 0.0f, 0.0f, x_flip, y_flip);
}

void button_add_texture_nostretch(button_t *button, GX2Texture *texture, float x_shift, float y_shift, float width, float height, float x_corner_size, float top_y_corner_size, float bottom_y_corner_size, bool x_flip, bool y_flip)
{
    button->textures = realloc(button->textures, ++button->num_textures*sizeof(button_texture*));
    button->textures[button->num_textures-1] = calloc(1, sizeof(button_texture));
    
    button->textures[button->num_textures-1]->texture = texture;
    button->textures[button->num_textures-1]->x_shift = x_shift;
    button->textures[button->num_textures-1]->y_shift = y_shift;
    button->textures[button->num_textures-1]->width = width;
    button->textures[button->num_textures-1]->height = height;
    button->textures[button->num_textures-1]->x_flip = x_flip;
    button->textures[button->num_textures-1]->y_flip = y_flip;
    button->textures[button->num_textures-1]->scale = 1.0f;
    button->textures[button->num_textures-1]->x_corner_size = x_corner_size;
    button->textures[button->num_textures-1]->top_y_corner_size = top_y_corner_size;
    button->textures[button->num_textures-1]->bottom_y_corner_size = bottom_y_corner_size;
    
    if(x_shift + width > button->width) button->width = x_shift + width;
    if(y_shift + height > button->height) button->height = y_shift + height;
}


void button_add_text(button_t *button, float x_shift, float y_shift, int height, int color, char *text)
{
    button->texts = realloc(button->texts, ++button->num_texts*sizeof(button_text*));
    button->texts[button->num_texts-1] = calloc(1, sizeof(button_text));
    
    
    button->texts[button->num_texts-1]->x_shift = x_shift;
    button->texts[button->num_texts-1]->y_shift = y_shift;
    button->texts[button->num_texts-1]->height = height;
    button->texts[button->num_texts-1]->color = color;
    button->texts[button->num_texts-1]->text = malloc(strlen(text) + 1);
    strcpy(button->texts[button->num_texts-1]->text, text);
    
    if(x_shift + (float)font_measure_string_width_height(height, text) > button->width) button->width = x_shift + (float)font_measure_string_width_height(height, text);
    if(y_shift + (float)height > button->height) button->height = y_shift + height;
}

void button_add_start_event(button_t *button, void *callback)
{
    button->start_events = realloc(button->start_events, ++button->num_start_events*sizeof(void*));
    button->start_events[button->num_start_events-1] = callback;
}

void button_add_inflate_event(button_t *button, void *callback)
{
    button->inflate_events = realloc(button->inflate_events, ++button->num_inflate_events*sizeof(void*));
    button->inflate_events[button->num_inflate_events-1] = callback;
}

void button_add_inflate_release_event(button_t *button, void *callback)
{
    button->inflate_release_events = realloc(button->inflate_release_events, ++button->num_inflate_release_events*sizeof(void*));
    button->inflate_release_events[button->num_inflate_release_events-1] = callback;
}

void button_add_deflate_event(button_t *button, void *callback)
{
button->deflate_events = realloc(button->deflate_events, ++button->num_deflate_events*sizeof(void*));
    button->deflate_events[button->num_deflate_events-1] = callback;
}

void button_add_button_map(button_t *button, u32 mask)
{
    button->button_mask |= mask;
}

void button_remove_button_map(button_t *button, u32 mask)
{
    button->button_mask &= (0xFFFFFFFF - mask);
}

void button_update(button_t *button, float tpXPos, float tpYPos, bool tpIsTouched)
{
    //void (*func)() = function;
    
    
    float interpolate_scale = (button->scale_pressed - 1.0f) / 6.0f;
    if((in_rect(tpXPos, tpYPos, button->x_pos, button->y_pos, button->width, button->height) && !button->touching && tpIsTouched) || ((vpad.hold & button->button_mask) && button->selected && !button->touching))
    {
        button->scale = 1.0f;
        button->touching = true;
        button->touch_x = tpXPos;
        button->touch_y = tpYPos;
        button->inflated = false;
        
        for(int i = 0; i < button->num_start_events; i++)
        {
            void (*func)(button_t *button) = button->start_events[i];
            func(button);
        }
    }
    
    if(button->touching)
    {
        if(!button->touch_down)
            button->scale += interpolate_scale;
        else
            button->scale -= interpolate_scale;
            
        if(button->scale > button->scale_pressed)
        {
            button->scale = button->scale_pressed;
            
            if(!button->inflated)
            {
                //call events
                for(int i = 0; i < button->num_inflate_events; i++)
                {
                    void (*func)(button_t *button) = button->inflate_events[i];
                    func(button);
                }
                button->inflated = true;
            }
            
            //Touch is still down, but moved off button. Don't fire events.
            if((!in_rect(tpXPos, tpYPos, button->x_pos, button->y_pos, button->width, button->height) && tpIsTouched) || ((vpad.hold & button->button_mask) && !button->selected))
            {
                button->touch_down = true;
                button->inflated = false;
            }
            else if(!tpIsTouched && !(vpad.hold & button->button_mask)) //Touch was released, fire events.
            {
                for(int i = 0; i < button->num_inflate_release_events; i++)
                {
                    void (*func)(button_t *button) = button->inflate_release_events[i];
                    func(button);
                }
                button->touch_down = true;
                button->inflated = false;
            } 
        }
        else if(button->scale < 1.0f)
        {
            button->scale = 1.0f;
            button->touching = false;
            button->touch_down = false;
            
            for(int i = 0; i < button->num_deflate_events; i++)
            {
                void (*func)(button_t *button) = button->deflate_events[i];
                func(button);
            }
        }
    }
}

void button_draw(button_t *button)
{
    for(int i = 0; i < button->num_textures; i++)
    {
        float width = button->textures[i]->width*button->textures[i]->scale;
        float height = button->textures[i]->height*button->textures[i]->scale;
        float render_width = width*button->scale;
        float render_height = height*button->scale;
        float full_width = button->width*(button->x_center_scale ? button->scale : 1.0f);
        float full_height = button->height*(button->y_center_scale ? button->scale : 1.0f);
    
        render_texture_nostretch(button->textures[i]->texture, button->x_pos + button->textures[i]->x_shift - ((full_width - button->width)/2) + (button->textures[i]->x_flip?render_width:0.0f), button->y_pos + button->textures[i]->y_shift - ((full_height - button->height)/2) + (button->textures[i]->y_flip?render_height:0.0f), button->textures[i]->x_corner_size, button->textures[i]->top_y_corner_size, button->textures[i]->bottom_y_corner_size, render_width*(button->textures[i]->x_flip?-1.0f:1.0f), render_height*(button->textures[i]->y_flip?-1.0f:1.0f));
    }
    
    for(int i = 0; i < button->num_texts; i++)
    {
        float full_width = button->width*button->scale;
        float full_height = button->height*button->scale;
    
        font_set_color(button->texts[i]->color);
        font_draw_string_height(button->x_pos + button->texts[i]->x_shift - ((full_width - button->width)/2), button->y_pos + button->texts[i]->y_shift - ((full_height - button->height)/2), button->texts[i]->height*button->scale, button->texts[i]->text);
    }
}

void button_destroy(button_t *button)
{
    if(button->num_textures)
    {
        for(int i = 0; i < button->num_textures; i++)
        {
            free(button->textures[i]);
        }
        free(button->textures);
    }
    
    if(button->num_texts)
    {
        for(int i = 0; i < button->num_texts; i++)
        {
            free(button->texts[i]->text);
            free(button->texts[i]);
        }
        free(button->texts);
    }
    
    if(button->num_start_events)
        free(button->start_events);
    if(button->num_inflate_events)
        free(button->inflate_events);
    if(button->num_inflate_release_events)
        free(button->inflate_release_events);
    if(button->num_deflate_events)
        free(button->deflate_events);
        
    memset(button, 0, sizeof(button_t));
    
    free(button);
}
