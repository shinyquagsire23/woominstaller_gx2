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

#ifndef TEXTURE_H
#define TEXTURE_H

#include <wut_types.h>
#include <coreinit/debug.h>

#include <stdio.h>

#include <gx2/texture.h>
#include <gx2/mem.h>
#include <gx2/enum.h>
#include <gx2/shaders.h>
#include "gx2_ext.h"

#include "memory.h"

typedef struct tga_hdr tga_hdr;
struct __attribute__((__packed__)) tga_hdr
{
    u8 idlength;
    u8 colormaptype;
    u8 datatype;
    u16 colormaporigin;
    u16 colormaplength;
    u8 colormapdepth;
    u16 x_origin;
    u16 y_origin;
    u16 width;
    u16 height;
    u8 bpp;
    u8 imagedescriptor;
};

void load_tga(GX2Texture *texture, void* img_data);
bool load_img_texture(GX2Texture *texture, char *path);
bool load_img_texture_mask(GX2Texture *texture, char *path, char *mask);
void free_img_texture(GX2Texture *texture);

#endif
