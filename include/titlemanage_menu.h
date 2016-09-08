/*
 *  Woomïnstaller GX2 - Homebrew package installer for Wii U
 *
 *  Copyright (C) 2016          SALT
 *  Copyright (C) 2016          Max Thomas (Shiny Quagsire) <mtinc2@gmail.com>
 *
 *  This code is licensed under the terms of the GNU LGPL, version 2.1
 *  see file LICENSE.md or https://www.gnu.org/licenses/lgpl-2.1.txt
 */

#ifndef TITLEMANAGE_MENU_H
#define TITLEMANAGE_MENU_H

#include "common.h"

#include "draw.h"
#include "font.h"
#include "gui.h"
#include "button.h"

void titlemanage_menu_init();
void titlemanage_menu_update();
void titlemanage_menu_draw(bool screen);

#endif
