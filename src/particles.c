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
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "particles.h"
#include "texture.h"
#include "draw.h"
#include "memory.h"

GX2Texture bubbleTex;

typedef struct particle
{
    float x;
    float y;
    float scale;
    float x_vel;
    float y_vel;
    float alpha;
    int size;
    int life;
    int lived;
} particle;

particle particles_top[NUM_PARTICLES];
particle particles_bottom[NUM_PARTICLES];

void spawn_particle(particle *to_spawn)
{
    to_spawn->x = (float)(rand() % TARGET_WIDTH);
    to_spawn->y = (float)(rand() % TARGET_HEIGHT);
        
    if(to_spawn->x >= (float)(TARGET_WIDTH / 2))
        to_spawn->x_vel = 1.0f + (float)(rand() % 6);
    else
        to_spawn->x_vel = -1.0f - (float)(rand() % 6);
            
    if(to_spawn->y >= (float)(TARGET_HEIGHT / 2))
        to_spawn->y_vel = 1.0f + (float)(rand() % 6);
    else
        to_spawn->y_vel = -1.0f - (float)(rand() % 6);
        
    to_spawn->x_vel *= (rand() % 20 == 0) ? -1.0f : 1.0f;
    to_spawn->y_vel *= (rand() % 20 == 0) ? -1.0f : 1.0f;
    to_spawn->alpha = (float)((rand() % 60) + 40) / 100.0f;
       
    to_spawn->scale = 0.0f;
    to_spawn->size = (rand() % 20) + 10;
    to_spawn->life = 240 + (rand() % (60*4));
    to_spawn->lived = 0;
}

void particles_load()
{
    load_img_texture(&bubbleTex, "bubble.png");
    
    for(int i = 0; i < NUM_PARTICLES; i++)
    {
        spawn_particle(&particles_top[i]);
        spawn_particle(&particles_bottom[i]);
    }
}

void particles_draw(bool screen)
{
    for(int i = 0; i < NUM_PARTICLES; i++)
    {
        particle *draw_particle = screen ? &particles_bottom[i] : &particles_top[i];
    
        GX2Texture *tex = &bubbleTex;
        draw_particle->alpha -= 0.005f;
        if(draw_particle->alpha <= 0.0f)
            draw_particle->life = 0;
        
        render_texture_color(tex, draw_particle->x, draw_particle->y, (float)draw_particle->size * draw_particle->scale * ((sin((float)draw_particle->life / 40.0f) + 0.5f) / 2.0f), (float)draw_particle->size * draw_particle->scale * ((sin((float)draw_particle->life / 40.0f) + 0.5f) / 2.0f), 1.0f, 1.0f, 1.0f, draw_particle->alpha);
        
        //Update particle
        draw_particle->x += draw_particle->x_vel;
        draw_particle->y += draw_particle->y_vel;
        draw_particle->life--;
        draw_particle->lived++;
        
        if(draw_particle->lived <= 20)
        {
            draw_particle->scale += 0.05f;
        }
        
        if(draw_particle->life <= 20)
        {
            draw_particle->scale -= 0.05f;
        }
        
        if(draw_particle->life <= 0)
        {
            spawn_particle(draw_particle);
        }
    }
}
