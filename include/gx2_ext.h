#ifndef __GX2_EXT_H_
#define __GX2_EXT_H_

#define GX2_COMP_SEL_NONE                               0x04040405
#define GX2_COMP_SEL_X001                               0x00040405
#define GX2_COMP_SEL_XY01                               0x00010405
#define GX2_COMP_SEL_XYZ1                               0x00010205
#define GX2_COMP_SEL_XYZW                               0x00010203
#define GX2_COMP_SEL_XXXX                               0x00000000
#define GX2_COMP_SEL_YYYY                               0x01010101
#define GX2_COMP_SEL_ZZZZ                               0x02020202
#define GX2_COMP_SEL_WWWW                               0x03030303
#define GX2_COMP_SEL_WZYX                               0x03020100
#define GX2_COMP_SEL_WXYZ                               0x03000102

static const u32 attribute_dest_comp_selector[20] = {
    GX2_COMP_SEL_X001, GX2_COMP_SEL_XY01, GX2_COMP_SEL_X001, GX2_COMP_SEL_X001,  GX2_COMP_SEL_XY01, GX2_COMP_SEL_X001,
    GX2_COMP_SEL_X001, GX2_COMP_SEL_XY01, GX2_COMP_SEL_XY01, GX2_COMP_SEL_XYZ1, GX2_COMP_SEL_XYZW, GX2_COMP_SEL_XYZW,
    GX2_COMP_SEL_XY01, GX2_COMP_SEL_XY01, GX2_COMP_SEL_XYZW, GX2_COMP_SEL_XYZW, GX2_COMP_SEL_XYZ1, GX2_COMP_SEL_XYZ1,
    GX2_COMP_SEL_XYZW, GX2_COMP_SEL_XYZW
};

static const u32 texture_comp_selector[54] = {
    GX2_COMP_SEL_NONE, GX2_COMP_SEL_X001, GX2_COMP_SEL_XY01, GX2_COMP_SEL_NONE, GX2_COMP_SEL_NONE, GX2_COMP_SEL_X001,
    GX2_COMP_SEL_X001, GX2_COMP_SEL_XY01, GX2_COMP_SEL_XYZ1, GX2_COMP_SEL_XYZ1, GX2_COMP_SEL_XYZW, GX2_COMP_SEL_XYZW,
    GX2_COMP_SEL_WZYX, GX2_COMP_SEL_X001, GX2_COMP_SEL_X001, GX2_COMP_SEL_XY01, GX2_COMP_SEL_XY01, GX2_COMP_SEL_NONE,
    GX2_COMP_SEL_NONE, GX2_COMP_SEL_NONE, GX2_COMP_SEL_NONE, GX2_COMP_SEL_NONE, GX2_COMP_SEL_XYZ1, GX2_COMP_SEL_NONE,
    GX2_COMP_SEL_NONE, GX2_COMP_SEL_XYZW, GX2_COMP_SEL_XYZW, GX2_COMP_SEL_WZYX, GX2_COMP_SEL_XY01, GX2_COMP_SEL_XY01,
    GX2_COMP_SEL_XY01, GX2_COMP_SEL_XYZW, GX2_COMP_SEL_XYZW, GX2_COMP_SEL_NONE, GX2_COMP_SEL_XYZW, GX2_COMP_SEL_XYZW,
    GX2_COMP_SEL_NONE, GX2_COMP_SEL_NONE, GX2_COMP_SEL_NONE, GX2_COMP_SEL_XYZ1, GX2_COMP_SEL_XYZ1, GX2_COMP_SEL_X001,
    GX2_COMP_SEL_XY01, GX2_COMP_SEL_XYZ1, GX2_COMP_SEL_NONE, GX2_COMP_SEL_NONE, GX2_COMP_SEL_NONE, GX2_COMP_SEL_XYZ1,
    GX2_COMP_SEL_XYZ1, GX2_COMP_SEL_XYZW, GX2_COMP_SEL_XYZW, GX2_COMP_SEL_XYZW, GX2_COMP_SEL_X001, GX2_COMP_SEL_XY01
};

static inline void GX2InitAttribStream(GX2AttribStream* attr, u32 location, u32 buffer, u32 offset, s32 format)
{
    attr->location = location;
    attr->buffer = buffer;
    attr->offset = offset;
    attr->format = format;
    attr->type = 0;
    attr->aluDivisor = 0;
    attr->mask = attribute_dest_comp_selector[format & 0xff]; //TODO: eww
    attr->endianSwap  = GX2_ENDIAN_SWAP_DEFAULT;
}

static inline void GX2InitDepthBuffer(GX2DepthBuffer *depthBuffer, s32 dim, u32 width, u32 height, u32 depth, s32 format, s32 aa)
{
    depthBuffer->surface.dim = dim;
    depthBuffer->surface.width = width;
    depthBuffer->surface.height = height;
    depthBuffer->surface.depth = depth;
    depthBuffer->surface.mipLevels = 1;
    depthBuffer->surface.format = format;
    depthBuffer->surface.aa = aa;
    depthBuffer->surface.use = ((format==GX2_SURFACE_FORMAT_UNORM_R24_X8) || (format==GX2_SURFACE_FORMAT_FLOAT_D24_S8)) ? GX2_SURFACE_USE_DEPTH_BUFFER : (GX2_SURFACE_USE_DEPTH_BUFFER | GX2_SURFACE_USE_TEXTURE);
    depthBuffer->surface.tileMode = GX2_TILE_MODE_DEFAULT;
    depthBuffer->surface.swizzle  = 0;
    depthBuffer->viewMip = 0;
    depthBuffer->viewFirstSlice = 0;
    depthBuffer->viewNumSlices = depth;
    depthBuffer->depthClear = 1.0f;
    depthBuffer->stencilClear = 0;
    depthBuffer->hiZPtr = NULL;
    depthBuffer->hiZSize = 0;
    GX2CalcSurfaceSizeAndAlignment(&depthBuffer->surface);
    GX2InitDepthBufferRegs(depthBuffer);
}

static inline void GX2InitColorBuffer(GX2ColorBuffer *colorBuffer, s32 dim, u32 width, u32 height, u32 depth, s32 format, s32 aa)
{
    colorBuffer->surface.dim = dim;
    colorBuffer->surface.width = width;
    colorBuffer->surface.height = height;
    colorBuffer->surface.depth = depth;
    colorBuffer->surface.mipLevels = 1;
    colorBuffer->surface.format = format;
    colorBuffer->surface.aa = aa;
    colorBuffer->surface.use = GX2_SURFACE_USE_COLOR_BUFFER | 0x80000000;
    colorBuffer->surface.imageSize = 0;
    colorBuffer->surface.image = NULL;
    colorBuffer->surface.mipmapSize = 0;
    colorBuffer->surface.mipmaps = NULL;
    colorBuffer->surface.tileMode = GX2_TILE_MODE_DEFAULT;
    colorBuffer->surface.swizzle = 0;
    colorBuffer->surface.alignment = 0;
    colorBuffer->surface.pitch = 0;
    u32 i;
    for(i = 0; i < 13; i++)
        colorBuffer->surface.mipLevelOffset[i] = 0;
    colorBuffer->viewMip = 0;
    colorBuffer->viewFirstSlice = 0;
    colorBuffer->viewNumSlices = depth;
    colorBuffer->aaBuffer = NULL;
    colorBuffer->aaSize = 0;
    for(i = 0; i < 5; i++)
        colorBuffer->regs[i] = 0;

    GX2CalcSurfaceSizeAndAlignment(&colorBuffer->surface);
    GX2InitColorBufferRegs(colorBuffer);
}

static inline void GX2InitTexture(GX2Texture *tex, u32 width, u32 height, u32 depth, u32 mipLevels, s32 format, s32 dim, s32 tile)
{
    tex->surface.dim = dim;
    tex->surface.width = width;
    tex->surface.height = height;
    tex->surface.depth = depth;
    tex->surface.mipLevels = mipLevels;
    tex->surface.format = format;
    tex->surface.aa = GX2_AA_MODE1X;
    tex->surface.use = GX2_SURFACE_USE_TEXTURE;
    tex->surface.imageSize = 0;
    tex->surface.image = NULL;
    tex->surface.mipmapSize = 0;
    tex->surface.mipmaps = NULL;
    tex->surface.tileMode = tile;
    tex->surface.swizzle = 0;
    tex->surface.alignment = 0;
    tex->surface.pitch = 0;
    u32 i;
    for(i = 0; i < 13; i++)
        tex->surface.mipLevelOffset[i] = 0;
    tex->viewFirstMip = 0;
    tex->viewNumMips = mipLevels;
    tex->viewFirstSlice = 0;
    tex->viewNumSlices = depth;
    tex->compMap = texture_comp_selector[format & 0x3f]; //TODO: eww
    for(i = 0; i < 5; i++)
        tex->regs[i] = 0;

    GX2CalcSurfaceSizeAndAlignment(&tex->surface);
    GX2InitTextureRegs(tex);
}

#endif
