/*
 *  Woomïnstaller GX2 - Homebrew package installer for Wii U
 *
 *  Copyright (C) 2016          SALT
 *  Copyright (C) 2016          Max Thomas (Shiny Quagsire) <mtinc2@gmail.com>
 *
 *  This code is licensed under the terms of the GNU LGPL, version 2.1
 *  see file LICENSE.md or https://www.gnu.org/licenses/lgpl-2.1.txt
 */

#ifndef ICON_MENU_H
#define ICON_MENU_H

#include <wut.h>
#include <memory.h>
#include <math.h>
#include <coreinit/debug.h>
#include <gx2/texture.h>

#include "draw.h"
#include "font.h"
#include "gui.h"
#include "button.h"
#include "install_menu.h"
#include "filebrowse_menu.h"

typedef struct menu_index_t
{
    GX2Texture *icon;
    button_t *big_button;
    int num_texts;
    char **texts;
} menu_index_t;

void icon_menu_init();
void icon_menu_update();
void icon_menu_draw(bool screen);
void icon_menu_draw_icons();

#endif
