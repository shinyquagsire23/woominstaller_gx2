/*
 *  Woomïnstaller GX2 - Homebrew package installer for Wii U
 *
 *  Copyright (C) 2016          SALT
 *  Copyright (C) 2016          Max Thomas (Shiny Quagsire) <mtinc2@gmail.com>
 *
 *  This code is licensed under the terms of the GNU LGPL, version 2.1
 *  see file LICENSE.md or https://www.gnu.org/licenses/lgpl-2.1.txt
 */

#include "draw.h"

GX2VertexShader *vertexShader = NULL;
GX2PixelShader *pixelShader = NULL;
GX2FetchShader *fetchShader = NULL;
GX2AttribStream *attributes = NULL;
void *fetchShaderProgramm;
mat4_t projectionMtx;

bool sampler_init = false;
GX2Sampler sampler;

float **tex_allocs = NULL;
int num_allocs = 0;

//Global alpha, scaling for transitions
bool use_globals = true;
bool use_global_shifts = true;
float global_alpha = 1.0f;
float global_scale = 1.0f;
float global_xshift = 0.0f;
float global_yshift = 0.0f;

void clean_tex_allocs()
{
    if(!tex_allocs)
    {
        tex_allocs = calloc(4096, sizeof(float*));
        return;
    }
    
    for(int i = 0; i < num_allocs; i++)
    {
        free(tex_allocs[i]);
    }
    
    num_allocs = 0;
}

void draw_set_global_alpha(float a)
{
    global_alpha = a;
}

void draw_set_global_scale(float scale)
{
    global_scale = scale;
}

void draw_set_global_xshift(float shift)
{
    global_xshift = shift;
}

void draw_set_global_yshift(float shift)
{
    global_yshift = shift;
}

void draw_set_use_globals(bool state)
{
    use_globals = state;
}

void draw_set_use_global_shifts(bool state)
{
    use_global_shifts = state;
}

//TODO: I think this has reached the point where a struct would be justified.
void render_texture_partial_color(GX2Texture *render_texture, float x_pos, float y_pos, float width, float height, int partial_x, int partial_y, int partial_width, int partial_height, float r, float g, float b, float a)
{
    if(use_globals)
    {
        a *= global_alpha;
        x_pos = (x_pos * global_scale) + ((TARGET_WIDTH - (TARGET_WIDTH * global_scale)) / 2) + (use_global_shifts ? global_xshift : 0);
        y_pos = (y_pos * global_scale) + ((TARGET_HEIGHT - (TARGET_HEIGHT * global_scale)) / 2) + (use_global_shifts ? global_yshift : 0);
        width *= global_scale;
        height *= global_scale;
    }

    if(x_pos-width > TARGET_WIDTH || y_pos-height > TARGET_HEIGHT || x_pos < (width < 0 ? width : -width) || y_pos < (height < 0 ? height : -height))
        return;

    if(!sampler_init)
    {
        GX2InitSampler(&sampler, GX2_TEX_CLAMP_MODE_CLAMP , GX2_TEX_XY_FILTER_MODE_LINEAR);
        sampler_init = true;
    }

    int total_width = render_texture->surface.width;
    int total_height = render_texture->surface.height;
    
    float tex_x_min = (float)partial_x / (float)total_width;
    float tex_x_max = (float)(partial_x + partial_width) / (float)total_width;
    float tex_y_min = (float)partial_y / (float)total_height;
    float tex_y_max = (float)(partial_y + partial_height) / (float)total_height;

    GX2SetFetchShader(fetchShader);
    GX2SetVertexShader(vertexShader);
    GX2SetPixelShader(pixelShader);
    
    GX2SetPixelTexture(render_texture, 0);
    GX2SetPixelSampler(&sampler, 0);
    
    //Assumes that the coordinate space is 1920x1080 with 0,0 in the bottom left corner
    float transform_x = ((x_pos / (float)TARGET_WIDTH) * 2.0f)-1.0f;
    float transform_y = ((y_pos / (float)TARGET_HEIGHT) * 2.0f)-1.0f;
    float transform_width = (width / (float)TARGET_WIDTH) * 2.0f;
    float transform_height = (height / (float)TARGET_HEIGHT) * 2.0f;
    
    float g_tex_buffer_data_temp[] =
    {
         tex_x_min, tex_y_max,
         tex_x_min, tex_y_min,
         tex_x_max, tex_y_max,
         
         tex_x_max, tex_y_min,
    };

    float g_vertex_buffer_data_temp[] =
    {
         transform_x, transform_y, 0.0f,
         transform_x, transform_y+transform_height, 0.0f,
         transform_x+transform_width, transform_y, 0.0f,
         
         transform_x+transform_width, transform_y+transform_height, 0.0f,
    };
    
    float g_color_buffer_data_temp[] = 
    {
        r, g, b, a,
        r, g, b, a,
        r, g, b, a,
        r, g, b, a,
    };
    
    float *g_color_buffer_data = malloc(sizeof(g_color_buffer_data_temp));
    float *g_tex_buffer_data = malloc(sizeof(g_tex_buffer_data_temp));
    float *g_vertex_buffer_data = malloc(sizeof(g_vertex_buffer_data_temp));
    memcpy(g_color_buffer_data, g_color_buffer_data_temp, sizeof(g_color_buffer_data_temp));
    memcpy(g_tex_buffer_data, g_tex_buffer_data_temp, sizeof(g_tex_buffer_data_temp));
    memcpy(g_vertex_buffer_data, g_vertex_buffer_data_temp, sizeof(g_vertex_buffer_data_temp));
    
    GX2Invalidate(GX2_INVALIDATE_MODE_CPU | GX2_INVALIDATE_MODE_ATTRIBUTE_BUFFER, g_color_buffer_data, sizeof(g_color_buffer_data_temp));
    GX2Invalidate(GX2_INVALIDATE_MODE_CPU | GX2_INVALIDATE_MODE_ATTRIBUTE_BUFFER, g_tex_buffer_data, sizeof(g_tex_buffer_data_temp));
    GX2Invalidate(GX2_INVALIDATE_MODE_CPU | GX2_INVALIDATE_MODE_ATTRIBUTE_BUFFER, g_vertex_buffer_data, sizeof(g_vertex_buffer_data_temp));
    
    tex_allocs[num_allocs++] = g_tex_buffer_data;
    tex_allocs[num_allocs++] = g_vertex_buffer_data;
    tex_allocs[num_allocs++] = g_color_buffer_data;
    
    unsigned int vtxCount = sizeof(g_vertex_buffer_data_temp) / (sizeof(float) * 3);
    
    GX2SetAttribBuffer(2, sizeof(g_color_buffer_data_temp), sizeof(f32) * 4, g_color_buffer_data);
    GX2SetAttribBuffer(1, sizeof(g_tex_buffer_data_temp), sizeof(f32) * 2, g_tex_buffer_data);
    GX2SetAttribBuffer(0, sizeof(g_vertex_buffer_data_temp), sizeof(f32) * 3, g_vertex_buffer_data);
    GX2SetVertexUniformReg(vertexShader->uniformVars[0].offset, 16, (uint32_t*)projectionMtx);

    GX2DrawEx(GX2_PRIMITIVE_MODE_TRIANGLE_STRIP, vtxCount, 0, 1);
}

void render_texture_partial(GX2Texture *render_texture, float x_pos, float y_pos, float width, float height, int partial_x, int partial_y, int partial_width, int partial_height)
{
    render_texture_partial_color(render_texture, x_pos, y_pos, width, height, partial_x, partial_y, partial_width, partial_height, 1.0f, 1.0f, 1.0f, 1.0f);
}

void render_texture_color(GX2Texture *render_texture, float x_pos, float y_pos, float width, float height, float r, float g, float b, float a)
{
    render_texture_partial_color(render_texture, x_pos, y_pos, width, height, 0, 0, render_texture->surface.width, render_texture->surface.height, r, g, b, a);
}

void render_texture(GX2Texture *render_texture, float x_pos, float y_pos, float width, float height)
{
    render_texture_partial(render_texture, x_pos, y_pos, width, height, 0, 0, render_texture->surface.width, render_texture->surface.height);
}

void render_texture_nostretch_color(GX2Texture *render_texture, float x_pos, float y_pos, float x_corner_size, float top_y_corner_size, float bottom_y_corner_size, float width, float height, float r, float g, float b, float a)
{
    if(x_corner_size == 0.0f && top_y_corner_size == 0.0f && bottom_y_corner_size == 0.0f)
    {
        render_texture_partial_color(render_texture, x_pos, y_pos, width, height, 0, 0, render_texture->surface.width, render_texture->surface.height,r,g,b,a);
        return;
    }

    render_texture_partial_color(render_texture, x_pos+0.0f, y_pos+0.0f, x_corner_size, bottom_y_corner_size, 0, render_texture->surface.height-bottom_y_corner_size, x_corner_size, bottom_y_corner_size,r,g,b,a);
    render_texture_partial_color(render_texture, x_pos+x_corner_size, y_pos+0.0f, width-2.0f*x_corner_size, bottom_y_corner_size, x_corner_size, render_texture->surface.height-bottom_y_corner_size, render_texture->surface.width-2.0f*x_corner_size, bottom_y_corner_size,r,g,b,a);
    render_texture_partial_color(render_texture, x_pos+width-x_corner_size, y_pos+0.0f, x_corner_size, bottom_y_corner_size, render_texture->surface.width-x_corner_size, render_texture->surface.height-bottom_y_corner_size, x_corner_size, bottom_y_corner_size,r,g,b,a);
    
    render_texture_partial_color(render_texture, x_pos+0.0f, y_pos+bottom_y_corner_size, x_corner_size, height-bottom_y_corner_size-top_y_corner_size, 0, top_y_corner_size, x_corner_size, bottom_y_corner_size,r,g,b,a);
    render_texture_partial_color(render_texture, x_pos+x_corner_size, y_pos+bottom_y_corner_size, width-2.0f*x_corner_size, height-bottom_y_corner_size-top_y_corner_size, x_corner_size, top_y_corner_size, render_texture->surface.width-2.0f*x_corner_size, bottom_y_corner_size,r,g,b,a);
    render_texture_partial_color(render_texture, x_pos+width-x_corner_size, y_pos+bottom_y_corner_size, x_corner_size, height-bottom_y_corner_size-top_y_corner_size, render_texture->surface.width-x_corner_size, top_y_corner_size, x_corner_size, bottom_y_corner_size,r,g,b,a);
    
    render_texture_partial_color(render_texture, x_pos+0.0f, y_pos+height-top_y_corner_size, x_corner_size, top_y_corner_size, 0, 0, x_corner_size, top_y_corner_size,r,g,b,a);
    render_texture_partial_color(render_texture, x_pos+x_corner_size, y_pos+height-top_y_corner_size, width-2.0f*x_corner_size, top_y_corner_size, x_corner_size, 0, render_texture->surface.width-2.0f*x_corner_size, top_y_corner_size,r,g,b,a);
    render_texture_partial_color(render_texture, x_pos+width-x_corner_size, y_pos+height-top_y_corner_size, x_corner_size, top_y_corner_size, render_texture->surface.width-x_corner_size, 0, x_corner_size, top_y_corner_size,r,g,b,a);
}

void render_texture_nostretch(GX2Texture *render_texture, float x_pos, float y_pos, float x_corner_size, float top_y_corner_size, float bottom_y_corner_size, float width, float height)
{
    render_texture_nostretch_color(render_texture, x_pos, y_pos, x_corner_size, top_y_corner_size, bottom_y_corner_size, width, height, 1.0f, 1.0f, 1.0f, 1.0f);
}
