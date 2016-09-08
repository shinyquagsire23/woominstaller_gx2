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
 
#include "install.h"

//Install target state
int selectedInstallTarget = 0;
int numDevices = 0;
int numInstallDevices = 0;
MCPDeviceList *devicelist;
InstallDevice installDevices[32];

//Install queue state
InstallQueueItem installItems[INSTALL_QUEUE_SIZE];
char *currentlyInstalling;
bool installing = false;
MCPInstallProgress *mcp_prog_buf;
int mcp_handle;

//woomy install state
mz_zip_archive woomy_archive = {0};
ezxml_t woomy_xml;
int woomy_install_index = 0;
int woomy_extract_prog = 0;
int woomy_extract_total = 0;
bool woomy_processing = false;
bool woomy_extracting = false;

char woomy_entry_name[0x200];

GX2Texture textures[INSTALL_QUEUE_SIZE];

void clear_dir(char *path)
{
    DIR *dir;
    struct dirent *ent;
    
    OSReport("Clearing %s\n", path);
    
    dir = opendir(path);
    if(dir == NULL)
        return;
        
    OSReport("Opened %s\n", path);

    while((ent = readdir(dir)))
    {
        char temp_buf[0x200];
        sprintf(temp_buf, "%s%s", path, ent->d_name);
        remove(temp_buf);
        OSReport("Removing %s\n", temp_buf);
    }
    
    closedir(dir);
    
    remove(path);
}


int install_queue_thread(int argc, const char **argv)
{
    //We want that priority for unpacking
    OSSetThreadPriority(OSGetCurrentThread(), 0);

    int mcp_thread_handle = MCP_Open();
    MCPInstallProgress *mcp_install = memalign(0x1000, 0x24);
    void *mcp_info_buf = memalign(0x1000, 0x27F);
    void *mcp_install_buf = memalign(0x1000, 0x27F);

    OSReport("Install queue process thread started.\n");
    while(app_is_running)
    {
        //Don't completely murder IOS with progress requests here, 
        //MCP crashes if you ping it too much apparently.
        OSSleepTicks(50000000);
        
        int ret = MCP_InstallGetProgress(mcp_thread_handle, mcp_install);
        OSReport("Install Progress: %u overall:%llu/%llu contents:%u/%u\n", mcp_install->inProgress, mcp_install->sizeProgress, mcp_install->sizeTotal, mcp_install->contentsProgress, mcp_install->contentsTotal);
        
        if(!woomy_entry_name[0] && mcp_install->inProgress)
            sprintf(woomy_entry_name, "%016llX", mcp_install->tid);
            
        //Fake info for decaf
        if(ret == 0x12345678)
        {
            mcp_install->inProgress = false;//installItems[0].installing;
            mcp_install->sizeProgress = 5;
            mcp_install->sizeTotal = 11;
            mcp_install->contentsProgress = 17;
            mcp_install->contentsTotal = 30;
        }
        
        if(mcp_install->inProgress)
        {
            installItems[0].current_content = mcp_install->contentsProgress;
            installItems[0].total_content = mcp_install->contentsTotal;
            installItems[0].percent_complete = (float)mcp_install->sizeProgress / (float)mcp_install->sizeTotal;
        }
        
        if(!mcp_install->inProgress)
        {
            installing = false;
            if(installItems[0].installing && !installItems[0].woomy)
                install_shift_back_queue();
            
            //Free queue allocations
            if(currentlyInstalling != NULL)
            {
                free(currentlyInstalling);
                currentlyInstalling = NULL;
            }
            
            if(installItems[0].targetPath == NULL)
                continue;
                
            char *to_install = installItems[0].targetPath;
            installItems[0].percent_complete = 0.0f;
            
            //If it's a file, attempt to process it
            if(to_install[strlen(to_install)-1] != '/')
            {
                if(!woomy_processing)
                {
                    woomy_extracting = false;
                    if(mz_zip_reader_init_file(&woomy_archive, to_install, 0))
                    {
                        void *meta_buf = malloc(0x8000);
                        int status = mz_zip_reader_extract_file_to_mem(&woomy_archive, "metadata.xml", meta_buf, 0x8000, 0);
                        if(!status)
                        {
                            OSReport("Install for %s failed, missing metadata.xml\n", to_install);
                            install_shift_back_queue();
                            continue;
                        }
                        
                        woomy_xml = ezxml_parse_str(meta_buf, strlen(meta_buf));
                    }
                    else
                    {
                        OSReport("Install for %s failed\n", to_install);
                        install_shift_back_queue();
                        continue;
                    }
                    
                    woomy_install_index = 0;
                    woomy_processing = true;
                }
                
                woomy_extract_prog = 0;
                woomy_extract_total = 0;
                ezxml_t entries = ezxml_get(woomy_xml, "entries", 0, "entry", -1);
                ezxml_t next_entry = ezxml_idx(entries, woomy_install_index++);
                if(next_entry)
                {
                    woomy_extract_total = (u32)(strtoul(ezxml_attr(next_entry, "entries"), NULL, 10) & 0xFFFFFFFF);
                    installItems[0].woomy = true;
                    installItems[0].pre_install = true;
                    installItems[0].total_unpack = woomy_extract_total;
                    
                    if(ezxml_attr(next_entry, "name"))
                        strcpy(woomy_entry_name, (char*)ezxml_attr(next_entry, "name"));
                    else
                        strcpy(woomy_entry_name, "<no name>");
                        
                    OSReport("Installing woomy entry '%s' from '%s'\n", woomy_entry_name, ezxml_attr(next_entry, "folder"));
                    
                    //TODO: tmp to mlc or elsewhere?
                    clear_dir("/vol/external01/tmp/");
                    mkdir("/vol/external01/tmp/", 0x666);
                    
                    woomy_extracting = true;
                    char *temp_tmp_filename = malloc(0x200);
                    
                    for (int i = 0; i < (int)mz_zip_reader_get_num_files(&woomy_archive); i++)
                    {
                        mz_zip_archive_file_stat file_stat;
                        if (!mz_zip_reader_file_stat(&woomy_archive, i, &file_stat))
                        {
                            OSReport("mz_zip_reader_file_stat() failed!\n");
                            continue;
                        }

                        if (!strncmp(file_stat.m_filename, ezxml_attr(next_entry, "folder"), strlen(ezxml_attr(next_entry, "folder"))) && !mz_zip_reader_is_file_a_directory(&woomy_archive, i))
                        {
                            OSReport("Extracting '%s' (Comment: \"%s\", Uncompressed size: %u, Compressed size: %u)\n", file_stat.m_filename, file_stat.m_comment, (u32)file_stat.m_uncomp_size, (u32)file_stat.m_comp_size);
                            
                            snprintf(temp_tmp_filename, 0x200, "/vol/external01/tmp/%s", file_stat.m_filename + strlen(ezxml_attr(next_entry, "folder")));
                            OSReport("%s\n", temp_tmp_filename);
                            mz_zip_reader_extract_file_to_file(&woomy_archive, file_stat.m_filename, temp_tmp_filename, 0);
                            
                            //TODO: Maybe use a callback wrapper to show progress?
                            
                            char *ext = strchr(file_stat.m_filename, '.');
                            if(ext && !strcmp(ext, ".app"))
                                woomy_extract_prog++;
                                
                            installItems[0].current_unpack = woomy_extract_prog;
                        }
                    }

                    free(temp_tmp_filename);
                    
                    to_install = malloc(0x200);
                    snprintf(to_install, 0x200, "/vol/app_sd/tmp/");
                }
                else
                {
                    OSReport("Exhausted entries from '%s', advancing install queue.\n", to_install);
                    
                    mz_zip_reader_end(&woomy_archive);
                    ezxml_free(woomy_xml);
                    
                    woomy_entry_name[0] = '\x00';
                    
                    //TODO: tmp to mlc or elsewhere?
                    clear_dir("/vol/external01/tmp/");
                    
                    woomy_extracting = false;
                    woomy_processing = false;
                    install_shift_back_queue();
                    continue;
                }
                
                woomy_extracting = false;
            }
            else
            {
                installItems[0].woomy = false;
                woomy_entry_name[0] = '\x00';
            }
            
            installItems[0].pre_install = false;
            
            if(installDevices[installItems[0].targetDevice].deviceID == MCP_INSTALL_TARGET_USB)
            {
                if(MCP_InstallSetTargetUsb(mcp_thread_handle, installDevices[installItems[0].targetDevice].deviceNum) < 0)
                {
                    OSReport("Install for %s failed, failed to set install target to USB\n", installItems[0].targetPath);
                    install_shift_back_queue();
                    continue;
                }
            }
        
            if(MCP_InstallSetTargetDevice(mcp_thread_handle, installDevices[installItems[0].targetDevice].deviceID) >= 0)
            {
                OSReport("Set install target to %s%02u (device ID 0x%x)\n", installDevices[installItems[0].targetDevice].deviceName, installDevices[installItems[0].targetDevice].deviceNum, installDevices[installItems[0].targetDevice].deviceID);
                if(MCP_InstallGetInfo(mcp_thread_handle, to_install, mcp_info_buf) >= 0)
                {
                    if(MCP_InstallTitleAsync(mcp_thread_handle, to_install, mcp_install_buf) >= 0)
                    {
                        installing = true;
                        installItems[0].installing = true;
                        
                        currentlyInstalling = to_install;
                        
                        OSReport("Installing %s\n", currentlyInstalling);
                        continue;
                    }
                }
            }
            
            //Install failed for some reason, remove item from queue
            OSReport("Install for %s failed\n", installItems[0].targetPath);
            install_shift_back_queue();
        }
    }
    
    free(mcp_info_buf);
    free(mcp_install_buf);
    MCP_Close(mcp_thread_handle);
    return 0;
}

void install_shift_back_queue()
{
    if(textures[0].surface.image)
        free_img_texture(&textures[0]);
        
    woomy_entry_name[0] = '\x00';

    for(int i = 1; i < INSTALL_QUEUE_SIZE; i++)
    {
        installItems[i-1].pre_install = false;
        installItems[i-1].installing = false;
        installItems[i-1].woomy = false;
        installItems[i-1].percent_complete = 0.0f;
        installItems[i-1].current_unpack = 0;
        installItems[i-1].total_unpack = 0;
        installItems[i-1].current_content = 0;
        installItems[i-1].total_content = 0;
        installItems[i-1].archiveName = installItems[i].archiveName;
        installItems[i-1].targetPath = installItems[i].targetPath;
        installItems[i-1].targetDevice = installItems[i].targetDevice;
        textures[i-1] = textures[i];
        if(installItems[i].targetPath == NULL)
            break;
    }
}

void install_add_to_queue(char *path)
{
    OSReport("Attempting to add %s to the install queue...\n", path);
    if(currentlyInstalling != NULL)
    {
        if(!strcmp(currentlyInstalling, path))
        {
            OSReport("%s is being installed", path);
            return;
        }
    }
        
        
    for(int i = 0; i < INSTALL_QUEUE_SIZE; i++)
    {
        if(installItems[i].targetPath != NULL)
        {
            if(!strcmp(installItems[i].targetPath, path))
            {
                OSReport("%s is already in the install queue\n", path);
                return;
            }
        }
        else
        {
            OSReport("Adding %s to the install queue.\n", path);
            installItems[i].targetPath = memalign(0x40, strlen(path));
            strcpy(installItems[i].targetPath, path);
            installItems[i].targetDevice = selectedInstallTarget;
            
            //Extract icon TGA
            mz_zip_archive icon_woomy_archive = {0};
            ezxml_t icon_woomy_xml;
            if(mz_zip_reader_init_file(&icon_woomy_archive, path, 0))
            {
                void *meta_buf = malloc(0x8000);
                int status = mz_zip_reader_extract_file_to_mem(&icon_woomy_archive, "metadata.xml", meta_buf, 0x8000, 0);
                if(!status)
                {
                    free(meta_buf);
                    break;
                }
                
                icon_woomy_xml = ezxml_parse_str(meta_buf, strlen(meta_buf));
                   
                ezxml_t woomy_metadata_name = ezxml_get(icon_woomy_xml, "metadata", 0, "name", -1);
                char *woomy_archive_name;
                if(!woomy_metadata_name)
                    woomy_archive_name = "<no name>";
                else
                    woomy_archive_name = woomy_metadata_name->txt;
                    
                installItems[i].archiveName = calloc(0x200, sizeof(u8));
                strcpy(installItems[i].archiveName, woomy_archive_name);
                    
                //Show the icon if it's available
                if(!strcmp(ezxml_get(icon_woomy_xml, "metadata", 0, "icon", -1)->txt, "1"))
                {
                    void *icon_tga = memalign(0x100, 0x10100);
                    status = mz_zip_reader_extract_file_to_mem(&icon_woomy_archive, "icon.tga", icon_tga, 0x10100, 0);
                    if(status)
                    { 
                        load_tga(&textures[i], icon_tga);
                    }
                    free(icon_tga);
                }
                
                free(meta_buf);
            }
            else
            {
                installItems[i].archiveName = NULL;
                //TODO: FST icons?
            }
            
            break;
        }
    }
}


void install_init_start_queue_manager()
{
    // Allocate MCP buffers
    mcp_handle = MCP_Open();
    mcp_prog_buf = memalign(0x40, 0x24);
    
    devicelist = memalign(0x40, sizeof(MCPDeviceList));
    memset(devicelist, 0, sizeof(MCPDeviceList));
    int devret = MCP_FullDeviceList(mcp_handle, &numDevices, devicelist, sizeof(MCPDeviceList));
    
    memset(textures, 0, sizeof(textures));
    
    if(devret == 0x12345678)
    {
        //Fake decaf names
        numDevices = 8;
        strcpy(devicelist->devices[0].name, "usb");
        strcpy(devicelist->devices[1].name, "slccmpt");
        strcpy(devicelist->devices[2].name, "slc");
        strcpy(devicelist->devices[3].name, "sdcard");
        strcpy(devicelist->devices[4].name, "ramdisk");
        strcpy(devicelist->devices[5].name, "drh");
        strcpy(devicelist->devices[6].name, "bt");
        strcpy(devicelist->devices[7].name, "mlc");
    }
    
    //Iterate devices to discover install points
    int numUSB = 1;
    for(int i = 0; i < numDevices; i++)
    {
        if(!strcmp(devicelist->devices[i].name, "usb"))
        {
            if(numUSB == 1)
                selectedInstallTarget = numInstallDevices;
        
            installDevices[numInstallDevices].deviceID = MCP_INSTALL_TARGET_USB;
            installDevices[numInstallDevices].deviceNum = numUSB++;
            installDevices[numInstallDevices].deviceName = devicelist->devices[i].name;
            numInstallDevices++;
            
        }
        else if(!strcmp(devicelist->devices[i].name, "mlc"))
        {
            installDevices[numInstallDevices].deviceID = MCP_INSTALL_TARGET_MLC;
            installDevices[numInstallDevices].deviceNum = 1;
            installDevices[numInstallDevices].deviceName = devicelist->devices[i].name;
            numInstallDevices++;
        }
    }

    OSThread *threadCore2 = OSGetDefaultThread(2);
    OSRunThread(threadCore2, (OSThreadEntryPointFn)install_queue_thread, 0, NULL);
}

void install_deinit()
{
    
}

void install_cycle_device()
{
    selectedInstallTarget = (selectedInstallTarget + 1) % numInstallDevices;
}

int install_get_target()
{
    return selectedInstallTarget;
}

InstallDevice *install_get_device(u8 device_id)
{
    return &installDevices[device_id];
}

void install_abort_current()
{
    if(installing)
    {
        MCP_InstallTitleAbort(mcp_handle);
    }
}

InstallQueueItem *install_get_queue_item(int index)
{
    return &installItems[index];
}

char *install_get_queue_path(int index)
{
    return installItems[index].targetPath;
}

char *install_get_current_install_name()
{
    return woomy_entry_name;
}

u8 install_get_queue_install_device(int index)
{
    if(installItems[index].targetPath == NULL) return 0;
    
    return installItems[index].targetDevice;
}

GX2Texture *install_get_queue_texture(int index)
{
    return &textures[index];
}
