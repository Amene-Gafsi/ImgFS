#include "imgfs.h"
#include "imgfscmd_functions.h"
#include "util.h" // for _unused

#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

/**********************************************************************
 * Deletes an image.
 ********************************************************************** */
int do_delete(const char *img_id, struct imgfs_file *imgfs_file)
{
    // Check if arguments are valid
    M_REQUIRE_NON_NULL(img_id);
    M_REQUIRE_NON_NULL(imgfs_file);

    int image = NOT_FOUND;
    uint32_t index = EMPTY;

    if (imgfs_file->metadata == NULL)
    {
        return ERR_OUT_OF_MEMORY; // Ensure memory allocation succeeded
    }
    
    for (uint32_t i = 0; i < imgfs_file->header.max_files; i++)
    {
            if (imgfs_file->metadata[i].is_valid == NON_EMPTY && !strcmp(imgfs_file->metadata[i].img_id, img_id)) 
            {
                image = FOUND;
                index = i;
                imgfs_file->header.version++; 
                imgfs_file->header.nb_files--;
                imgfs_file->metadata[i].is_valid = EMPTY;
                break;
            }
    }

    if (image == NOT_FOUND)
    {
        return ERR_IMAGE_NOT_FOUND;
    }

    // Changes are made first to the metadata (memory, then disk), then to the header if successful

    //TODO : write only one element of the of the metadata array

    /*if (!fseek(imgfs_file->file, sizeof(struct imgfs_header), SEEK_SET))
    {
        if (fwrite(imgfs_file->metadata, sizeof(struct img_metadata), imgfs_file->header.max_files, imgfs_file->file) == imgfs_file->header.max_files)
        {
            if (!fseek(imgfs_file->file, 0, SEEK_SET))
            {
                if (fwrite(&(imgfs_file->header), sizeof(struct imgfs_header), ONE_ELEMENT, imgfs_file->file) == ONE_ELEMENT)
                {
                    return ERR_NONE;
                }
            }
        }
    }*/
    size_t metadata_offset = sizeof(imgfs_file->header) + index * sizeof(struct img_metadata);

    if (!fseek(imgfs_file->file, metadata_offset, SEEK_SET))
    {
        if (fwrite(&(imgfs_file->metadata[index]), sizeof(struct img_metadata), ONE_ELEMENT, imgfs_file->file) == ONE_ELEMENT)
        {
            if (!fseek(imgfs_file->file, 0, SEEK_SET))
            {
                if (fwrite(&(imgfs_file->header), sizeof(struct imgfs_header), ONE_ELEMENT, imgfs_file->file) == ONE_ELEMENT)
                {
                    return ERR_NONE;
                }
            }
        }
    }

    return ERR_IO;
}
