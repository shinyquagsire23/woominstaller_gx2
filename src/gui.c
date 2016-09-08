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

#include "gui.h"

//Common GUI Textures
GX2Texture texture;
GX2Texture textureBackdrop;
GX2Texture textureHeader;
GX2Texture textureInfoHeader;
GX2Texture textureContent;
GX2Texture textureContentSubheader;
GX2Texture textureBullet;
GX2Texture textureFolder;
GX2Texture textureFSTFolder;
GX2Texture textureYButton;
GX2Texture textureIconBack;
GX2Texture textureDRCArrow;
GX2Texture textureDRCButton;
GX2Texture textureDRCInstall;
GX2Texture textureDRCShadow;
GX2Texture textureDRCMeme;
GX2Texture textureDRCCornerButton;
GX2Texture textureDRCBackImage;
GX2Texture textureDRCTitleButton;
GX2Texture fontTexture;

//Stuff for state
VPADStatus vpad;
float tpXPos = 0.0f;
float tpYPos = 0.0f;
bool tpTouched = false;
bool app_is_running = true;

//Transition state
bool gui_transitioning = true;
bool gui_transitioning_in = true;
int gui_transition_stationary_frames = 0;
float gui_transition_scale = 0.9f;
float gui_transition_alpha = 0.0f;
float gui_transition_xshift = 0.0f;
float gui_transition_yshift = 0.0f;
float gui_transition_scale_delta = 0.0f;
float gui_transition_alpha_delta = 0.0f;
float gui_transition_xshift_delta = 0.0f;
float gui_transition_yshift_delta = 0.0f;

int current_state = STATE_LOADING;

void gui_init()
{
    current_state = STATE_LOADING;

    load_img_texture(&texture, "placeholder-top.png");
    load_img_texture(&textureBackdrop, "backdrop.png");
    load_img_texture(&textureHeader, "header.png");
    load_img_texture(&textureInfoHeader, "header_instructions.png");
    load_img_texture(&textureContent, "content.png");
    load_img_texture(&textureContentSubheader, "subheader.png");
    load_img_texture(&textureBullet, "bullet.png");
    load_img_texture(&textureFolder, "folder.png");
    load_img_texture(&textureFSTFolder, "folder_fst.png");
    load_img_texture(&textureYButton, "y_button.png");
    load_img_texture(&textureIconBack, "icon_back.png");
    load_img_texture(&textureProgressBar, "progress_bar_end.png");
    load_img_texture(&textureDRCArrow, "drcarrow.png");
    load_img_texture(&textureDRCButton, "drcbutton.png");
    load_img_texture(&textureDRCInstall, "install-icon.png");
    load_img_texture(&textureDRCMeme, "meme.png");
    load_img_texture(&textureDRCShadow, "shadow.png");
    load_img_texture(&textureDRCCornerButton, "corner-button.png");
    load_img_texture(&textureDRCBackImage, "back_graphic.png");
    load_img_texture(&textureDRCTitleButton, "titlebutton.png");
    
    current_state = STATE_MAIN_MENU;
    //current_state = STATE_TITLEMANAGE_MENU;
}

void gui_transition_update()
{
    if(gui_transitioning)
    {
        if(gui_transitioning_in)
        {
            gui_transition_scale += gui_transition_scale_delta;
            gui_transition_alpha += gui_transition_alpha_delta;
            gui_transition_xshift += gui_transition_xshift_delta;
            gui_transition_yshift += gui_transition_yshift_delta;
            
            if(gui_transition_scale > 1.0f)
                gui_transition_scale = 1.0f;
            if(gui_transition_alpha > 1.0f)
                gui_transition_alpha = 1.0f;
                
            if((gui_transition_yshift_delta < 0.0f && gui_transition_yshift < 0.0f) || (gui_transition_yshift_delta > 0.0f && gui_transition_yshift > 0.0f))
                gui_transition_yshift = 0.0f;
            if((gui_transition_xshift_delta < 0.0f && gui_transition_xshift < 0.0f) || (gui_transition_xshift_delta > 0.0f && gui_transition_xshift > 0.0f))
                gui_transition_xshift = 0.0f;
                
            if(gui_transition_scale >= 1.0f && gui_transition_alpha >= 1.0f)
            {
                gui_transition_stationary_frames++;
                if(gui_transition_stationary_frames > 10)
                {
                    gui_transitioning = false;
                    gui_transition_stationary_frames = 0;
                }
            }
        }
        else
        {
            gui_transition_scale += gui_transition_scale_delta;
            gui_transition_alpha += gui_transition_alpha_delta;
            gui_transition_xshift += gui_transition_xshift_delta;
            gui_transition_yshift += gui_transition_yshift_delta;
            
            if(gui_transition_alpha < 0.0f)
                gui_transition_alpha = 0.0f;
                
            if(gui_transition_alpha <= 0.0f)
            {
                gui_transition_stationary_frames++;
                if(gui_transition_stationary_frames > 10)
                {
                    gui_transitioning = false;
                    gui_transition_stationary_frames = 0;
                }
            }
        }
    }
    
    draw_set_global_alpha(gui_transition_alpha);
    draw_set_global_scale(gui_transition_scale);
    draw_set_global_xshift(gui_transition_xshift);
    draw_set_global_yshift(gui_transition_yshift);
}

void gui_transition_in(float start_scale, float start_xshift, float start_yshift, float frames)
{
    gui_transition_scale = start_scale;  
    gui_transition_alpha = 0.0f;
    gui_transition_xshift = start_xshift;
    gui_transition_yshift = start_yshift;
    gui_transition_scale_delta = (1.0f - gui_transition_scale) / frames;
    gui_transition_alpha_delta = (1.0f - gui_transition_alpha) / frames;
    gui_transition_xshift_delta = (0.0f - gui_transition_xshift) / frames;
    gui_transition_yshift_delta = (0.0f - gui_transition_yshift) / frames;
    
    gui_transitioning = true;
    gui_transitioning_in = true;
}

void gui_transition_out(float end_scale, float end_xshift, float end_yshift, float frames)
{
    gui_transition_alpha = 1.0f;
    gui_transition_xshift = 0.0f;
    gui_transition_yshift = 0.0f;
    gui_transition_scale_delta = (end_scale - gui_transition_scale) / frames;
    gui_transition_alpha_delta = (0.0f - gui_transition_alpha) / frames;
    gui_transition_xshift_delta = (end_xshift - gui_transition_xshift) / frames;
    gui_transition_yshift_delta = (end_yshift - gui_transition_yshift) / frames;

    gui_transitioning = true;
    gui_transitioning_in = false;
}

bool gui_transition_done()
{
    return !gui_transitioning;
}
