/*
 *  Woomïnstaller GX2 - Homebrew package installer for Wii U
 *
 *  Copyright (C) 2016          SALT
 *  Copyright (C) 2016          Max Thomas (Shiny Quagsire) <mtinc2@gmail.com>
 *
 *  This code is licensed under the terms of the GNU LGPL, version 2.1
 *  see file LICENSE.md or https://www.gnu.org/licenses/lgpl-2.1.txt
 */

#include "filebrowse_menu.h"

void filebrowse_deinit_entry_buttons();
void filebrowse_initialize_entry_buttons();
void filebrowse_move_up_directory();

//Global vars
button_t *button_back;
bool filebrowse_is_initialized = false;
bool filebrowse_transition_out = false;
float filebrowse_transition_y = 0.0f;

//File browsing state
struct dirent **directory_read;
char *current_directory;
int num_entries;
int filebrowse_dir_level = 0;

bool filebrowse_file_transition_in = false;
bool filebrowse_file_transition_out = false;
int next_num_entries;

//Scroll state
bool last_tp_touched;
float start_tp_x;
float start_tp_y;
float last_tp_x;
float last_tp_y;

float filebrowse_scroll_y = 0.0f;
float filebrowse_stick_velocity = 10.0f;
bool filebrowse_scrolling_y = false;
//bool is_using_touch = true; //TODO: Maybe allow buttons for file select?

button_t **entry_buttons;

void filebrowse_entry_button_press(button_t *button)
{
    if(!filebrowse_is_initialized) return;

    struct dirent *entry = (struct dirent*)button->extra_data;
    if(!entry) return;
    
    if(entry->d_type & DT_DIR)
    { 
        {
            filebrowse_dir_level++;

            strcat(current_directory, entry->d_name);
            strcat(current_directory, "/");
            int entries = read_directory(current_directory, directory_read);
            if(!entries)
            {
                filebrowse_dir_level--;
                filebrowse_move_up_directory();
                num_entries = read_directory(current_directory, directory_read);
            }
            else
            {
                next_num_entries = entries;
                filebrowse_file_transition_out = true;
                gui_transition_out(1.0f, 0.0f, -60.0f, 25.0f);
            }
        }
    }
    else
    {
        //Something with files
    }
}

void filebrowse_button_press_back(button_t *button)
{
    OSReport("back button was pressed\n");
    
    if(filebrowse_dir_level != 0)
    {
        filebrowse_dir_level--;
        
        filebrowse_move_up_directory();
        next_num_entries = read_directory(current_directory, directory_read);
        
        filebrowse_file_transition_out = true;
        gui_transition_out(1.0f, 0.0f, -60.0f, 25.0f);
    }
    else
    {
        filebrowse_transition_out = true;
        gui_transition_out(1.0f, 0.0f, -60.0f, 30.0f);
    }
}

void filebrowse_move_up_directory()
{
    // Backtrack stdlib directory
    current_directory[strlen(current_directory)-1] = 0;
    int len = strlen(current_directory)-1;
    
    for(int i = len; i >= 0; i--)
    {
        if(current_directory[i] == '/')
            break;
        
        current_directory[i] = 0;
    }
}

void filebrowse_menu_init()
{
    current_directory = malloc(0x200);
    strcpy(current_directory, "fs:/vol/storage_mlc01/");
    filebrowse_transition_y = 100.0f;

    directory_read = memalign(0x20, sizeof(struct dirent*)*0x200);
    for(int i = 0; i < 0x200; i++)
    {
        struct dirent *entry = memalign(0x20, sizeof(struct dirent));
        directory_read[i] = entry;
    }
    
    num_entries = read_directory(current_directory, directory_read);
    filebrowse_initialize_entry_buttons();    
        
    button_back = button_instantiate(-250.0f, -250.0f, 1.05f);
    button_add_texture(button_back, &textureDRCCornerButton, 0.0f, 0.0f, 512.0f, 512.0f,false,false);
    button_add_texture(button_back, &textureDRCBackImage, 0.0f, -40.0f, 256.0f, 256.0f,false,false);
    button_add_inflate_release_event(button_back, &filebrowse_button_press_back);
    button_add_button_map(button_back, VPAD_BUTTON_B);
    button_back->x_center_scale = false;
    button_back->y_center_scale = false;
    button_back->width = 350;
    button_back->height = 250;
    button_back->selected = true;
    
    filebrowse_is_initialized = true;
}

void filebrowse_menu_deinit()
{
    button_destroy(button_back);
    
    filebrowse_deinit_entry_buttons();
    
    for(int i = 0; i < 0x200; i++)
    {
        free(directory_read[i]);
    }
    free(directory_read);
    
    free(current_directory);
    
    filebrowse_is_initialized = false;
}

void filebrowse_menu_update()
{
    if(!filebrowse_is_initialized) return;
    
    //Button management
    /*if(!(vpad.hold & VPAD_BUTTON_Y) && (last_button_state & VPAD_BUTTON_Y))
        install_cycle_device();
    if(!(vpad.hold & VPAD_BUTTON_X) && (last_button_state & VPAD_BUTTON_X))
        install_abort_current();
    last_button_state = vpad.hold;*/
    
    if(filebrowse_file_transition_out && gui_transition_done())
    {
        filebrowse_file_transition_out = false;
        filebrowse_deinit_entry_buttons();
                
        num_entries = next_num_entries;
        filebrowse_initialize_entry_buttons();
        
        filebrowse_file_transition_in = true;
        gui_transition_in(1.0f, 0.0f, -60.0f, 25.0f);
    }
    
    if(filebrowse_file_transition_in && gui_transition_done())
        filebrowse_file_transition_in = false;
    
    if(filebrowse_transition_out && gui_transition_done())
    {
        filebrowse_transition_out = false;
        filebrowse_menu_deinit();
        current_state = STATE_MAIN_MENU;
        
        gui_transition_in(1.0f, 0.0f, 0.0f, 30.0f);
        return;
    }
    
    if(filebrowse_transition_out)
    {
        button_back->y_pos -= 8.0f;
        button_back->x_pos -= 8.0f;
        filebrowse_transition_y += 5.0f;
    }
    else
    {
        if(filebrowse_transition_y > 0.0f)
            filebrowse_transition_y -= 5.0f;
            
        if(button_back->x_pos < 0.0f)
        {
            button_back->y_pos += 8.0f;
            button_back->x_pos += 8.0f;
        }
        
        if(button_back->x_pos > 0.0f)
        {
            button_back->y_pos = 0.0f;
            button_back->x_pos = 0.0f;
        }
    }
    
    button_update(button_back, tpXPos, tpYPos, tpTouched);
    
    //Touch screen scrolling
    if(tpTouched && !last_tp_touched)
    {
        start_tp_x = tpXPos;
        start_tp_y = tpYPos;
    }
    
    if((start_tp_y - tpYPos > 30.0f || start_tp_y - tpYPos < -30.0f || filebrowse_scrolling_y) && tpTouched)
    {
        filebrowse_scroll_y += last_tp_y - tpYPos;
        
        //File scroll limits
        if(filebrowse_scroll_y > 0.0f) filebrowse_scroll_y = 0.0f;
        if(filebrowse_scroll_y < -250.0f*(num_entries-3)) filebrowse_scroll_y = -250.0f*(num_entries-3);
        if(num_entries < 4) filebrowse_scroll_y = 0.0f;
        
        filebrowse_scrolling_y = true;
    }
    else
    {
        filebrowse_scrolling_y = false;
    }
    
    //Left stick scrolling
    if(vpad.leftStick.y > 0.01f || vpad.leftStick.y < -0.01f)
    {
        filebrowse_scrolling_y = true;
        
        //File scroll limits
        if(filebrowse_scroll_y > 0.0f) filebrowse_scroll_y = 0.0f;
        if(filebrowse_scroll_y < -250.0f*(num_entries-3)) filebrowse_scroll_y = -250.0f*(num_entries-3);
        if(num_entries < 4) filebrowse_scroll_y = 0.0f;
        
        filebrowse_stick_velocity += 1.0f;
        if(filebrowse_stick_velocity > 25.0f)
            filebrowse_stick_velocity = 25.0f;
    }
    else
    {
        if(!tpTouched)
            filebrowse_scrolling_y = false;
        filebrowse_stick_velocity = 10.0f;
    }
    filebrowse_scroll_y += (vpad.leftStick.y * filebrowse_stick_velocity);
    
    last_tp_x = tpXPos;
    last_tp_y = tpYPos;
    last_tp_touched = tpTouched;
    
    for(int i = 0; i < num_entries; i++)
    {
        entry_buttons[i]->y_pos = (float)(TARGET_HEIGHT - 400) - 250.0f*i - filebrowse_scroll_y;
        button_update(entry_buttons[i], filebrowse_scrolling_y ? -entry_buttons[i]->x_pos : tpXPos, filebrowse_scrolling_y ? -entry_buttons[i]->y_pos : tpYPos, tpTouched);
        
        if(button_back->touching)
        {
            entry_buttons[i]->touching = false;
        }
    }
}

void filebrowse_menu_draw(bool screen)
{
    if(!filebrowse_is_initialized) return;
    
    //Draw our header
    char *header_string = "Browse Filesystem";//"Woom\xEFnstaller";
    if(!screen)
    {
        for(int i = 0; i < num_entries; i++)
        {
            button_draw(entry_buttons[i]);
        }
        
        draw_set_use_globals(false);
        render_texture_color(&textureHeader, 0.0f, (float)(TARGET_HEIGHT - 256) + filebrowse_transition_y, (float)TARGET_WIDTH, 256.0f, INSTALL_COLOR);
        font_set_color(FONT_COLOR_WHITE);
        font_draw_string_height((float)(TARGET_WIDTH/2 - font_measure_string_width_height(70, header_string)/2), (float)(TARGET_HEIGHT - 70) + filebrowse_transition_y, 70, header_string);
        draw_set_use_globals(true);
    }
    
    //Contents
    if(screen)
    {
        draw_set_use_global_shifts(false);
        if(filebrowse_file_transition_out || filebrowse_file_transition_in)
            draw_set_use_globals(false);

        render_texture_nostretch(&textureContent, 0.0f, 0.0f, 100.0f, 100.0f, 240.0f, (float)TARGET_WIDTH, (float)TARGET_HEIGHT - 150.0f);
        render_texture(&textureContentSubheader, 55.0f, (float)(TARGET_HEIGHT - 200 - 190), 1700.0f, 200.0f);
        
        font_set_color(FONT_COLOR_WHITE);
        font_draw_string_height(100.0f, (float)(TARGET_HEIGHT - 280), 100, "He has no style");
        font_set_color(FONT_COLOR_BLACK);
        font_draw_string_height(1080.0f, (float)(TARGET_HEIGHT - 470.0f), 70, "He has no grace. This");
        font_draw_string_height(1080.0f, (float)(TARGET_HEIGHT - 470.0f - 70.0f), 70, "kong has a funny face.");
        
        render_texture(&texture, 65.0f, 55.0f, 1024.0f, 1024.0f);
        
        render_texture(&textureInfoHeader, 0.0f, (float)(TARGET_HEIGHT - 256), (float)TARGET_WIDTH, 256.0f);
        font_set_color(FONT_COLOR_WHITE);
        font_draw_string_height(64.0f, (float)(TARGET_HEIGHT - 120), 100, header_string);
        
        draw_set_use_global_shifts(true);
        draw_set_use_globals(true);
    }
    else
    {
        draw_set_use_globals(false);
        button_draw(button_back);
        draw_set_use_globals(true);
    }
}

void filebrowse_deinit_entry_buttons()
{
    for(int i = 0; i < num_entries; i++)
        button_destroy(entry_buttons[i]);
    free(entry_buttons);
}

void filebrowse_initialize_entry_buttons()
{
    filebrowse_scroll_y = 0.0f;
    entry_buttons = calloc(num_entries, sizeof(button_t*));
    
    for(int i = 0; i < num_entries; i++)
    {
        entry_buttons[i] = button_instantiate(50.0f, (float)(TARGET_HEIGHT - 400) - 250.0f*i, 1.025f);
        button_add_texture_nostretch(entry_buttons[i], &textureContent, 0.0f, 0.0f, (float)TARGET_WIDTH, 250.0f, 100.0f, 100.0f, 100.0f, false, false);
        entry_buttons[i]->height -= 100.0f; 
        
        button_add_text(entry_buttons[i], 330.0f, 100.0f, 100, FONT_COLOR_BLACK, directory_read[i]->d_name);
        
        if(directory_read[i]->d_type & DT_DIR)
            button_add_texture(entry_buttons[i], &textureFolder, 80.0f, 20.0f, 200.0f, 200.0f, false, false);

        entry_buttons[i]->extra_data = directory_read[i];
        button_add_inflate_release_event(entry_buttons[i], filebrowse_entry_button_press);
    }
}
