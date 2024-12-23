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
            break;
        }
    }

    if (image == NOT_FOUND)
    {
        return ERR_IMAGE_NOT_FOUND;
    }

    uint32_t old_version = imgfs_file->header.version;
    uint32_t old_nb_files = imgfs_file->header.nb_files;

    imgfs_file->metadata[index].is_valid = EMPTY;
    imgfs_file->header.version++;
    imgfs_file->header.nb_files--;

    size_t metadata_offset = sizeof(imgfs_file->header) + index * sizeof(struct img_metadata);

    if (!fseek(imgfs_file->file, (long)metadata_offset, SEEK_SET))
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
    imgfs_file->metadata[index].is_valid = NON_EMPTY;
    imgfs_file->header.version = old_version;
    imgfs_file->header.nb_files = old_nb_files;
    return ERR_IO;
}
