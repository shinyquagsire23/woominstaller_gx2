/*
 *  Woomïnstaller GX2 - Homebrew package installer for Wii U
 *
 *  Copyright (C) 2016          SALT
 *  Copyright (C) 2016          Max Thomas (Shiny Quagsire) <mtinc2@gmail.com>
 *
 *  This code is licensed under the terms of the GNU LGPL, version 2.1
 *  see file LICENSE.md or https://www.gnu.org/licenses/lgpl-2.1.txt
 */

#ifndef FILEBROWSE_MENU_H
#define FILEBROWSE_MENU_H

#include "common.h"

#include <gx2/texture.h>

#include "draw.h"
#include "font.h"
#include "gui.h"
#include "button.h"

void filebrowse_menu_init();
void filebrowse_menu_deinit();
void filebrowse_menu_update();
void filebrowse_menu_draw(bool screen);

#endif
