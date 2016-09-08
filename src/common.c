/*
 *  Woomïnstaller GX2 - Homebrew package installer for Wii U
 *
 *  Copyright (C) 2016          SALT
 *  Copyright (C) 2016          Max Thomas (Shiny Quagsire) <mtinc2@gmail.com>
 *
 *  This code is licensed under the terms of the GNU LGPL, version 2.1
 *  see file LICENSE.md or https://www.gnu.org/licenses/lgpl-2.1.txt
 */

#include "common.h"

int read_directory(char *path, struct dirent** out)
{
    int entryNum = 0;
    DIR *dir;
    struct dirent *ent;
    
    OSReport("Opening %s\n", path);
    dir = opendir(path);
    if(dir == NULL)
        return 0;
        
    OSReport("Opened %s\n", path);

    while((ent = readdir(dir)))
    {
        memcpy(out[entryNum], ent, sizeof(struct dirent));
        
        if(out[entryNum]->d_type == DT_DIR)
            entryNum++;
            
        //Our tmp folder should never be shown, ever.
        if(!strcmp(ent->d_name, "tmp") && (ent->d_type & DT_DIR) && !strncmp(path, "fs:/vol/external01", 18))
            entryNum--;
    }
    
    rewinddir(dir);
    while((ent = readdir(dir)))
    {
        memcpy(out[entryNum], ent, sizeof(struct dirent));
        
        printf("%x\n", out[entryNum]->d_type);
        
        if(out[entryNum]->d_type != DT_DIR)
            entryNum++;
    }
    
    closedir(dir);
    
    return entryNum;
}
