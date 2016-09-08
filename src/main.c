/*
 *  Woomïnstaller GX2 - Homebrew package installer for Wii U
 *
 *  Copyright (C) 2016          SALT
 *  Copyright (C) 2016          Max Thomas (Shiny Quagsire) <mtinc2@gmail.com>
 *
 *  This code is licensed under the terms of the GNU LGPL, version 2.1
 *  see file LICENSE.md or https://www.gnu.org/licenses/lgpl-2.1.txt
 */

#include <coreinit/core.h>
#include <coreinit/debug.h>
#include <coreinit/thread.h>
#include <coreinit/foreground.h>
#include <proc_ui/procui.h>
#include <sysapp/launch.h>

#include <gx2/mem.h>
#include <gx2/draw.h>
#include <gx2/enum.h>
#include <gx2/swap.h>
#include <gx2/clear.h>
#include <gx2/event.h>
#include <gx2/state.h>
#include <gx2/texture.h>
#include <gx2/display.h>
#include <gx2/context.h>
#include <gx2/shaders.h>
#include <gx2/registers.h>

#include <vpad/input.h>
#include <unistd.h>
#include <malloc.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

#include "memory.h"
#include "draw.h"
#include "gl-matrix.h"
#include "gx2_ext.h"
#include "font.h"
#include "button.h"
#include "gui.h"
#include "particles.h"
#include "icon_menu.h"
#include "install_menu.h"

#include "ezxml.h"

bool is_app_running = true;

float box_x = 512.0f;

unsigned char *gx2CommandBuffer;
unsigned char *tvScanBuffer;
unsigned char *drcScanBuffer;
GX2ContextState *tvContextState;
GX2ContextState *drcContextState;
GX2ColorBuffer tvColorBuffer;
GX2DepthBuffer tvDepthBuffer;
GX2ColorBuffer drcColorBuffer;
GX2DepthBuffer drcDepthBuffer;

bool scene_setup = false;
bool textures_loaded = false;
bool initialized = false;

void *texture2DData;

static void setup_scene(void)
{
    if(scene_setup) return;
    
    //TODO: Better .gsh loading, all offsets and amounts are hardcoded to texture2D.gsh here
    
    vertexShader = (GX2VertexShader*) memalign(0x40, sizeof(GX2VertexShader));
    memset(vertexShader, 0, sizeof(GX2VertexShader));
    vertexShader->mode = GX2_SHADER_MODE_UNIFORM_REGISTER;
    vertexShader->size = 0x139;
    vertexShader->program = memalign(0x100, vertexShader->size);
    memcpy(vertexShader->program, (u8*)(texture2DData+0x24C), vertexShader->size);
    
    //Patch in vertex colors
    *(u32*)(vertexShader->program+(0x1C * sizeof(u8))) = 0x88048013;
    *(u32*)(vertexShader->program+(0x20 * sizeof(u8))) = 0x01C00100;
    *(u32*)(vertexShader->program+(0x24 * sizeof(u8))) = 0x88060014;
    *(u32*)(vertexShader->program+(0x28 * sizeof(u8))) = 0x26000000;
    *(u32*)(vertexShader->program+(0x2C * sizeof(u8))) = 0x000000A0;
    *(u32*)(vertexShader->program+(0x30 * sizeof(u8))) = 0x00000000;
    *(u32*)(vertexShader->program+(0x34 * sizeof(u8))) = 0x00002000;
    //End patch in vertex colors
    
    GX2Invalidate(GX2_INVALIDATE_MODE_CPU | GX2_INVALIDATE_MODE_SHADER, vertexShader->program, vertexShader->size);
    memcpy(&vertexShader->regs, (u8*)(texture2DData+0x40), sizeof(vertexShader->regs));
    
    //Patch in vertex colors
    vertexShader->regs.sq_pgm_resources_vs++;
    vertexShader->regs.spi_vs_out_config = (1 << 1); //One export, color
    vertexShader->regs.spi_vs_out_id[0] = 0xFFFF0100;
    vertexShader->regs.sq_vtx_semantic_clear = 0xFFFFFFF8;
    vertexShader->regs.num_sq_vtx_semantic++;
    vertexShader->regs.sq_vtx_semantic[2] = 2;
    //End patch in vertex colors

    vertexShader->uniformVarCount = 1;
    vertexShader->uniformVars = (GX2UniformVar*) malloc(vertexShader->uniformVarCount * sizeof(GX2UniformVar));
    vertexShader->uniformVars[0].name = "u_offset";
    vertexShader->uniformVars[0].type = GX2_SHADER_VAR_TYPE_MATRIX4X4 ;
    vertexShader->uniformVars[0].count = 1;
    vertexShader->uniformVars[0].offset = 0;
    vertexShader->uniformVars[0].block = -1;

    vertexShader->attribVarCount = 3;
    vertexShader->attribVars = (GX2AttribVar*) malloc(vertexShader->attribVarCount * sizeof(GX2AttribVar));
    vertexShader->attribVars[0].name = "a_texCoord";
    vertexShader->attribVars[0].type = GX2_SHADER_VAR_TYPE_FLOAT2;
    vertexShader->attribVars[0].count = 0;
    vertexShader->attribVars[0].location = 1;
    vertexShader->attribVars[1].name = "a_position";
    vertexShader->attribVars[1].type = GX2_SHADER_VAR_TYPE_FLOAT3;
    vertexShader->attribVars[1].count = 0;
    vertexShader->attribVars[1].location = 0;
    vertexShader->attribVars[2].name = "a_color";
    vertexShader->attribVars[2].type = GX2_SHADER_VAR_TYPE_FLOAT4;
    vertexShader->attribVars[2].count = 0;
    vertexShader->attribVars[2].location = 2;

    //Pixel shader setup
    pixelShader = (GX2PixelShader*) memalign(0x40, sizeof(GX2PixelShader));
    memset(pixelShader, 0, sizeof(GX2PixelShader));
    pixelShader->mode = GX2_SHADER_MODE_UNIFORM_REGISTER;
    pixelShader->size = 0x190;
    pixelShader->program = memalign(0x100, pixelShader->size);
    memcpy(pixelShader->program, (u8*)(texture2DData+0x518), pixelShader->size);
    
    
    //Patch in vertex colors
    memset(pixelShader->program+(0x90*sizeof(u8)), 0, 0x100);
    *(u32*)(pixelShader->program) = 0x30000000;
    
    memcpy(pixelShader->program+(0x10*sizeof(u8)), pixelShader->program+(0x8*sizeof(u8)), 0x8);
    *(u32*)(pixelShader->program+(0x8*sizeof(u8))) = 0x20000000;
    *(u32*)(pixelShader->program+(0xC*sizeof(u8))) = 0x00000CA0;
    
    memcpy(pixelShader->program+(0x180*sizeof(u8)), pixelShader->program+(0x80*sizeof(u8)), 0x10);
    memset(pixelShader->program+(0x80*sizeof(u8)), 0x0, 0x10);
    
    *(u32*)(pixelShader->program+(0x100*sizeof(u8))) = 0x00200000;
    *(u32*)(pixelShader->program+(0x104*sizeof(u8))) = 0x90000000;
    *(u32*)(pixelShader->program+(0x108*sizeof(u8))) = 0x00248000;
    *(u32*)(pixelShader->program+(0x10C*sizeof(u8))) = 0x90000020;
    *(u32*)(pixelShader->program+(0x110*sizeof(u8))) = 0x00280001;
    *(u32*)(pixelShader->program+(0x114*sizeof(u8))) = 0x90000040;
    *(u32*)(pixelShader->program+(0x118*sizeof(u8))) = 0x002C8081;
    *(u32*)(pixelShader->program+(0x11C*sizeof(u8))) = 0x90000060;
    //End patch in vertex colors
    
    GX2Invalidate(GX2_INVALIDATE_MODE_CPU | GX2_INVALIDATE_MODE_SHADER, pixelShader->program, pixelShader->size);
    memcpy(&pixelShader->regs, (u8*)(texture2DData+0x3A4), sizeof(pixelShader->regs));
    
    //Patch in vertex colors
    pixelShader->regs.sq_pgm_resources_ps++;
    pixelShader->regs.spi_ps_in_control_0 |= (1<<1);
    pixelShader->regs.num_spi_ps_input_cntl++;
    pixelShader->regs.spi_ps_input_cntls[1] = 0x00000101;
    //End patch in vertex colors

    //Attributes
    attributes = (GX2AttribStream*) malloc(sizeof(GX2AttribStream) * 3);
    GX2InitAttribStream(&attributes[0], vertexShader->attribVars[1].location, 0, 0, GX2_ATTRIB_FORMAT_FLOAT_32_32_32);
    GX2InitAttribStream(&attributes[1], vertexShader->attribVars[0].location, 1, 0, GX2_ATTRIB_FORMAT_FLOAT_32_32);
    GX2InitAttribStream(&attributes[2], vertexShader->attribVars[2].location, 2, 0, GX2_ATTRIB_FORMAT_FLOAT_32_32_32_32);

    //Fetch Shader
    u32 shaderSize = GX2CalcFetchShaderSizeEx(3, GX2_FETCH_SHADER_TESSELLATION_NONE, GX2_TESSELLATION_MODE_DISCRETE);
    fetchShaderProgramm = memalign(0x100, shaderSize);
    fetchShader = (GX2FetchShader *) malloc(sizeof(GX2FetchShader));
    GX2InitFetchShaderEx(fetchShader, fetchShaderProgramm, 3, attributes, GX2_FETCH_SHADER_TESSELLATION_NONE, GX2_TESSELLATION_MODE_DISCRETE);
    GX2Invalidate(GX2_INVALIDATE_MODE_CPU | GX2_INVALIDATE_MODE_SHADER, fetchShaderProgramm, shaderSize);
    
    //TODO: We never really unload this, no idea if it multitasks
    if(!textures_loaded)
    {   
        gui_init();
        icon_menu_init();
        //titlemanage_menu_init();
        font_init_texture();
        particles_load();
        
        textures_loaded = true;
    }
    
    scene_setup = true;
}

void takedown_scene()
{
    if(!scene_setup) return;
    
    free(vertexShader->attribVars);
    free(vertexShader->uniformVars);
    free(vertexShader->program);
    free(vertexShader);
    
    free(pixelShader->program);
    free(pixelShader);
    
    free(attributes);
    
    free(fetchShaderProgramm);
    free(fetchShader);
    
    scene_setup = false;
}

bool mem1_freed = false;
void free_MEM1_buffers()
{
    if(mem1_freed) return;
    
    MEM2_free(gx2CommandBuffer);
    MEM2_free(tvContextState);
    MEM2_free(drcContextState);
    
    if(tvColorBuffer.surface.aa)
        MEM2_free(tvColorBuffer.aaBuffer);

    if(drcColorBuffer.surface.aa)
        MEM2_free(drcColorBuffer.aaBuffer);

    MEMBucket_free(tvScanBuffer);
    MEMBucket_free(drcScanBuffer);
    
    MEM1_free(tvColorBuffer.surface.image);
    MEM1_free(tvDepthBuffer.surface.image);
    MEM1_free(tvDepthBuffer.hiZPtr);
    MEM1_free(drcColorBuffer.surface.image);
    MEM1_free(drcDepthBuffer.surface.image);
    MEM1_free(drcDepthBuffer.hiZPtr);
    mem1_freed = true;
    
    memoryRelease();
}

bool gx2_killed = false;
void kill_GX2()
{
    if(gx2_killed) return;
    
    GX2SetTVEnable(FALSE);
    GX2SetDRCEnable(FALSE);
    GX2WaitForVsync();
    GX2DrawDone();
    GX2Shutdown();
    gx2_killed = true;
}

void SaveCallback()
{
   OSSavesDone_ReadyToRelease(); // Required
}

bool app_running()
{
   if(!OSIsMainCore())
   {
      ProcUISubProcessMessages(true);
   }
   else
   {
      ProcUIStatus status = ProcUIProcessMessages(true);
    
      if(status == PROCUI_STATUS_EXITING)
      {
          // Being closed, deinit, free, and prepare to exit
          kill_GX2();
          takedown_scene();
          free_MEM1_buffers();
          
          initialized = false;
          app_is_running = false;
          
          ProcUIShutdown();
      }
      else if(status == PROCUI_STATUS_RELEASE_FOREGROUND)
      {
          OSReport("Preparing to release foreground\n");
      
          // Free up MEM1 to next foreground app, deinit screen, etc.
          kill_GX2();
          takedown_scene();
          free_MEM1_buffers();
          initialized = false;
          
          OSReport("Releasing foreground\n");
          ProcUIDrawDoneRelease();
      }
      else if(status == PROCUI_STATUS_IN_FOREGROUND)
      {
         // Executed while app is in foreground
         if(!initialized)
         {
            OSReport("Gaining foreground\n");
            memoryInitialize();
         
            //! allocate MEM2 command buffer memory
            gx2CommandBuffer = MEM2_alloc(0x400000, 0x40);

            //! initialize GX2 command buffer
            u32 gx2_init_attributes[9];
            gx2_init_attributes[0] = GX2_INIT_CMD_BUF_BASE;
            gx2_init_attributes[1] = (u32)gx2CommandBuffer;
            gx2_init_attributes[2] = GX2_INIT_CMD_BUF_POOL_SIZE;
            gx2_init_attributes[3] = 0x400000;
            gx2_init_attributes[4] = GX2_INIT_ARGC;
            gx2_init_attributes[5] = 0;
            gx2_init_attributes[6] = GX2_INIT_ARGV;
            gx2_init_attributes[7] = 0;
            gx2_init_attributes[8] = GX2_INIT_END;
            GX2Init(gx2_init_attributes);
            
            //! allocate memory and setup context state TV
            tvContextState = (GX2ContextState*)MEM2_alloc(sizeof(GX2ContextState), 0x100);
            GX2SetupContextStateEx(tvContextState, true);

            //! allocate memory and setup context state DRC
            drcContextState = (GX2ContextState*)MEM2_alloc(sizeof(GX2ContextState), 0x100);
            GX2SetupContextStateEx(drcContextState, true);

            u32 scanBufferSize = 0;
            u32 scaleNeeded = 0;

            s32 tvScanMode = GX2GetSystemTVScanMode();
            s32 drcScanMode = GX2GetSystemDRCScanMode();

            s32 tvRenderMode;
            u32 tvWidth = 0;
            u32 tvHeight = 0;

            switch(tvScanMode)
            {
            case GX2_TV_SCAN_MODE_480I:
            case GX2_TV_SCAN_MODE_480P:
                tvWidth = 854;
                tvHeight = 480;
                tvRenderMode = GX2_TV_RENDER_MODE_WIDE_480P;
                break;
            case GX2_TV_SCAN_MODE_1080I:
            case GX2_TV_SCAN_MODE_1080P:
                tvWidth = 1920;
                tvHeight = 1080;
                tvRenderMode = GX2_TV_RENDER_MODE_WIDE_1080P;
                break;
            case GX2_TV_SCAN_MODE_720P:
            default:
                tvWidth = 1280;
                tvHeight = 720;
                tvRenderMode = GX2_TV_RENDER_MODE_WIDE_720P;
                break;
            }

            s32 tvAAMode = GX2_AA_MODE1X;
            s32 drcAAMode = GX2_AA_MODE1X;
            
            u32 size, align;
            u32 surface_format = GX2_ATTRIB_FORMAT_UNORM_8_8_8_8 | 0x10;

            //Allocate scan buffer for TV
            GX2CalcTVSize(tvRenderMode, surface_format, GX2_BUFFERING_MODE_DOUBLE, &scanBufferSize, &scaleNeeded);
            tvScanBuffer = MEMBucket_alloc(scanBufferSize, 0x1000);
            GX2Invalidate(GX2_INVALIDATE_MODE_CPU, tvScanBuffer, scanBufferSize);
            GX2SetTVBuffer(tvScanBuffer, scanBufferSize, tvRenderMode, surface_format, GX2_BUFFERING_MODE_DOUBLE);

            //Allocate scan buffer for DRC
            GX2CalcDRCSize(drcScanMode, surface_format, GX2_BUFFERING_MODE_DOUBLE, &scanBufferSize, &scaleNeeded);
            drcScanBuffer = MEMBucket_alloc(scanBufferSize, 0x1000);
            GX2Invalidate(GX2_INVALIDATE_MODE_CPU, drcScanBuffer, scanBufferSize);
            GX2SetDRCBuffer(drcScanBuffer, scanBufferSize, drcScanMode, surface_format, GX2_BUFFERING_MODE_DOUBLE);

            //TV color buffer
            GX2InitColorBuffer(&tvColorBuffer, GX2_SURFACE_DIM_TEXTURE_2D, tvWidth, tvHeight, 1, surface_format, tvAAMode);
            tvColorBuffer.surface.image = MEM1_alloc(tvColorBuffer.surface.imageSize, tvColorBuffer.surface.alignment);
            GX2Invalidate(GX2_INVALIDATE_MODE_CPU, tvColorBuffer.surface.image, tvColorBuffer.surface.imageSize);

            //TV depth buffer
            GX2InitDepthBuffer(&tvDepthBuffer, GX2_SURFACE_DIM_TEXTURE_2D, tvColorBuffer.surface.width, tvColorBuffer.surface.height, 1, GX2_SURFACE_FORMAT_FLOAT_R32, tvAAMode);
            tvDepthBuffer.surface.image = MEM1_alloc(tvDepthBuffer.surface.imageSize, tvDepthBuffer.surface.alignment);
            GX2Invalidate(GX2_INVALIDATE_MODE_CPU, tvDepthBuffer.surface.image, tvDepthBuffer.surface.imageSize);

            //TV HiZ buffer
            GX2InitDepthBufferHiZEnable(&tvDepthBuffer, true);
            GX2CalcDepthBufferHiZInfo(&tvDepthBuffer, &size, &align);
            tvDepthBuffer.hiZPtr = MEM1_alloc(size, align);
            GX2Invalidate(GX2_INVALIDATE_MODE_CPU, tvDepthBuffer.hiZPtr, size);

            //DRC color buffer
            GX2InitColorBuffer(&drcColorBuffer, GX2_SURFACE_DIM_TEXTURE_2D, 854, 480, 1, surface_format, drcAAMode);
            drcColorBuffer.surface.image = MEM1_alloc(drcColorBuffer.surface.imageSize, drcColorBuffer.surface.alignment);
            GX2Invalidate(GX2_INVALIDATE_MODE_CPU, drcColorBuffer.surface.image, drcColorBuffer.surface.imageSize);

            //DRC depth buffer
            GX2InitDepthBuffer(&drcDepthBuffer, GX2_SURFACE_DIM_TEXTURE_2D, drcColorBuffer.surface.width, drcColorBuffer.surface.height, 1, GX2_SURFACE_FORMAT_FLOAT_R32, drcAAMode);
            drcDepthBuffer.surface.image = MEM1_alloc(drcDepthBuffer.surface.imageSize, drcDepthBuffer.surface.alignment);
            GX2Invalidate(GX2_INVALIDATE_MODE_CPU, drcDepthBuffer.surface.image, drcDepthBuffer.surface.imageSize);

            //DRC HiZ buffer
            GX2InitDepthBufferHiZEnable(&drcDepthBuffer, true);
            GX2CalcDepthBufferHiZInfo(&drcDepthBuffer, &size, &align);
            drcDepthBuffer.hiZPtr = MEM1_alloc(size, align);
            GX2Invalidate(GX2_INVALIDATE_MODE_CPU, drcDepthBuffer.hiZPtr, size);
            
            //Allocate anti aliasing buffers to MEM2
            if (tvColorBuffer.surface.aa)
            {
                u32 auxSize, auxAlign;
                GX2CalcColorBufferAuxInfo(&tvColorBuffer, &auxSize, &auxAlign);
                tvColorBuffer.aaBuffer = MEM2_alloc(auxSize, auxAlign);

                tvColorBuffer.aaSize = auxSize;
                memset(tvColorBuffer.aaBuffer, 0xCC, auxSize);
                GX2Invalidate(GX2_INVALIDATE_MODE_CPU, tvColorBuffer.aaBuffer, auxSize);
            }

            if (drcColorBuffer.surface.aa)
            {
                u32 auxSize, auxAlign;
                GX2CalcColorBufferAuxInfo(&drcColorBuffer, &auxSize, &auxAlign);
                drcColorBuffer.aaBuffer = MEM2_alloc(auxSize, auxAlign);
                drcColorBuffer.aaSize = auxSize;
                memset(drcColorBuffer.aaBuffer, 0xCC, auxSize);
                GX2Invalidate(GX2_INVALIDATE_MODE_CPU, drcColorBuffer.aaBuffer, auxSize );
            }
             
            //Set context and buffers
            GX2SetContextState(tvContextState);
            GX2SetColorBuffer(&tvColorBuffer, GX2_RENDER_TARGET_0);
            GX2SetDepthBuffer(&tvDepthBuffer);

            GX2SetContextState(drcContextState);
            GX2SetColorBuffer(&drcColorBuffer, GX2_RENDER_TARGET_0);
            GX2SetDepthBuffer(&drcDepthBuffer);
             
            //Load all of our shaders
            setup_scene();
             
            initialized = true;
            gx2_killed = false;
            mem1_freed = false;
         }
      }
   }

   return app_is_running;
}

void prepare_render(GX2ColorBuffer * currColorBuffer, GX2DepthBuffer * currDepthBuffer, GX2ContextState * currContextState)
{
    GX2ClearColor(currColorBuffer, 0.746f, 0.773f, 0.8125f, 1.0f);
    GX2ClearDepthStencilEx(currDepthBuffer, currDepthBuffer->depthClear, currDepthBuffer->stencilClear, GX2_CLEAR_FLAGS_DEPTH | GX2_CLEAR_FLAGS_STENCIL);

    GX2SetContextState(currContextState);
    GX2SetViewport(0.0f, 0.0f, currColorBuffer->surface.width, currColorBuffer->surface.height, 0.0f, 1.0f);
    GX2SetScissor(0, 0, currColorBuffer->surface.width, currColorBuffer->surface.height);

    GX2SetAlphaTest(true, GX2_COMPARE_FUNC_GREATER, 0.0f);
    GX2SetDepthOnlyControl(false, false, GX2_COMPARE_FUNC_NEVER);
    GX2SetColorControl(GX2_LOGIC_OP_COPY, 0xFF, false, true);
    GX2SetBlendControl(GX2_RENDER_TARGET_0, GX2_BLEND_MODE_SRC_ALPHA, GX2_BLEND_MODE_INV_SRC_ALPHA, GX2_BLEND_COMBINE_MODE_ADD, true, GX2_BLEND_MODE_SRC_ALPHA, GX2_BLEND_MODE_INV_SRC_ALPHA, GX2_BLEND_COMBINE_MODE_ADD);

    GX2SetCullOnlyControl(GX2_FRONT_FACE_CCW, false, false);
}

//This is our actual rendering code
void render(bool screen)
{
    draw_set_use_globals(false);
    render_texture(&textureBackdrop, 0.0f, 0.0f, (float)TARGET_WIDTH, (float)TARGET_HEIGHT);
    draw_set_use_globals(true);
    
    //We have to mess with depth for the particles so they don't draw over things
    GX2SetDepthOnlyControl(true, true, GX2_COMPARE_FUNC_LESS);
    particles_draw(screen);
    GX2SetDepthOnlyControl(false, false, GX2_COMPARE_FUNC_NEVER);
    
    gui_transition_update();
    
    switch(current_state)
    {
        case STATE_LOADING:
            break; //TODO: Load resources in another thread?
        case STATE_MAIN_MENU:
            icon_menu_draw(screen);
            icon_menu_update();
            break;
        case STATE_INSTALL_MENU:
            install_menu_draw(screen);
            install_menu_update();
            break;
        case STATE_FILEBROWSE_MENU:
            filebrowse_menu_draw(screen);
            filebrowse_menu_update();
            break;
        case STATE_TITLEMANAGE_MENU:
            titlemanage_menu_draw(screen);
            titlemanage_menu_update();
            break;
        default:
            break;
    }
}

int main(int argc, char **argv)
{
    ProcUIInit(&SaveCallback);
   
    //US Health and Safety
    texture2DData = read_file_to_bytes("fs:/vol/storage_mlc01/sys/title/00050010/1004E100/content/shader/texture2D.gsh");
   
    if(!texture2DData)
    {
        //JPN Health and Safety
        texture2DData = read_file_to_bytes("fs:/vol/storage_mlc01/sys/title/00050010/1004E000/content/shader/texture2D.gsh");
        if(!texture2DData)
        {
            //EUR Health and Safety
            texture2DData = read_file_to_bytes("fs:/vol/storage_mlc01/sys/title/00050010/1004E200/content/shader/texture2D.gsh");
            if(!texture2DData)
            {
                //Just do content/ I guess
                OSReport("Mount load failed, loading from content/\n");
                texture2DData = read_file_to_bytes("fs:/vol/content/texture2D.gsh");
                if(!texture2DData)
                {
                    OSFatal("Failed to load texture2D.gsh!");
                    return 0;
                }
            }
        }
    }
    
    chdir("fs:/vol/content/");
   
    font_load_data();

    int vpadError = -1;

    projectionMtx = mat4_ortho(0.0f, 1.0f*TARGET_WIDTH, 0.0f, 1.0f*TARGET_HEIGHT, 0.1f, 1000.0f, NULL);
    install_init_start_queue_manager();

    while(app_running())
    {
        if(!initialized) continue;
        
        VPADRead(0, &vpad, 1, &vpadError);
        VPADTouchData tpCalib;
        VPADGetTPCalibratedPoint(0, &tpCalib, &vpad.tpNormal);
        tpXPos = (((float)tpCalib.x / 1280.0f) * (float)TARGET_WIDTH);
	    tpYPos = (float)TARGET_HEIGHT - (((float)tpCalib.y / 720.0f) * (float)TARGET_HEIGHT);
	    tpTouched = tpCalib.touched;
        
        //Clean up from last render
        clean_tex_allocs();

        //Render DRC
        prepare_render(&drcColorBuffer, &drcDepthBuffer, drcContextState);
        render(false);
        GX2CopyColorBufferToScanBuffer(&drcColorBuffer, GX2_SCAN_TARGET_DRC);

        //Render TV
        prepare_render(&tvColorBuffer, &tvDepthBuffer, tvContextState);
        render(true);
        GX2CopyColorBufferToScanBuffer(&tvColorBuffer, GX2_SCAN_TARGET_TV);
        GX2SwapScanBuffers();
        GX2Flush();
        GX2DrawDone();
        
        GX2SetTVEnable(true);
        GX2SetDRCEnable(true);

        GX2WaitForVsync();
    }
   
   return 0;
}
