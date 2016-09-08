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

#include <wut.h>
#include <gd.h>

#include "texture.h"

void load_tga(GX2Texture *texture, void* img_data)
{
    tga_hdr *tga_header = (tga_hdr*)img_data;
    int height = ((tga_header->height & 0xFF) << 8) + ((tga_header->height & 0xFF00) >> 8);
    int width = ((tga_header->width & 0xFF) << 8) + ((tga_header->width & 0xFF00) >> 8);
    
    OSReport("Loading TGA size %ux%u\n", width, height);

    //Our one texture
    GX2InitTexture(texture, width, height, 1, 0, GX2_SURFACE_FORMAT_UNORM_R8_G8_B8_A8, GX2_SURFACE_DIM_TEXTURE_2D, GX2_TILE_MODE_LINEAR_ALIGNED);
    texture->surface.image = memalign(texture->surface.alignment, texture->surface.imageSize);
    
    if(tga_header->y_origin != 0)
    {
        for(int i = 0; i < texture->surface.imageSize/sizeof(u32); i++)
        {
            u32 *tga_bgra_data = (u32*)(img_data+sizeof(tga_hdr));
            u32 *out_data = (u32*)(texture->surface.image);
            out_data[i] = ((tga_bgra_data[i] & 0x00FF00FF)) | ((tga_bgra_data[i] & 0xFF000000) >> 16) | ((tga_bgra_data[i] & 0x0000FF00) << 16);
        }
    }
    else
    {
        u32 *tga_bgra_data = (u32*)(img_data+sizeof(tga_hdr));
        u32 *out_data = (u32*)(texture->surface.image);
        for(int x = 0; x < width; x++)
        {
            for(int y = 0; y < height; y++)
            {
                out_data[((height-y-1)*width)+x] = ((tga_bgra_data[(y*width)+x] & 0x00FF00FF)) | ((tga_bgra_data[(y*width)+x] & 0xFF000000) >> 16) | ((tga_bgra_data[(y*width)+x] & 0x0000FF00) << 16);
            }
        }
        for(int i = 0; i < texture->surface.imageSize/sizeof(u32); i++)
        {
            
            
        }
    }
    //memcpy(texture->surface.image, img_data+sizeof(tga_hdr), texture->surface.imageSize);
    GX2Invalidate(GX2_INVALIDATE_MODE_CPU | GX2_INVALIDATE_MODE_TEXTURE, texture->surface.image, texture->surface.imageSize);
}

gdImagePtr get_gdimage_from_path(char* path)
{
    // load our image
    u8 *img_data = read_file_to_bytes(path);
    if(!img_data) 
        return NULL;
    u32 img_data_size = malloc_usable_size(img_data);
    
    // determine image type and create gdImage
    gdImagePtr gd_img = NULL;
    // BMP
    if(img_data[0] == 'B' && img_data[1] == 'M')
    {
        OSReport("Loading BMP...\n");
        gd_img = gdImageCreateFromBmpPtr(img_data_size, img_data);
    }
    // PNG
    else if(*(u32*)img_data == 0x89504E47)
    {
        OSReport("Loading PNG...\n");
        gd_img = gdImageCreateFromPngPtr(img_data_size, img_data);
    }
    // JPG (for the love of god please don't use JPG textures)
    else if(*(u16*)img_data == 0xFFD8)
    {
        OSReport("Loading JPG...\n");
        gd_img = gdImageCreateFromJpegPtr(img_data_size, img_data);
    }
    // try tga last (no good magic, but will fail on invalid data)
    else if(img_data[0] == 0x00)
    {
        OSReport("Loading TGA...\n");
        gd_img = gdImageCreateFromTgaPtr(img_data_size, img_data);
    }
    free(img_data);
    
    if(gd_img == NULL)
        return NULL;
    return gd_img;
}

void gd_image_mask_to_unorm_R8G8B8A8(gdImagePtr gdImg, gdImagePtr gdMask, u32 *img_buffer, u32 width, u32 height, u32 pitch)
{
    for(u32 y = 0; y < height; ++y)
    {
        for(u32 x = 0; x < width; ++x)
        {
			u32 pixel = gdImageGetPixel(gdImg, x, y);

			u8 a = 254 - 2*((u8)gdImageAlpha(gdImg, pixel));
			if(a == 254) a++;

            u8 r = gdImageRed(gdImg, pixel);
            u8 g = gdImageGreen(gdImg, pixel);
            u8 b = gdImageBlue(gdImg, pixel);
            
            u8 ma = gdImageRed(gdMask, gdImageGetPixel(gdMask, x, y));
            a = (u8)(a * (ma / 255));

            img_buffer[y * pitch + x] = (r << 24) | (g << 16) | (b << 8) | (a);
        }
    }
}

void gd_image_to_unorm_R8G8B8A8(gdImagePtr gdImg, u32 *img_buffer, u32 width, u32 height, u32 pitch)
{
    for(u32 y = 0; y < height; ++y)
    {
        for(u32 x = 0; x < width; ++x)
        {
			u32 pixel = gdImageGetPixel(gdImg, x, y);

			u8 a = 254 - 2*((u8)gdImageAlpha(gdImg, pixel));
			if(a == 254) a++;

            u8 r = gdImageRed(gdImg, pixel);
            u8 g = gdImageGreen(gdImg, pixel);
            u8 b = gdImageBlue(gdImg, pixel);

            img_buffer[y * pitch + x] = (r << 24) | (g << 16) | (b << 8) | (a);
        }
    }
}


bool load_img_texture(GX2Texture *texture, char *path)
{
    OSReport("Loading image %s...\n", path);
    
    
    gdImagePtr img = get_gdimage_from_path(path);
    if(img == NULL)
    {
        OSReport("Failed to load image file!\n");
        return false;
    }
    
    u32 height = gdImageSY(img);
    u32 width = gdImageSX(img);
    
    // create a texture
    GX2InitTexture(texture, width, height, 1, 0, GX2_SURFACE_FORMAT_UNORM_R8_G8_B8_A8, GX2_SURFACE_DIM_TEXTURE_2D, GX2_TILE_MODE_LINEAR_ALIGNED);
    
    // sanity-check texture creation
    if(texture->surface.imageSize == 0)
    {
        gdImageDestroy(img);
        return false;
    }
    // allocate memory for texture data
    texture->surface.image = memalign(texture->surface.alignment, texture->surface.imageSize);
    // verify alloc
    if(texture->surface.image == NULL)
    {
        gdImageDestroy(img);
        return false;
    }
    // Get raw image data onto our texture
    gd_image_to_unorm_R8G8B8A8(img, texture->surface.image, texture->surface.width, texture->surface.height, texture->surface.pitch);
    gdImageDestroy(img);
    GX2Invalidate(GX2_INVALIDATE_MODE_CPU | GX2_INVALIDATE_MODE_TEXTURE, texture->surface.image, texture->surface.imageSize);
    return true;
}

bool load_img_texture_mask(GX2Texture *texture, char *path, char *mask)
{
    OSReport("Loading image %s...\n", path);
    
    
    gdImagePtr img = get_gdimage_from_path(path);
    if(img == NULL)
    {
        OSReport("Failed to load image file!\n");
        return false;
    }
    
    gdImagePtr img_mask = get_gdimage_from_path(mask);
    if(img_mask == NULL)
    {
        OSReport("Failed to load image file!\n");
        return false;
    }
    
    u32 height = gdImageSY(img);
    u32 width = gdImageSX(img);
    
    // create a texture
    GX2InitTexture(texture, width, height, 1, 0, GX2_SURFACE_FORMAT_UNORM_R8_G8_B8_A8, GX2_SURFACE_DIM_TEXTURE_2D, GX2_TILE_MODE_LINEAR_ALIGNED);
    
    // sanity-check texture creation
    if(texture->surface.imageSize == 0)
    {
        gdImageDestroy(img);
        return false;
    }
    // allocate memory for texture data
    texture->surface.image = memalign(texture->surface.alignment, texture->surface.imageSize);
    // verify alloc
    if(texture->surface.image == NULL)
    {
        gdImageDestroy(img);
        return false;
    }
    // Get raw image data onto our texture
    gd_image_mask_to_unorm_R8G8B8A8(img, img_mask, texture->surface.image, texture->surface.width, texture->surface.height, texture->surface.pitch);
    gdImageDestroy(img_mask);
    gdImageDestroy(img);
    GX2Invalidate(GX2_INVALIDATE_MODE_CPU | GX2_INVALIDATE_MODE_TEXTURE, texture->surface.image, texture->surface.imageSize);
    return true;
}


void free_img_texture(GX2Texture *texture)
{
    free(texture->surface.image);
    texture->surface.image = NULL;
}
