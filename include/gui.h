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

#ifndef GUI_H
#define GUI_H

#include <wut.h>
#include <memory.h>
#include <gx2/texture.h>
#include <vpad/input.h>

#include "draw.h"
#include "font.h"
#include "texture.h"

enum available_states
{
    STATE_LOADING = 0,
    STATE_MAIN_MENU,
    STATE_INSTALL_MENU,
    STATE_FILEBROWSE_MENU,
    STATE_TITLEMANAGE_MENU,
};

#define NEUTRAL_COLOR 0.5f, 0.5f, 0.5f, 1.0f
#define INSTALL_COLOR 0.192f, 0.404f, 0.737f, 1.0f

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
GX2Texture textureProgressBar;
GX2Texture textureDRCArrow;
GX2Texture textureDRCButton;
GX2Texture textureDRCInstall;
GX2Texture textureDRCShadow;
GX2Texture textureDRCMeme;
GX2Texture textureDRCCornerButton;
GX2Texture textureDRCBackImage;
GX2Texture textureDRCTitleButton;
GX2Texture fontTexture;

//State variables
VPADStatus vpad;
float tpXPos;
float tpYPos;
bool tpTouched;
bool app_is_running;

int current_state;

void gui_init();
void gui_transition_update();
void gui_transition_in(float start_scale, float start_xshift, float start_yshift, float frames);
void gui_transition_out(float end_scale, float end_xshift, float end_yshift, float frames);
bool gui_transition_done();

#endif
