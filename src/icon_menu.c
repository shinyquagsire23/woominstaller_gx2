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

#include "icon_menu.h"

button_t *button_install;
button_t *button_left;
button_t *button_right;

int rendered_frames = 0;
float icon_sine = 0.0f;

int num_menus = 0;
int current_menu = 0;
int target_menu = 0;
menu_index_t *menus;

bool transition = false;
bool transition_left = false;
float transition_x_shift = 0.0f;
float transition_y_icon_shift = 0.0f;
float transition_y_icon_height = 0.0f;

const char *install_texts[2] = {"Install Package Files", "Install Raw FST Packages"};
const char *filebrowse_texts[2] = {"Browse Full Filesystem", "Move/Copy/Delete Files and Folders"};
const char *titlemanage_texts[2] = {"Move/Copy/Delete Applications", "Manage Storage"};
const char *meme_texts[4] = {"meme","meme","meme","meme"};
const char *indev_texts[3] = {"indev", "test", "Copy A Certain Dir..."};

//Screen transition state
int next_state;
bool transitioning_out = false;

void test_indev(button_t* button)
{
    OSReport("Indev: Indev button pressed!\n");
}

void test_install(button_t *button)
{
    OSReport("install button was pressed\n");
    
    //Set transitioning
    transitioning_out = true;
    gui_transition_out(1.05f, 0.0f, 0.0f, 10.0f);
    next_state = STATE_INSTALL_MENU;
}

void press_titlemanage(button_t *button)
{
    OSReport("titlemanage button was pressed\n");
    
    //Set transitioning
    transitioning_out = true;
    gui_transition_out(1.05f, 0.0f, 0.0f, 10.0f);
    next_state = STATE_TITLEMANAGE_MENU;
}

void press_filebrowse(button_t *button)
{
    OSReport("filebrowse button was pressed\n");
    
    //Set transitioning
    transitioning_out = true;
    gui_transition_out(1.05f, 0.0f, 0.0f, 10.0f);
    next_state = STATE_FILEBROWSE_MENU;
}

void test_meme(button_t *button)
{
    OSReport("meme button was pressed\n");
}

void button_press_left(button_t *button)
{
    OSReport("left button was pressed\n");
    
    if(current_menu > 0 && !transition)
    {
        transition = true;
        transition_left = true;
        transition_x_shift = 0.0f;
        
        target_menu--;
    }
}

void button_press_right(button_t *button)
{
    OSReport("right button was pressed\n");
    
    if(current_menu < num_menus-1 && !transition)
    {
        transition = true;
        transition_left = false;
        transition_x_shift = 0.0f;
        
        target_menu++;
    }
}

void icon_menu_add_menu(GX2Texture *icon, char *button_text, int num_texts, const char **texts, void *callback)
{
    OSReport("Adding menu %u, %s\n", num_menus, button_text);
    menus = realloc(menus, sizeof(menu_index_t)*++num_menus);
    menus[num_menus-1].icon = icon;
    menus[num_menus-1].big_button = button_instantiate(200.0f, (float)(TARGET_HEIGHT / 2.0f) - (512.0f / 2.0f), 1.05f);
    
    //Big button
    button_add_texture(menus[num_menus-1].big_button, &textureDRCButton, 0.0f, 0.0f,512.0f, 512.0f,false,false);
    button_add_texture(menus[num_menus-1].big_button, icon, 120.0f, (256.0f / 2.0f) + 50.0f, 256.0f, 256.0f,false,false);
    button_add_text(menus[num_menus-1].big_button, (512.0f/2.0f) - (font_measure_string_width_height(60, button_text) / 2), (256.0f / 2.0f) - 20.0f, 60, FONT_COLOR_BLACK, button_text);
    button_add_inflate_release_event(menus[num_menus-1].big_button, callback);
    
    menus[num_menus-1].num_texts = num_texts;
    menus[num_menus-1].texts = (char**)texts;
}

void icon_menu_init()
{    
    gui_transition_in(1.0f, 0.0f, 0.0f, 30.0f);

    icon_menu_add_menu(&textureDRCInstall, "Install Packages", 2, install_texts, &test_install);
    icon_menu_add_menu(&textureDRCMeme, "Browse Filesystem", 2, filebrowse_texts, &press_filebrowse);
    
#ifdef DEBUG
    icon_menu_add_menu(&textureDRCMeme, "Manage Titles", 2, titlemanage_texts, &press_titlemanage);
    icon_menu_add_menu(&textureDRCMeme, "Indev", 3, indev_texts, &test_indev);
    icon_menu_add_menu(&textureDRCMeme, "Meme", 4, meme_texts, &test_meme);
    icon_menu_add_menu(&textureDRCMeme, "Meme", 4, meme_texts, &test_meme);
    icon_menu_add_menu(&textureDRCMeme, "Meme", 4, meme_texts, &test_meme);
#endif
    
    
    //Arrows
    button_left = button_instantiate(-32.0f, (float)(TARGET_HEIGHT / 2.0f) - (290.0f / 2.0f), 1.1f);
    button_add_texture(button_left, &textureDRCArrow, 0.0f, 0.0f, 290.0f, 290.0f,true,false);
    button_add_inflate_release_event(button_left, &button_press_left);
    button_left->x_pos -= 200.0f;
    
    button_right = button_instantiate((float)TARGET_WIDTH + 32.0f - 290.0f, (float)(TARGET_HEIGHT / 2.0f) - (290.0f / 2.0f), 1.1f);
    button_add_texture(button_right, &textureDRCArrow, 0.0f, 0.0f, 290.0f, 290.0f,false,false);
    button_add_inflate_release_event(button_right, &button_press_right);
}

void icon_menu_update()
{
    if(transitioning_out && gui_transition_done())
    {
        transitioning_out = false;
        if(next_state == STATE_INSTALL_MENU)
        {
            install_menu_init();
            current_state = STATE_INSTALL_MENU;
            
            gui_transition_in(1.0f, 0.0f, 60.0f, 30.0f);
        }
        else if(next_state == STATE_FILEBROWSE_MENU)
        {
            filebrowse_menu_init();
            current_state = STATE_FILEBROWSE_MENU;
            
            gui_transition_in(1.0f, 0.0f, 60.0f, 30.0f);
        }
        else if(next_state == STATE_TITLEMANAGE_MENU)
        {
            titlemanage_menu_init();
            current_state = STATE_TITLEMANAGE_MENU;
            
            gui_transition_in(1.0f, 0.0f, 60.0f, 30.0f);
        }
    }

    button_update(button_left, tpXPos, tpYPos, tpTouched);
    button_update(button_right, tpXPos, tpYPos, tpTouched);
    
    if(current_menu <= 0 && button_left->x_pos >= -220.0f)
    {
        button_left->x_pos -= 200.0f/10.0f;
    }
    else if(button_left->x_pos < -40.0f)
    {
        button_left->x_pos += 200.0f/10.0f;
    }
        
    if(current_menu >= num_menus-1 && button_right->x_pos < (float)TARGET_WIDTH + 32.0f - 290.0f + 200.0f)
    {
        button_right->x_pos += 200.0f/10.0f;
    }
    else if(button_right->x_pos > (float)TARGET_WIDTH + 32.0f - 290.0f)
    {
        button_right->x_pos -= 200.0f/10.0f;
    }
        
    if(!transition)
    {
        for(int i = 0; i < num_menus; i++)
        {
            button_update(menus[i].big_button, tpXPos, tpYPos, tpTouched);        
        }
    }
    else
    {
        if(transition_left)
        {
            transition_x_shift += (1800.0f*(current_menu-target_menu))/20.0f;
            if(transition_x_shift >= (1800.0f*(current_menu-target_menu)))
            {
                transition_x_shift = 0.0f;
                current_menu = target_menu;
                transition = false;
            }
        }
        else
        {
            transition_x_shift -= (1800.0f*(target_menu-current_menu))/20.0f;
            if(transition_x_shift <= -(1800.0f*(target_menu-current_menu)))
            {
                transition_x_shift = 0.0f;
                current_menu = target_menu;
                transition = false;
            }
        }
    }
}

void icon_menu_draw(bool screen)
{
    //Draw our header
    //increment += 1;
    char *header_string = "Installer";//"Woom\xEFnstaller";
    if(!screen)
    {
        rendered_frames++;
        
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
        for(int i = 0; i < num_menus; i++)
        {
            float index_shift =  (float)(i-current_menu)*1800.0f + transition_x_shift;
            render_texture_nostretch(&textureContent, 50.0f + index_shift, -128.0f + 350.0f, 100.0f, 100.0f, 240.0f, (float)TARGET_WIDTH, (float)TARGET_HEIGHT - 400.0f);
            
            menus[i].big_button->x_pos = 200.0f + index_shift;
            button_draw(menus[i].big_button);
            
            //Spacing for four
            if(menus[i].num_texts == 4)
            {
                render_texture(&textureBullet, 750.0f + index_shift, 775.0f-(160.0f*0), 32.0f, 32.0f);
                font_draw_string_height(790.0f + index_shift, 775.0f-(160.0f*0), 80, menus[i].texts[0]);
                
                render_texture(&textureBullet, 750.0f + index_shift, 775.0f-(160.0f*1), 32.0f, 32.0f);
                font_draw_string_height(790.0f + index_shift, 775.0f-(160.0f*1), 80, menus[i].texts[1]);
                
                render_texture(&textureBullet, 750.0f + index_shift, 775.0f-(160.0f*2), 32.0f, 32.0f);
                font_draw_string_height(790.0f + index_shift, 775.0f-(160.0f*2), 80, menus[i].texts[2]);
                
                render_texture(&textureBullet, 750.0f + index_shift, 775.0f-(160.0f*3), 32.0f, 32.0f);
                font_draw_string_height(790.0f + index_shift, 775.0f-(160.0f*3), 80, menus[i].texts[3]);
            }
            else if(menus[i].num_texts == 2)
            {
                //Spacing for two
                render_texture(&textureBullet, 750.0f + index_shift, 775.0f-(160.0f*1), 32.0f, 32.0f);
                font_draw_string_height(790.0f + index_shift, 775.0f-(160.0f*1), 80, menus[i].texts[0]);
                
                render_texture(&textureBullet, 750.0f + index_shift, 775.0f-(160.0f*2), 32.0f, 32.0f);
                font_draw_string_height(790.0f + index_shift, 775.0f-(160.0f*2), 80, menus[i].texts[1]);
            }
        }
        
        //Arrows
        button_draw(button_left);
        button_draw(button_right);
        
        //Row of icons at the bottom
        icon_menu_draw_icons();
    }
}

void icon_menu_draw_icons()
{
    //Either increment sine or do transition adjustment
    if(!transition)
    {
        icon_sine += 0.125f;
        transition_y_icon_shift = 0.0f;
    }
    else
    {
        if(transition_y_icon_height == 0.0f && transition)
            transition_y_icon_height = 60.0f + sin(icon_sine)*7.0f;
        
        transition_y_icon_shift += (transition_y_icon_height - 24.0f) / 10.0f;
    }
        
    for(int i = 0; i < num_menus; i++)
    {
        //Cool bobbing + shadow effect, layer more shading w/ less height
        float height = 24.0f;
        if(i == current_menu)
        {
            height = 60.0f + sin(icon_sine)*7.0f - transition_y_icon_shift;
        }
        else if(i == target_menu)
        {
            height += transition_y_icon_shift;
        }
    
        render_texture_color(&textureDRCShadow, (float)(TARGET_WIDTH / 2) - (float)(num_menus * 140 / 2) + (float)(i*140), 0.0f, 100.0f, 100.0f, 1.0f, 1.0f, 1.0f, 15.0f / height);
        
        render_texture(menus[i].icon, (float)(TARGET_WIDTH / 2) - (float)(num_menus * 140 / 2) + (float)(i*140), height, 100.0f, 100.0f);
        
        if(in_rect(tpXPos, tpYPos, (float)(TARGET_WIDTH / 2) - (float)(num_menus * 140 / 2) + (float)(i*140), height, 100.0f, 100.0f) && !transition && tpTouched)
        {
            transition = true;
            transition_left = (i < current_menu);
            transition_x_shift = 0.0f;
            
            target_menu = i;
        }
            
    }
}
