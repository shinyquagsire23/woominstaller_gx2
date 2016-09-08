/*
 *  Woomïnstaller GX2 - Homebrew package installer for Wii U
 *
 *  Copyright (C) 2016          SALT
 *  Copyright (C) 2016          Max Thomas (Shiny Quagsire) <mtinc2@gmail.com>
 *
 *  This code is licensed under the terms of the GNU LGPL, version 2.1
 *  see file LICENSE.md or https://www.gnu.org/licenses/lgpl-2.1.txt
 */

#include "titlemanage_menu.h"

//Screen transition state
int template_next_state;
bool template_transitioning_out = false;

void template_test_meme(button_t *button)
{
    OSReport("meme button was pressed\n");
}

void template_menu_init()
{    
     
}

void template_menu_update()
{
    if(template_transitioning_out && gui_transition_done())
    {
        template_transitioning_out = false;
        if(template_next_state == STATE_INSTALL_MENU)
        {
            install_menu_init();
            current_state = STATE_INSTALL_MENU;
            
            gui_transition_in(1.0f, 0.0f, 60.0f, 30.0f);
        }
        else if(template_next_state == STATE_FILEBROWSE_MENU)
        {
            filebrowse_menu_init();
            current_state = STATE_FILEBROWSE_MENU;
            
            gui_transition_in(1.0f, 0.0f, 60.0f, 30.0f);
        }
    }
    
    //Button updates, etc
}

void template_menu_draw(bool screen)
{
    //Draw our header
    char *header_string = "Template";
    if(!screen)
    {
        render_texture_color(&textureHeader, 0.0f, (float)(TARGET_HEIGHT - 256), (float)TARGET_WIDTH, 256.0f, NEUTRAL_COLOR);
        font_set_color(FONT_COLOR_WHITE);
        font_draw_string_height((float)(TARGET_WIDTH/2 - font_measure_string_width_height(70, header_string)/2), (float)(TARGET_HEIGHT - 70), 70, header_string);
    }
    else
    {
        render_texture(&textureInfoHeader, 0.0f, (float)(TARGET_HEIGHT - 256), (float)TARGET_WIDTH, 256.0f);
        font_set_color(FONT_COLOR_WHITE);
        font_draw_string_height(64.0f, (float)(TARGET_HEIGHT - 120), 100, header_string);
    }
    
    //Contents
    if(screen)
    {
        render_texture_nostretch(&textureContent, 0.0f, 0.0f, 100.0f, 100.0f, 240.0f, (float)TARGET_WIDTH, (float)TARGET_HEIGHT - 150.0f);
        render_texture(&textureContentSubheader, 55.0f, (float)(TARGET_HEIGHT - 200 - 190), 1700.0f, 200.0f);
        
        font_set_color(FONT_COLOR_WHITE);
        font_draw_string_height(100.0f, (float)(TARGET_HEIGHT - 280), 100, "He has no style");
        font_set_color(FONT_COLOR_BLACK);
        font_draw_string_height(1080.0f, (float)(TARGET_HEIGHT - 470.0f), 70, "He has no grace. This");
        font_draw_string_height(1080.0f, (float)(TARGET_HEIGHT - 470.0f - 70.0f), 70, "kong has a funny face.");
        
        render_texture(&texture, 65.0f, 55.0f, 1024.0f, 1024.0f);
    }
    else
    {
        //Bottom screen stuff
    }
}
