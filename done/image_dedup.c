#include "image_dedup.h"
#include "imgfs.h"
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#define OFFSET_ZERO 0

/**********************************************************************
 * Deduplicates the image if its content is identical to another existing image
 ********************************************************************** */
int do_name_and_content_dedup(struct imgfs_file *imgfs_file, uint32_t index)
{
    // Check if arguments are valid
    M_REQUIRE_NON_NULL(imgfs_file);
    M_REQUIRE_NON_NULL(imgfs_file->file);
    if (index < 0 || index >= imgfs_file->header.max_files)
        return ERR_IMAGE_NOT_FOUND;

    // Check among all images (metadata) for duplicated image ID or content
    int duplicate = EMPTY;
    for (uint32_t i = 0; i < imgfs_file->header.max_files; i++)
    {
        if (i != index && imgfs_file->metadata[i].is_valid)
        {
            if (!strcmp(imgfs_file->metadata[i].img_id, imgfs_file->metadata[index].img_id))
            {
                return ERR_DUPLICATE_ID;
            }
            else if (!memcmp(imgfs_file->metadata[i].SHA, imgfs_file->metadata[index].SHA, SHA256_DIGEST_LENGTH))
            {
                // If the content is duplicated, we copy the size and offset of the original image to the indexed image
                memcpy(imgfs_file->metadata[index].size, imgfs_file->metadata[i].size, sizeof(imgfs_file->metadata[i].size));
                memcpy(imgfs_file->metadata[index].offset, imgfs_file->metadata[i].offset, sizeof(imgfs_file->metadata[i].offset));
                duplicate = NON_EMPTY;
            }
        }
    }
    if (duplicate == EMPTY)
    {
        imgfs_file->metadata[index].offset[ORIG_RES] = OFFSET_ZERO;
    }
    return ERR_NONE;
}
