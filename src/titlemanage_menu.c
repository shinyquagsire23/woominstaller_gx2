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

#include "ezxml.h"

void titlemanage_buttons_deinit();

//Screen transition state
int titlemanage_next_state;
bool titlemanage_transitioning_out = false;

button_t *button_back;
button_t *button_left;
button_t *button_right;
button_t **entry_buttons;
int titlemanage_num_entries = 10;
GX2Texture **button_textures;

bool titlemanage_transition = false;
bool titlemanage_transition_left = false;
bool titlemanage_browse_installed = true;
bool titlemanage_title_transitioning_out = false;
float titlemanage_transition_x_shift = 0.0f;
int current_page = 0;
int target_page = 0;
int num_pages = 0; //TODO

u64 titlemanage_title = 0x000500001014b700;

int get_save_path_list(char **titles)
{
    char sys_tmp[0x200];
    char usr_tmp[0x200];
    char xml_tmp[0x200];
    
    sprintf(sys_tmp, "/vol/storage_mlc01/sys/save/%08x/%08x", (u32)(titlemanage_title >> 32), (u32)(titlemanage_title));
    sprintf(usr_tmp, "/vol/storage_mlc01/usr/save/%08x/%08x", (u32)(titlemanage_title >> 32), (u32)(titlemanage_title));

    int num_saves = 0;
    sprintf(xml_tmp, "%s/meta/saveinfo.xml", usr_tmp);
    FILE *xml_file = fopen(xml_tmp, "r");
    if(!xml_file) return 0; //TODO
    
    ezxml_t saveinfo_xml = ezxml_parse_fp(xml_file);
    ezxml_t entries = ezxml_get(saveinfo_xml, "account", -1);
    ezxml_t next_entry = ezxml_idx(entries, num_saves);
    if(!next_entry) return 0;
    
    while(next_entry)
    {
        OSReport("save %s %s\n", (char*)ezxml_attr(next_entry, "persistentId"), next_entry->txt);
        
        titles[num_saves] = malloc(0x200);
        snprintf(titles[num_saves], 0x200, "%s/user/%s", usr_tmp, strcmp((char*)ezxml_attr(next_entry, "persistentId"), "00000000") ? (char*)ezxml_attr(next_entry, "persistentId") : "common");
        
        next_entry = ezxml_idx(entries, ++num_saves);
    }
    
    return num_saves;
}


int get_title_path_list(char **titles)
{
    struct dirent **directory_read_sys;
    struct dirent **directory_read_usr;
    struct dirent **directory_read_tmp;
    int title_count = 0;

    directory_read_sys = memalign(0x20, sizeof(struct dirent*)*0x200);
    for(int i = 0; i < 0x200; i++)
    {
        struct dirent *entry = memalign(0x20, sizeof(struct dirent));
        directory_read_sys[i] = entry;
    }
    
    directory_read_usr = memalign(0x20, sizeof(struct dirent*)*0x200);
    for(int i = 0; i < 0x200; i++)
    {
        struct dirent *entry = memalign(0x20, sizeof(struct dirent));
        directory_read_usr[i] = entry;
    }
    
    directory_read_tmp = memalign(0x20, sizeof(struct dirent*)*0x200);
    for(int i = 0; i < 0x200; i++)
    {
        struct dirent *entry = memalign(0x20, sizeof(struct dirent));
        directory_read_tmp[i] = entry;
    }
    
    int entries_base_sys = read_directory("/vol/storage_mlc01/sys/title/", directory_read_sys);
    int entries_base_usr = read_directory("/vol/storage_mlc01/usr/title/", directory_read_usr);

    for(int i = 0; i < entries_base_sys; i++)
    {
        char temp_path[0x200];
        strcpy(temp_path, "/vol/storage_mlc01/sys/title/");
        strcat(temp_path, directory_read_sys[i]->d_name);
        
        int entries = read_directory(temp_path, directory_read_tmp);
        for(int i = 0; i < entries; i++)
        {
            titles[title_count] = malloc(0x200);
            sprintf(titles[title_count++], "%s/%s", temp_path, directory_read_tmp[i]->d_name);
        }
    }
    
    for(int i = 0; i < entries_base_usr; i++)
    {
        char temp_path[0x200];
        strcpy(temp_path, "/vol/storage_mlc01/usr/title/");
        strcat(temp_path, directory_read_sys[i]->d_name);
        
        int entries = read_directory(temp_path, directory_read_tmp);
        for(int i = 0; i < entries; i++)
        {
            titles[title_count] = malloc(0x200);
            sprintf(titles[title_count++], "%s/%s", temp_path, directory_read_tmp[i]->d_name);
        }
    }
    
    
    for(int i = 0; i < 0x200; i++)
    {
        free(directory_read_sys[i]);
    }
    free(directory_read_sys);
    
    for(int i = 0; i < 0x200; i++)
    {
        free(directory_read_usr[i]);
    }
    free(directory_read_usr);
    
    for(int i = 0; i < 0x200; i++)
    {
        free(directory_read_tmp[i]);
    }
    free(directory_read_tmp);

    return title_count;
}

void titlemanage_button_press_title(button_t *button)
{
    u64 tid = ((u64)button->extra_data_2 << 32) + (u32)button->extra_data;
    
    titlemanage_title = tid;
    titlemanage_browse_installed = false;
    titlemanage_title_transitioning_out = true;
    gui_transition_out(1.0f, 0.0f, -60.0f, 25.0f);
}

void titlemanage_button_press_back(button_t *button)
{
    if(titlemanage_browse_installed)
    {
        titlemanage_transitioning_out = true;
        gui_transition_out(1.0f, 0.0f, -60.0f, 30.0f);
    }
    else
    {
        titlemanage_browse_installed = true;
        titlemanage_title_transitioning_out = true;
        gui_transition_out(1.0f, 0.0f, -60.0f, 25.0f);
    }
}

void titlemanage_button_press_left(button_t *button)
{
    OSReport("left button was pressed\n");
    
    if(current_page > 0 && !titlemanage_transition)
    {
        titlemanage_transition = true;
        titlemanage_transition_left = true;
        titlemanage_transition_x_shift = 0.0f;
        
        target_page--;
    }
}

void titlemanage_button_press_right(button_t *button)
{
    OSReport("right button was pressed\n");
    
    if(current_page < num_pages-1 && !titlemanage_transition)
    {
        titlemanage_transition = true;
        titlemanage_transition_left = false;
        titlemanage_transition_x_shift = 0.0f;
        
        target_page++;
    }
}

void titlemanage_title_browse_init()
{
    char *titles[0x200];
    int title_count = get_title_path_list(titles);
    current_page = 0;
    
    titlemanage_num_entries = title_count;
    entry_buttons = calloc(titlemanage_num_entries, sizeof(button_t*));
    button_textures = calloc(titlemanage_num_entries, sizeof(GX2Texture*));
    
    int button_count = 0;
    for(int i = 0; i < titlemanage_num_entries; i++)
    {
        char icon_temp[0x200];
        char xml_temp[0x200];
        sprintf(icon_temp, "%s/meta/iconTex.tga", titles[i]);
        FILE *file = fopen(icon_temp, "r");
        if(!file)
            continue;
    
        fclose(file);
        entry_buttons[button_count] = button_instantiate(200.0f + ((button_count % 2) + (button_count/6)*2) * 800.0f, (TARGET_HEIGHT - 370.0f) - ((button_count/2) - (button_count/6)*3) * 230.0f, 1.025f);
        button_add_texture_nostretch(entry_buttons[button_count], &textureDRCTitleButton, 0.0f, 0.0f, 700.0, 200.0, 32.0f, 32.0f, 32.0f, false, false);
        
        button_textures[button_count] = calloc(sizeof(GX2Texture), 1);
        load_img_texture_mask(button_textures[button_count], icon_temp, "icon_mask.png");
        button_add_texture(entry_buttons[button_count], button_textures[button_count], 25, 25, 160, 160, false, false);
        
        sprintf(xml_temp, "%s/meta/meta.xml", titles[i]);
        FILE *metadata = fopen(xml_temp, "r");
        
        if(!metadata)
        {
            button_add_text(entry_buttons[button_count], 200, 140, 60, FONT_COLOR_GRAY, "Missing Metadata");
        }
        else
        {
            char name_temp[0x100];
            ezxml_t metadata_xml = ezxml_parse_fp(metadata);
            strcpy(name_temp, ezxml_get(metadata_xml, "shortname_en", -1)->txt);
            
            if(font_measure_string_width_height(60, name_temp) > 500)
            {
                int count = 1;
                while(1)
                {
                    memset(name_temp, 0, 0x100);
                    strncpy(name_temp, ezxml_get(metadata_xml, "shortname_en", -1)->txt, strlen(ezxml_get(metadata_xml, "shortname_en", -1)->txt)-count++);
                    strcat(name_temp, "...");
                    
                    if(font_measure_string_width_height(60, name_temp) < 500)
                        break;
                }
            }
            
            if(!strncmp(name_temp, "Woom", 4))
                strcpy(name_temp, "Meme");
            button_add_text(entry_buttons[button_count], 200, 140, 60, FONT_COLOR_BLACK, name_temp);
            //button_add_text(entry_buttons[button_count], 200, 80, 60, FONT_COLOR_GRAY, ezxml_get(metadata_xml, "title_id", -1)->txt);
            
            button_add_inflate_release_event(entry_buttons[button_count], titlemanage_button_press_title);
            entry_buttons[button_count]->extra_data = (void*)(strtoull(ezxml_get(metadata_xml, "title_id", -1)->txt, NULL, 16) & 0xFFFFFFFF);
            entry_buttons[button_count]->extra_data_2 = (void*)(strtoull(ezxml_get(metadata_xml, "title_id", -1)->txt, NULL, 16) >> 32);
            
            ezxml_free(metadata_xml);
            fclose(metadata);
        }
        
        button_count++;
    }
    
    for(int i = 0; i < titlemanage_num_entries; i++)
    {
        char icon_temp[0x200];
        char xml_temp[0x200];
        sprintf(icon_temp, "%s/meta/iconTex.tga", titles[i]);
        FILE *file = fopen(icon_temp, "r");
        if(file)
        {
            fclose(file);
            continue;
        }
    
        entry_buttons[button_count] = button_instantiate(200.0f + ((button_count % 2) + (button_count/6)*2) * 800.0f, (TARGET_HEIGHT - 370.0f) - ((button_count/2) - (button_count/6)*3) * 230.0f, 1.025f);
        button_add_texture_nostretch(entry_buttons[button_count], &textureDRCTitleButton, 0.0f, 0.0f, 700.0, 200.0, 32.0f, 32.0f, 32.0f, false, false);
        
        button_textures[button_count] = calloc(sizeof(GX2Texture), 1);
        load_img_texture_mask(button_textures[button_count], icon_temp, "icon_mask.png");
        button_add_texture(entry_buttons[button_count], button_textures[button_count], 25, 25, 160, 160, false, false);
        
        sprintf(xml_temp, "%s/meta/meta.xml", titles[i]);
        FILE *metadata = fopen(xml_temp, "r");
        
        if(!metadata)
        {
            char tid_str[0x200];
            
            memset(tid_str, 0, 0x200); 
            strncpy(tid_str, titles[i]+strlen("/vol/storage_mlc01/usr/title/"), 8);
            strncat(tid_str, titles[i]+strlen("/vol/storage_mlc01/usr/title/xxxxxxxx/"), 8);
            button_add_text(entry_buttons[button_count], 200, 140, 60, FONT_COLOR_BLACK, tid_str);
            
            button_add_inflate_release_event(entry_buttons[button_count], titlemanage_button_press_title);
            entry_buttons[button_count]->extra_data = (void*)(strtoull(tid_str, NULL, 16) & 0xFFFFFFFF);
            entry_buttons[button_count]->extra_data_2 = (void*)(strtoull(tid_str, NULL, 16) >> 32);
        }
        else
        {
            char name_temp[0x100];
            ezxml_t metadata_xml = ezxml_parse_fp(metadata);
            strcpy(name_temp, ezxml_get(metadata_xml, "shortname_en", -1)->txt);
            
            if(font_measure_string_width_height(60, name_temp) > 500)
            {
                int count = 1;
                while(1)
                {
                    memset(name_temp, 0, 0x100);
                    strncpy(name_temp, ezxml_get(metadata_xml, "shortname_en", -1)->txt, strlen(ezxml_get(metadata_xml, "shortname_en", -1)->txt)-count++);
                    strcat(name_temp, "...");
                    
                    if(font_measure_string_width_height(60, name_temp) < 500)
                        break;
                }
            }
            
            if(!strncmp(name_temp, "Woom", 4))
                strcpy(name_temp, "Meme");
            button_add_text(entry_buttons[button_count], 200, 140, 60, FONT_COLOR_BLACK, name_temp);
            //button_add_text(entry_buttons[button_count], 200, 80, 60, FONT_COLOR_GRAY, ezxml_get(metadata_xml, "title_id", -1)->txt);
            
            button_add_inflate_release_event(entry_buttons[button_count], titlemanage_button_press_title);
            entry_buttons[button_count]->extra_data = (void*)(strtoull(ezxml_get(metadata_xml, "title_id", -1)->txt, NULL, 16) & 0xFFFFFFFF);
            entry_buttons[button_count]->extra_data_2 = (void*)(strtoull(ezxml_get(metadata_xml, "title_id", -1)->txt, NULL, 16) >> 32);
            
            ezxml_free(metadata_xml);
            fclose(metadata);
        }
        
        button_count++;
    }
    
    num_pages = (titlemanage_num_entries/6)+1;
}

void titlemanage_title_detail_init()
{
    char *titles[0x200];
    int title_count = get_save_path_list(titles);
    current_page = 0;
    
    titlemanage_num_entries = title_count;
    entry_buttons = calloc(titlemanage_num_entries, sizeof(button_t*));
    button_textures = calloc(titlemanage_num_entries, sizeof(GX2Texture*));
    
    int button_count = 0;
    for(int i = 0; i < titlemanage_num_entries; i++)
    {
        char icon_temp[0x200];
        char xml_temp[0x200];
        char save_str[0x200];
        
        entry_buttons[button_count] = button_instantiate(200.0f + ((button_count % 2) + (button_count/6)*2) * 800.0f, (TARGET_HEIGHT - 370.0f) - ((button_count/2) - (button_count/6)*3) * 230.0f, 1.025f);
        button_add_texture_nostretch(entry_buttons[button_count], &textureDRCTitleButton, 0.0f, 0.0f, 700.0, 200.0, 32.0f, 32.0f, 32.0f, false, false);
        
        button_textures[button_count] = calloc(sizeof(GX2Texture), 1);
        
        
        sprintf(xml_temp, "%s/../../meta/meta.xml", titles[i]);
        FILE *metadata = fopen(xml_temp, "r");
        
        memset(save_str, 0, 0x200); 
        strcpy(save_str, titles[i]+strlen("/vol/storage_mlc01/usr/title/xxxxxxxx/xxxxxxxx/user"));
        
        //"/vol/storage_mlc01/usr/save/system/act/80000001/miiimg00.dat"
        if(!strcmp(save_str, "common"))
            sprintf(icon_temp, "%s/../../meta/iconTex.tga", titles[i]); //TODO Mii icons
        else
            sprintf(icon_temp, "/vol/storage_mlc01/usr/save/system/act/%s/miiimg00.dat", save_str); //TODO Mii icons
            
            
        load_img_texture_mask(button_textures[button_count], icon_temp, "icon_mask.png");
        button_add_texture(entry_buttons[button_count], &textureIconBack, 25, 25, 160, 160, false, false);
        button_add_texture(entry_buttons[button_count], button_textures[button_count], 25, 25, 160, 160, false, false);
        if(!metadata)
        {
            button_add_text(entry_buttons[button_count], 200, 140, 60, FONT_COLOR_GRAY, "Missing Metadata");
        }
        else
        {
            char name_temp[0x100];
            ezxml_t metadata_xml = ezxml_parse_fp(metadata);
            strcpy(name_temp, ezxml_get(metadata_xml, "shortname_en", -1)->txt);
            
            if(font_measure_string_width_height(60, name_temp) > 500)
            {
                int count = 1;
                while(1)
                {
                    memset(name_temp, 0, 0x100);
                    strncpy(name_temp, ezxml_get(metadata_xml, "shortname_en", -1)->txt, strlen(ezxml_get(metadata_xml, "shortname_en", -1)->txt)-count++);
                    strcat(name_temp, "...");
                    
                    if(font_measure_string_width_height(60, name_temp) < 500)
                        break;
                }
            }
            
            if(!strncmp(name_temp, "Woom", 4))
                strcpy(name_temp, "Meme");
            button_add_text(entry_buttons[button_count], 200, 140, 60, FONT_COLOR_BLACK, name_temp);
            button_add_text(entry_buttons[button_count], 200, 80, 60, FONT_COLOR_GRAY, !strcmp(save_str, "common") ? "Common Savedata" : "User Savedata");
            
            ezxml_free(metadata_xml);
            fclose(metadata);
        }
        
        button_count++;
    }
    
    num_pages = (titlemanage_num_entries/6)+1;
}

void titlemanage_menu_init()
{
    if(titlemanage_browse_installed)
        titlemanage_title_browse_init();
    else
        titlemanage_title_detail_init();
    
    button_back = button_instantiate(-250.0f, -250.0f, 1.05f);
    button_add_texture(button_back, &textureDRCCornerButton, 0.0f, 0.0f, 512.0f, 512.0f,false,false);
    button_add_texture(button_back, &textureDRCBackImage, 0.0f, -40.0f, 256.0f, 256.0f,false,false);
    button_add_inflate_release_event(button_back, titlemanage_button_press_back);
    button_add_button_map(button_back, VPAD_BUTTON_B);
    button_back->x_center_scale = false;
    button_back->y_center_scale = false;
    button_back->width = 350;
    button_back->height = 250;
    button_back->selected = true;
    
    //Arrows
    button_left = button_instantiate(-32.0f, (float)(TARGET_HEIGHT / 2.0f) - (290.0f / 2.0f), 1.1f);
    button_add_texture(button_left, &textureDRCArrow, 0.0f, 0.0f, 290.0f, 290.0f,true,false);
    button_add_inflate_release_event(button_left, titlemanage_button_press_left);
    button_left->x_pos -= 200.0f;
    
    button_right = button_instantiate((float)TARGET_WIDTH + 32.0f - 290.0f, (float)(TARGET_HEIGHT / 2.0f) - (290.0f / 2.0f), 1.1f);
    button_add_texture(button_right, &textureDRCArrow, 0.0f, 0.0f, 290.0f, 290.0f,false,false);
    button_add_inflate_release_event(button_right, titlemanage_button_press_right);
}

void titlemanage_buttons_deinit()
{
    for(int i = 0; i < titlemanage_num_entries; i++)
        button_destroy(entry_buttons[i]);
}

void titlemanage_menu_deinit()
{
    titlemanage_buttons_deinit();
    free(entry_buttons);
}

void titlemanage_menu_update()
{
    if(titlemanage_title_transitioning_out && gui_transition_done())
    {
        titlemanage_title_transitioning_out = false;
        
        titlemanage_buttons_deinit();
                
        if(titlemanage_browse_installed)
            titlemanage_title_browse_init();
        else
            titlemanage_title_detail_init();
        
        gui_transition_in(1.0f, 0.0f, -60.0f, 25.0f);
    }

    if(titlemanage_transitioning_out && gui_transition_done())
    {
        titlemanage_transitioning_out = false;
        
        titlemanage_menu_deinit();
        current_state = STATE_MAIN_MENU;
        gui_transition_in(1.0f, 0.0f, 0.0f, 30.0f);
    }

    for(int i = current_page*6; i < (current_page+1)*6 && i < titlemanage_num_entries; i++)
    {
        button_update(entry_buttons[i], tpXPos, tpYPos, tpTouched);
    }
    
    if(titlemanage_transitioning_out)
    {
        button_back->y_pos -= 8.0f;
        button_back->x_pos -= 8.0f;
        //filebrowse_transition_y += 5.0f;
    }
    else
    {
        //if(filebrowse_transition_y > 0.0f)
        //    filebrowse_transition_y -= 5.0f;
            
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
    button_update(button_left, tpXPos, tpYPos, tpTouched);
    button_update(button_right, tpXPos, tpYPos, tpTouched);
    
    if(current_page <= 0 && button_left->x_pos >= -220.0f)
    {
        button_left->x_pos -= 200.0f/10.0f;
    }
    else if(button_left->x_pos < -40.0f)
    {
        button_left->x_pos += 200.0f/10.0f;
    }
        
    if(current_page >= num_pages-1 && button_right->x_pos < (float)TARGET_WIDTH + 32.0f - 290.0f + 200.0f)
    {
        button_right->x_pos += 200.0f/10.0f;
    }
    else if(button_right->x_pos > (float)TARGET_WIDTH + 32.0f - 290.0f)
    {
        button_right->x_pos -= 200.0f/10.0f;
    }
    
    //Update title button positions
    for(int i = 0; i < titlemanage_num_entries; i++)
    {
        entry_buttons[i]->x_pos = 200.0f + ((i % 2) + (i/6)*2) * 800.0f + titlemanage_transition_x_shift - (1600.0f*current_page);
    }
        
    if(titlemanage_transition)
    {
        if(titlemanage_transition_left)
        {
            titlemanage_transition_x_shift += (1600.0f*(current_page-target_page))/20.0f;
            if(titlemanage_transition_x_shift >= (1600.0f*(current_page-target_page)))
            {
                titlemanage_transition_x_shift = 0.0f;
                current_page = target_page;
                titlemanage_transition = false;
            }
        }
        else
        {
            titlemanage_transition_x_shift -= (1600.0f*(target_page-current_page))/20.0f;
            if(titlemanage_transition_x_shift <= -(1600.0f*(target_page-current_page)))
            {
                titlemanage_transition_x_shift = 0.0f;
                current_page = target_page;
                titlemanage_transition = false;
            }
        }
    }
}

void titlemanage_menu_draw(bool screen)
{
    //Draw our header
    char *header_string = "Title Management";
    if(!screen)
    {
        draw_set_use_globals(false);
        render_texture_color(&textureHeader, 0.0f, (float)(TARGET_HEIGHT - 256), (float)TARGET_WIDTH, 256.0f, NEUTRAL_COLOR);
        font_set_color(FONT_COLOR_WHITE);
        font_draw_string_height((float)(TARGET_WIDTH/2 - font_measure_string_width_height(70, header_string)/2), (float)(TARGET_HEIGHT - 70), 70, header_string);
        draw_set_use_globals(true);
    }
    else
    {
        draw_set_use_global_shifts(false);
        render_texture(&textureInfoHeader, 0.0f, (float)(TARGET_HEIGHT - 256), (float)TARGET_WIDTH, 256.0f);
        font_set_color(FONT_COLOR_WHITE);
        font_draw_string_height(64.0f, (float)(TARGET_HEIGHT - 120), 100, header_string);
        draw_set_use_global_shifts(true);
    }
    
    //Contents
    if(screen)
    {
        draw_set_use_global_shifts(false);
        render_texture_nostretch(&textureContent, 0.0f, 0.0f, 100.0f, 100.0f, 240.0f, (float)TARGET_WIDTH, (float)TARGET_HEIGHT - 150.0f);
        render_texture(&textureContentSubheader, 55.0f, (float)(TARGET_HEIGHT - 200 - 190), 1700.0f, 200.0f);
        
        font_set_color(FONT_COLOR_WHITE);
        font_draw_string_height(100.0f, (float)(TARGET_HEIGHT - 280), 100, "He has no style");
        font_set_color(FONT_COLOR_BLACK);
        font_draw_string_height(1080.0f, (float)(TARGET_HEIGHT - 470.0f), 70, "He has no grace. This");
        font_draw_string_height(1080.0f, (float)(TARGET_HEIGHT - 470.0f - 70.0f), 70, "kong has a funny face.");
        
        render_texture(&texture, 65.0f, 55.0f, 1024.0f, 1024.0f);
        draw_set_use_global_shifts(true);
    }
    else
    {
        //render_texture_nostretch(&textureDRCTitleButton, 100.0f, 100.0f, 32.0f, 32.0f, 32.0f, 600.0, 256.0);
        for(int i = 0; i < titlemanage_num_entries; i++)
        {
            button_draw(entry_buttons[i]);
        }
        
        button_draw(button_back);
        button_draw(button_left);
        button_draw(button_right);
    }
}
