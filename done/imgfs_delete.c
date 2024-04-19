#include "imgfs.h"
#include "imgfscmd_functions.h"
#include "util.h" // for _unused

#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

/**********************************************************************
 * Deletes an image.
 ********************************************************************** */
int do_delete(const char* img_id, struct imgfs_file* imgfs_file)
{
    M_REQUIRE_NON_NULL(img_id);
    M_REQUIRE_NON_NULL(imgfs_file);
    const int FOUND = 1, NOT_FOUND = 0;
    int image = NOT_FOUND;
    
    
    for(int i = 0; i < imgfs_file->header.max_files; i++){
        if(!strcmp(imgfs_file->metadata[i].img_id, img_id)){
            image = FOUND;
            imgfs_file->metadata[i].is_valid = EMPTY;
            imgfs_file->header.version += 1;
            imgfs_file->header.nb_files -= 1;
            break;
        }
    }

    if(image == NOT_FOUND) return ERR_IMAGE_NOT_FOUND;
    
    // reset pointer
    rewind(imgfs_file->file);

    if (fwrite(&(imgfs_file->header), sizeof(imgfs_file->header), 1, imgfs_file->file) != 1)
    {
        return ERR_IO;
    }


    if (fwrite(imgfs_file->metadata, sizeof(*(imgfs_file->metadata)), imgfs_file->header.max_files, imgfs_file->file) != imgfs_file->header.max_files)
    {
        return ERR_IO;
    }

    //rewind(imgfs_file->file); TODO

    return ERR_NONE;
}
