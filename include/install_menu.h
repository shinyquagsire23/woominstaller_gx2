/*
 *  Woomïnstaller GX2 - Homebrew package installer for Wii U
 *
 *  Copyright (C) 2016          SALT
 *  Copyright (C) 2016          Max Thomas (Shiny Quagsire) <mtinc2@gmail.com>
 *
 *  This code is licensed under the terms of the GNU LGPL, version 2.1
 *  see file LICENSE.md or https://www.gnu.org/licenses/lgpl-2.1.txt
 */

#ifndef INSTALL_MENU_H
#define INSTALL_MENU_H

#include <wut.h>
#include <memory.h>
#include <math.h>
#include <gx2/texture.h>

#include <dirent.h>
#include <stdio.h>

#include "draw.h"
#include "font.h"
#include "gui.h"
#include "button.h"
#include "install.h"

void install_menu_init();
void install_menu_deinit();
void install_menu_update();
void install_menu_draw(bool screen);

#endif
