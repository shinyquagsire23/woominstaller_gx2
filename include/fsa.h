/*
 *  Woomïnstaller GX2 - Homebrew package installer for Wii U
 *
 *  Copyright (C) 2016          SALT
 *  Copyright (C) 2016          WulfyStylez
 *
 *  This code is licensed under the terms of the GNU LGPL, version 2.1
 *  see file LICENSE.md or https://www.gnu.org/licenses/lgpl-2.1.txt
 */

#ifndef FSA_H
#define FSA_H
#include "common.h"


typedef struct
{
    u32 flags;      // 0x0
    u32 permissions;// 0x4
	u32 unk1[0x2];  // 0x8
	u32 size;       // 0x10 size in bytes
	u32 physsize;   // 0x14 physical size on disk in bytes
	u32 unk2[0x13];
}fileStat_s;

typedef struct
{
	fileStat_s dirStat;
	char name[0x100];
}directoryEntry_s;

typedef enum
{
    MEDIA_READY = 0,
    MEDIA_NOT_PRESENT = 1,
    MEDIA_INVALID = 2,
    MEDIA_DIRTY = 3,
    MEDIA_FATAL = 4
} media_states;

typedef struct
{
    u32 flags;
    u32 media_state;
    s32 device_index;
    u32 block_size;
    u32 logical_block_size;
    u32 read_align;
    u32 write_align;
    char dev_type[8]; // sdcard, slc, mlc, etc
    char fs_type[8]; // isfs, wfs, fat, etc
    char vol_label[0x80];
    char vol_id[0x80];
    char dev_node[0x10];
    char mount_path[0x80];
} fsa_volume_info;

#define FSA_MOUNTFLAGS_BINDMOUNT (1 << 0)
#define FSA_MOUNTFLAGS_GLOBAL (1 << 1)

int FSA_Init();
int FSA_Exit();

int FSA_Mount(char* device_path, char* volume_path, u32 flags, char* arg_string, int arg_string_len);
int FSA_Unmount(char* path, u32 flags);

int FSA_GetDeviceInfo(char* device_path, int type, u32* out_data);
int FSA_GetVolumeInfo(char* volume_path, int type, fsa_volume_info* out_data);

int FSA_MakeDir(char* path, u32 flags);
int FSA_OpenDir(char* path, int* outHandle);
int FSA_ReadDir(int handle, directoryEntry_s* out_data);
int FSA_CloseDir(int handle);

int FSA_OpenFile(char* path, char* mode, int* outHandle);
int FSA_ReadFile(void* data, u32 size, u32 cnt, int fileHandle, u32 flags);
int FSA_WriteFile(void* data, u32 size, u32 cnt, int fileHandle, u32 flags);
int FSA_StatFile(int handle, fileStat_s* out_data);
int FSA_CloseFile(int fileHandle);

int FSA_ChangeMode(char* path, int mode);

int FSA_Format(char* device, char* fs_format, int flags, u32 what1, u32 what2);

int FSA_RawOpen(char* device_path, int* outHandle);
int FSA_RawRead(void* data, u32 size_bytes, u32 cnt, u64 sector_offset, int device_handle);
int FSA_RawWrite(void* data, u32 size_bytes, u32 cnt, u64 sector_offset, int device_handle);
int FSA_RawClose(int device_handle);

#endif
