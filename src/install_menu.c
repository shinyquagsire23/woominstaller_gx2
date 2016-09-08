/*
 *  Woomïnstaller GX2 - Homebrew package installer for Wii U
 *
 *  Copyright (C) 2016          SALT
 *  Copyright (C) 2016          Max Thomas (Shiny Quagsire) <mtinc2@gmail.com>
 *
 *  This code is licensed under the terms of the GNU LGPL, version 2.1
 *  see file LICENSE.md or https://www.gnu.org/licenses/lgpl-2.1.txt
 */

#include "install_menu.h"

void deinit_entry_buttons();
void initialize_entry_buttons();
void move_up_directory();

//Global vars
button_t *button_back;
bool is_initialized = false;
bool transition_out = false;
float transition_y = 0.0f;
u32 last_button_state = 0;

//File browsing state
struct dirent **directory_read;
char *current_directory;
char *current_ios_directory;
int num_entries;
int dirLevel = 0;

bool file_transition_in = false;
bool file_transition_out = false;
int next_num_entries;

//Scroll state
bool last_tp_touched = false;
float start_tp_x;
float start_tp_y;
float last_tp_x;
float last_tp_y;

int queue_count = 0;
float install_header_scroll = 0.0f;
float install_queue_scroll = 0.0f;
float scroll_y = 0.0f;
float stick_velocity = 10.0f;
bool scrolling_y = false;
bool is_using_touch = true; //TODO: Maybe allow buttons for file select?

button_t **entry_buttons;

void entry_button_press(button_t *button)
{
    if(!is_initialized) return;

    struct dirent *entry = (struct dirent*)button->extra_data;
    if(!entry) return;
    
    if(entry->d_type & DT_DIR)
    {
        char *fst_check = malloc(strlen(current_directory) + strlen(entry->d_name) + 11);
        strcpy(fst_check, current_directory);
        strcat(fst_check, entry->d_name);
        strcat(fst_check, "/title.tmd");
        OSReport("checking for %s\n", fst_check);
        FILE *fst_check_file = fopen(fst_check, "r");
        
        free(fst_check);
            
        if(fst_check_file)
        {
            fclose(fst_check_file);
            
            char *fst_install = malloc(strlen(current_ios_directory) + strlen(entry->d_name) + 2);
            strcpy(fst_install, current_ios_directory);
            strcat(fst_install, entry->d_name);
            strcat(fst_install, "/");
            install_add_to_queue(fst_install);
            free(fst_install);
        }
        else        
        {
            dirLevel++;

            strcat(current_directory, entry->d_name);
            strcat(current_directory, "/");
            strcat(current_ios_directory, entry->d_name);
            strcat(current_ios_directory, "/");
            int entries = read_directory(current_directory, directory_read);
            if(!entries)
            {
                dirLevel--;
                move_up_directory();
                num_entries = read_directory(current_directory, directory_read);
            }
            else
            {
                next_num_entries = entries;
                file_transition_out = true;
                gui_transition_out(1.0f, 0.0f, -60.0f, 25.0f);
            }
        }
    }
    else
    {
        //Add woomy to install queue
        char *tempstr = malloc(strlen(current_directory)+strlen(entry->d_name)+1);
        strcpy(tempstr, current_directory);
        strcat(tempstr, entry->d_name);
        install_add_to_queue(tempstr);
        free(tempstr);
    }
}

void button_press_back(button_t *button)
{
    OSReport("back button was pressed\n");
    
    if(dirLevel != 0)
    {
        dirLevel--;
        
        move_up_directory();
        next_num_entries = read_directory(current_directory, directory_read);
        
        file_transition_out = true;
        gui_transition_out(1.0f, 0.0f, -60.0f, 25.0f);
    }
    else
    {
        transition_out = true;
        gui_transition_out(1.0f, 0.0f, -60.0f, 30.0f);
    }
}

void move_up_directory()
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
    
    // Backtrack IOS directory
    current_ios_directory[strlen(current_ios_directory)-1] = 0;
    len = strlen(current_ios_directory)-1;
    
    for(int i = len; i >= 0; i--)
    {
        if(current_ios_directory[i] == '/')
            break;
        
        current_ios_directory[i] = 0;
    }
}

void install_menu_init()
{
    current_directory = malloc(0x200);
    current_ios_directory = malloc(0x200);
    strcpy(current_directory, "fs:/vol/external01/");
    strcpy(current_ios_directory, "/vol/app_sd/");
    transition_y = 100.0f;

    directory_read = memalign(0x20, sizeof(struct dirent*)*0x200);
    for(int i = 0; i < 0x200; i++)
    {
        struct dirent *entry = memalign(0x20, sizeof(struct dirent));
        directory_read[i] = entry;
    }
    
    num_entries = read_directory(current_directory, directory_read);
    initialize_entry_buttons();    
        
    button_back = button_instantiate(-250.0f, -250.0f, 1.05f);
    button_add_texture(button_back, &textureDRCCornerButton, 0.0f, 0.0f, 512.0f, 512.0f,false,false);
    button_add_texture(button_back, &textureDRCBackImage, 0.0f, -40.0f, 256.0f, 256.0f,false,false);
    button_add_inflate_release_event(button_back, &button_press_back);
    button_add_button_map(button_back, VPAD_BUTTON_B);
    button_back->x_center_scale = false;
    button_back->y_center_scale = false;
    button_back->width = 350;
    button_back->height = 250;
    button_back->selected = true;
    
    is_initialized = true;
}

void install_menu_deinit()
{
    button_destroy(button_back);
    
    deinit_entry_buttons();
    
    for(int i = 0; i < 0x200; i++)
    {
        free(directory_read[i]);
    }
    free(directory_read);
    
    free(current_ios_directory);
    free(current_directory);
    
    is_initialized = false;
}

void install_menu_update()
{
    if(!is_initialized) return;
    
    //Button management
    if(!(vpad.hold & VPAD_BUTTON_Y) && (last_button_state & VPAD_BUTTON_Y))
        install_cycle_device();
    if(!(vpad.hold & VPAD_BUTTON_X) && (last_button_state & VPAD_BUTTON_X))
        install_abort_current();
    last_button_state = vpad.hold;
    
    if(file_transition_out && gui_transition_done())
    {
        file_transition_out = false;
        deinit_entry_buttons();
                
        num_entries = next_num_entries;
        initialize_entry_buttons();
        
        file_transition_in = true;
        gui_transition_in(1.0f, 0.0f, -60.0f, 25.0f);
    }
    
    if(file_transition_in && gui_transition_done())
        file_transition_in = false;
    
    if(transition_out && gui_transition_done())
    {
        transition_out = false;
        install_menu_deinit();
        current_state = STATE_MAIN_MENU;
        
        gui_transition_in(1.0f, 0.0f, 0.0f, 30.0f);
        return;
    }
    
    if(transition_out)
    {
        button_back->y_pos -= 8.0f;
        button_back->x_pos -= 8.0f;
        transition_y += 5.0f;
    }
    else
    {
        if(transition_y > 0.0f)
            transition_y -= 5.0f;
            
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
    
    if((start_tp_y - tpYPos > 30.0f || start_tp_y - tpYPos < -30.0f || scrolling_y) && tpTouched)
    {
        scroll_y += last_tp_y - tpYPos;
        
        //File scroll limits
        if(scroll_y > 0.0f) scroll_y = 0.0f;
        if(scroll_y < -250.0f*(num_entries-3)) scroll_y = -250.0f*(num_entries-3);
        if(num_entries < 4) scroll_y = 0.0f;
        scrolling_y = true;
    }
    else
    {
        scrolling_y = false;
    }
    
    //Left stick scrolling
    if(vpad.leftStick.y > 0.01f || vpad.leftStick.y < -0.01f)
    {
        scrolling_y = true;
        
        //File scroll limits
        if(scroll_y > 0.0f) scroll_y = 0.0f;
        if(scroll_y < -250.0f*(num_entries-3)) scroll_y = -250.0f*(num_entries-3);
        if(num_entries < 4) scroll_y = 0.0f;
        
        stick_velocity += 1.0f;
        if(stick_velocity > 25.0f)
            stick_velocity = 25.0f;
    }
    else
    {
        if(!tpTouched)
            scrolling_y = false;
        stick_velocity = 10.0f;
    }
    scroll_y += (vpad.leftStick.y * stick_velocity);
    
    last_tp_x = tpXPos;
    last_tp_y = tpYPos;
    last_tp_touched = tpTouched;
    
    for(int i = 0; i < num_entries; i++)
    {
        entry_buttons[i]->y_pos = (float)(TARGET_HEIGHT - 400) - 250.0f*i - scroll_y;
        button_update(entry_buttons[i], scrolling_y ? -entry_buttons[i]->x_pos : tpXPos, scrolling_y ? -entry_buttons[i]->y_pos : tpYPos, tpTouched);
        
        if(button_back->touching)
        {
            entry_buttons[i]->touching = false;
        }
    }
    
    //Manage queue scrolling
    if(queue_count < TARGET_HEIGHT / 250)
    {
        install_header_scroll += 60.0f;
        install_queue_scroll += 60.0f;
    }
    else
    {
        install_queue_scroll += (vpad.rightStick.y * 10.0f);
        if(install_queue_scroll < (float)(queue_count - (TARGET_HEIGHT / 250) + 1) * -250.0f)
            install_queue_scroll = (float)(queue_count - (TARGET_HEIGHT / 250) + 1) * -250.0f;
    }
    
    if(install_queue_scroll < -5.0f)
        install_header_scroll -= 60.0f;
    else
        install_header_scroll += 60.0f;
    
    //Keep queue header from drifting off
    if(install_header_scroll < (float)(-TARGET_WIDTH))
        install_header_scroll = (float)(-TARGET_WIDTH);
    if(install_header_scroll > 0.0f)
        install_header_scroll = 0.0f;
        
    //Keep queue scroll within limits
    if(install_queue_scroll > 0.0f)
        install_queue_scroll = 0.0f;
}

void install_menu_draw(bool screen)
{
    if(!is_initialized) return;
    
    //Draw our header
    char *header_string = "Install Package Files";//"Woom\xEFnstaller";
    if(!screen)
    {
        for(int i = 0; i < num_entries; i++)
        {
            button_draw(entry_buttons[i]);
        }
        
        draw_set_use_globals(false);
        render_texture_color(&textureHeader, 0.0f, (float)(TARGET_HEIGHT - 256) + transition_y, (float)TARGET_WIDTH, 256.0f, INSTALL_COLOR);
        font_set_color(FONT_COLOR_WHITE);
        font_draw_string_height((float)(TARGET_WIDTH/2 - font_measure_string_width_height(70, header_string)/2), (float)(TARGET_HEIGHT - 70) + transition_y, 70, header_string);
        draw_set_use_globals(true);
    }
    
    //Contents
    if(screen)
    {
        draw_set_use_global_shifts(false);
        if(file_transition_out || file_transition_in)
            draw_set_use_globals(false);
        
        queue_count = 0;
        while(1)
        {
            if(install_get_queue_item(queue_count)->targetPath == NULL)
            {
                render_texture_nostretch_color(&textureContent, 0.0f, (float)(TARGET_HEIGHT-150.0f-150.0f) - 250.0f*queue_count - install_queue_scroll, 100.0f, 50.0f, 50.0f, (float)TARGET_WIDTH, 150.0f, 1.0f, 1.0f, 1.0f, 0.6f);
                
                //Get install device string
                char item_string[0x10];
                sprintf(item_string, "Current Install Target: %s%02u", install_get_device(install_get_target())->deviceName, install_get_device(install_get_target())->deviceNum);
                
                render_texture_color(&textureYButton, 60.0f, (float)(TARGET_HEIGHT-270.0f) - 250.0f*queue_count - install_queue_scroll, 128.0f, 128.0f, 1.0f, 1.0f, 1.0f, 0.7f);
                
                font_set_color(FONT_COLOR_GRAY);
                font_draw_string_height(160.0f, (float)(TARGET_HEIGHT-250.0f) - 250.0f*queue_count - install_queue_scroll, 70, item_string);
                
                break;
            }
            
            //Get install device string
            char item_install_device[0x10];
            sprintf(item_install_device, "%s%02u", install_get_device(install_get_queue_item(queue_count)->targetDevice)->deviceName, install_get_device(install_get_queue_item(queue_count)->targetDevice)->deviceNum);
        
            render_texture_nostretch(&textureContent, 0.0f, (float)(TARGET_HEIGHT-250.0f-150.0f) - 250.0f*queue_count - install_queue_scroll, 100.0f, 100.0f, 100.0f, (float)TARGET_WIDTH, 250.0f);
            
            if(install_get_queue_item(queue_count)->installing || install_get_queue_item(queue_count)->pre_install)
            {
                char installing_temp[0x200];
                char contents_temp[0x200];
                    
                if(install_get_queue_item(queue_count)->pre_install)
                {
                    sprintf(installing_temp, "Preparing to install %s from %s", install_get_current_install_name(), install_get_queue_item(queue_count)->archiveName ? install_get_queue_item(queue_count)->archiveName : install_get_queue_item(queue_count)->targetPath);
                    sprintf(contents_temp, "Unpacking contents %u of %u", (unsigned int)install_get_queue_item(queue_count)->current_unpack, (unsigned int)install_get_queue_item(queue_count)->total_unpack);
                }
                else
                {
                    sprintf(installing_temp, "Installing %s from %s", install_get_current_install_name(), install_get_queue_item(queue_count)->archiveName ? install_get_queue_item(queue_count)->archiveName : install_get_queue_item(queue_count)->targetPath);
                    sprintf(contents_temp, "Installing content %u of %u to %s", (unsigned int)install_get_queue_item(queue_count)->current_content, (unsigned int)install_get_queue_item(queue_count)->total_content, item_install_device);
                }
                
                int size = 100;
                while(1)
                {
                    size--;
                    if(font_measure_string_width_height(size, installing_temp) < TARGET_WIDTH-500)
                        break;
                }
                
                font_set_color(FONT_COLOR_BLACK);
                font_draw_string_height(330.0f, (float)(TARGET_HEIGHT-250.0f-80.0f) - 250.0f*queue_count + 100.0f - install_queue_scroll, size, installing_temp);
                    
                font_set_color(FONT_COLOR_GRAY);
                font_draw_string_height(330.0f, (float)(TARGET_HEIGHT-250.0f-150.0f) - 250.0f*queue_count + 100.0f - install_queue_scroll, 70, contents_temp);
                    
                    
                //Render progress bar
                render_texture_partial_color(&textureProgressBar, 330.0f, (float)(TARGET_HEIGHT-250.0f-210.0f) - 250.0f*queue_count + 100.0f - install_queue_scroll, 20, 40, 0, 0, 64, 128, 1.0f, 1.0f, 1.0f, 1.0f);
                render_texture_partial_color(&textureProgressBar, 350.0f, (float)(TARGET_HEIGHT-250.0f-210.0f) - 250.0f*queue_count + 100.0f - install_queue_scroll, 1380, 40, 64, 0, 64, 128, 1.0f, 1.0f, 1.0f, 1.0f);
                render_texture_partial_color(&textureProgressBar, 1730.0f, (float)(TARGET_HEIGHT-250.0f-210.0f) - 250.0f*queue_count + 100.0f - install_queue_scroll, 20, 40, 64, 0, -64, 128, 1.0f, 1.0f, 1.0f, 1.0f);
                    
                int percent_width = (int)(1420.0f * install_get_queue_item(queue_count)->percent_complete);
                int size_left = percent_width > 20 ? 20 : percent_width;
                int size_middle = (percent_width > 20 ? (percent_width > 1400 ? 1380 : percent_width - 20) : 0);
                int size_right = (percent_width > 1400 ? percent_width - 1400 : 0);
                render_texture_partial_color(&textureProgressBar, 330.0f, (float)(TARGET_HEIGHT-250.0f-210.0f) - 250.0f*queue_count + 100.0f - install_queue_scroll, size_left, 40, 0, 0, 64.0f * ((float)size_left / 20.0f), 128, INSTALL_COLOR);
                if(percent_width > 20)
                    render_texture_partial_color(&textureProgressBar, 350.0f, (float)(TARGET_HEIGHT-250.0f-210.0f) - 250.0f*queue_count + 100.0f - install_queue_scroll, size_middle, 40, 64, 0, 64, 128, INSTALL_COLOR);
                    
                if(percent_width > 1400)
                    render_texture_partial_color(&textureProgressBar, 1730.0f, (float)(TARGET_HEIGHT-250.0f-210.0f) - 250.0f*queue_count + 100.0f - install_queue_scroll, size_right, 40, 64, 0, -64.0f * ((float)size_right / 20.0f), 128, INSTALL_COLOR);
            }
            else
            {
                if(install_get_queue_item(queue_count)->archiveName != NULL)
                {
                    font_set_color(FONT_COLOR_BLACK);
                    font_draw_string_height(330.0f, (float)(TARGET_HEIGHT-250.0f-80.0f) - 250.0f*queue_count + 100.0f - install_queue_scroll, 100, install_get_queue_item(queue_count)->archiveName);
                    
                    font_set_color(FONT_COLOR_GRAY);
                    font_draw_string_height(330.0f, (float)(TARGET_HEIGHT-250.0f-150.0f) - 250.0f*queue_count + 100.0f - install_queue_scroll, 70, install_get_queue_item(queue_count)->targetPath);
                    font_draw_string_height(330.0f, (float)(TARGET_HEIGHT-250.0f-210.0f) - 250.0f*queue_count + 100.0f - install_queue_scroll, 70, item_install_device);
                }
                else
                {
                    font_set_color(FONT_COLOR_BLACK);
                    font_draw_string_height(330.0f, (float)(TARGET_HEIGHT-250.0f-100.0f) - 250.0f*queue_count + 100.0f - install_queue_scroll, 100, install_get_queue_item(queue_count)->targetPath);
                    
                    font_set_color(FONT_COLOR_GRAY);
                    font_draw_string_height(330.0f, (float)(TARGET_HEIGHT-250.0f-170.0f) - 250.0f*queue_count + 100.0f - install_queue_scroll, 70, item_install_device);
                }
            }
            
            
            
            
            
            //TODO: FST icons?
            if(install_get_queue_texture(queue_count)->surface.image)
                render_texture(install_get_queue_texture(queue_count), 80.0f, (float)(TARGET_HEIGHT-250.0f-135.0f) - 250.0f*queue_count + 20.0f - install_queue_scroll, 180.0f, 180.0f);
            else
            {
                font_set_color(FONT_COLOR_GRAY);
                font_draw_string_height(130.0f, (float)(TARGET_HEIGHT-250.0f-30.0f) - 250.0f*queue_count + 20.0f - install_queue_scroll, 90, "No");
                font_draw_string_height(110.0f, (float)(TARGET_HEIGHT-250.0f-110.0f) - 250.0f*queue_count + 20.0f - install_queue_scroll, 90, "Icon");
            }
                
            queue_count++;
        }
        
        render_texture(&textureInfoHeader, install_header_scroll, (float)(TARGET_HEIGHT - 256), (float)TARGET_WIDTH, 256.0f);
        font_set_color(FONT_COLOR_WHITE);
        font_draw_string_height(64.0f + (install_header_scroll < -950.0f ? install_header_scroll + 950.0f : 0.0f), (float)(TARGET_HEIGHT - 120), 100, header_string);
        
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

void deinit_entry_buttons()
{
    for(int i = 0; i < num_entries; i++)
        button_destroy(entry_buttons[i]);
    free(entry_buttons);
}

void initialize_entry_buttons()
{
    scroll_y = 0.0f;
    entry_buttons = calloc(num_entries, sizeof(button_t*));
    
    for(int i = 0; i < num_entries; i++)
    {
        entry_buttons[i] = button_instantiate(50.0f, (float)(TARGET_HEIGHT - 400) - 250.0f*i, 1.025f);
        button_add_texture_nostretch(entry_buttons[i], &textureContent, 0.0f, 0.0f, (float)TARGET_WIDTH, 250.0f, 100.0f, 100.0f, 100.0f, false, false);
        entry_buttons[i]->height -= 100.0f; 
        
        button_add_text(entry_buttons[i], 330.0f, 100.0f, 100, FONT_COLOR_BLACK, directory_read[i]->d_name);
        
        if(directory_read[i]->d_type & DT_DIR)
        {
            char *fst_check = malloc(strlen(current_directory) + strlen(directory_read[i]->d_name) + 11);
            strcpy(fst_check, current_directory);
            strcat(fst_check, directory_read[i]->d_name);
            strcat(fst_check, "/title.tmd");
            FILE *fst_check_file = fopen(fst_check, "r");
            free(fst_check);
            
            if(fst_check_file)
            {
                fclose(fst_check_file);
                button_add_texture(entry_buttons[i], &textureFSTFolder, 80.0f, 20.0f, 200.0f, 200.0f, false, false);
            }
            else
                button_add_texture(entry_buttons[i], &textureFolder, 80.0f, 20.0f, 200.0f, 200.0f, false, false);
        }
        //TODO: .woomy icons

        entry_buttons[i]->extra_data = directory_read[i];
        button_add_inflate_release_event(entry_buttons[i], entry_button_press);
    }
}
