/*
 *  Woomïnstaller GX2 - Homebrew package installer for Wii U
 *
 *  Copyright (C) 2016          SALT
 *  Copyright (C) 2016          Max Thomas (Shiny Quagsire) <mtinc2@gmail.com>
 *  Copyright (C) 2016          WulfyStylez
 *
 *  This code is licensed under the terms of the GNU LGPL, version 2.1
 *  see file LICENSE.md or https://www.gnu.org/licenses/lgpl-2.1.txt
 */

#include <wut.h>
#include <coreinit/debug.h>
#include <stdio.h>
#include <stdlib.h>

#include "font.h"
#include "texture.h"
#include "draw.h"
#include "ezxml.h"
#include "memory.h"

GX2Texture fontTexture;

int font_tex_x[0x10000]; 
int font_tex_y[0x10000];
int font_tex_width[0x10000];
int font_tex_height[0x10000];
int font_tex_xoffset[0x10000];
int font_tex_yoffset[0x10000];
int font_tex_xadvance[0x10000];
int font_lineheight;
int font_base;

int current_font_color = 0xFFFFFFFF;

ezxml_t font_xml;

void font_set_color(int color)
{
    current_font_color = color;
}

void font_load_data()
{
    FILE *font_xml_file = fopen("font.fnt", "r");
    
    OSReport("Loading font %s\n", "font.fnt");
    font_xml = ezxml_parse_fp(font_xml_file);
    
    OSReport("Parsing font XML...\n");
    int num_chars = atoi(ezxml_attr(ezxml_get(font_xml, "chars", -1), "count"));
    ezxml_t chars = ezxml_get(font_xml, "chars", 0, "char", -1);
    for(int j = 0; j < num_chars; j++)
    {
        ezxml_t char_entry = ezxml_idx(chars, j);
        if(!char_entry) break;
        
        int id = atoi(ezxml_attr(char_entry, "id"));
        font_tex_x[id] = atoi(ezxml_attr(char_entry, "x"));
        font_tex_y[id] = atoi(ezxml_attr(char_entry, "y"));
        font_tex_width[id] = atoi(ezxml_attr(char_entry, "width"));
        font_tex_height[id] = atoi(ezxml_attr(char_entry, "height"));
        font_tex_xoffset[id] = atoi(ezxml_attr(char_entry, "xoffset"));
        font_tex_yoffset[id] = atoi(ezxml_attr(char_entry, "yoffset"));
        font_tex_xadvance[id] = atoi(ezxml_attr(char_entry, "xadvance"));
    }
    font_lineheight = atoi(ezxml_attr(ezxml_get(font_xml, "common", -1), "lineHeight"));
    font_base = atoi(ezxml_attr(ezxml_get(font_xml, "common", -1), "base"));
   
    OSReport("Loaded font %s\n", ezxml_attr(ezxml_get(font_xml, "info", -1), "face"));
}

void font_init_texture()
{
    //Load bitmap font texture
    load_img_texture(&fontTexture, "font.png");
}

void font_draw_string_height(float x, float y, int height, char* string)
{
    font_draw_string(x, y, (float)height/(float)font_lineheight, string);
}

void font_draw_string(float x, float y, float scale, char* string)
{
    float draw_x = x;
    float draw_y = y;
    
    GX2Texture *renderTexture = &fontTexture;
    float r = (float)((current_font_color & 0xFF0000) >> 16) / 255.0f;
    float g = (float)((current_font_color & 0xFF00) >> 8) / 255.0f;
    float b = (float)(current_font_color & 0xFF) / 255.0f;
    float a = 1.0f;

    for(int i = 0; i < strlen(string); i++)
    {
        u8 character = string[i];
        render_texture_partial_color(renderTexture, draw_x+((float)font_tex_xoffset[character])*scale, draw_y-((float)font_tex_height[character])*scale-((float)font_tex_yoffset[character])*scale+((float)font_base)*scale, ((float)font_tex_width[character])*scale, ((float)font_tex_height[character])*scale, font_tex_x[character], font_tex_y[character], font_tex_width[character], font_tex_height[character], r, g, b, a);
        
        draw_x += ((float)font_tex_xadvance[character])*scale;
    }
}

int font_measure_string_width_height(int height, char *string)
{
    return font_measure_string_width((float)height/(float)font_lineheight, string);
}

int font_measure_string_width(float scale, char *string)
{
    float draw_width = 0;
    for(int i = 0; i < strlen(string); i++)
    {
        u8 character = string[i];
        draw_width += ((float)font_tex_xadvance[character])*scale;
    }
    
    return (int)draw_width;
}

int font_get_height(float scale)
{
    return (int)((float)font_lineheight * scale);
}
