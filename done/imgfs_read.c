/**
 * @file imgfscmd_read.c
 * @brief Provides a function that reads the content of an image from a imgFS
 *
 * @author Morgane Magnin
 * @author Amene Gafsi
 */

#include "imgfs.h"
#include "image_content.h"
#include <string.h>
#include <stdlib.h>

/********************************************************************
 * Read the content of an image from a imgFS
 *******************************************************************/
int do_read(const char *img_id, int resolution, char **image_buffer, uint32_t *image_size, struct imgfs_file *imgfs_file)
{
    M_REQUIRE_NON_NULL(img_id);
    M_REQUIRE_NON_NULL(image_buffer);
    M_REQUIRE_NON_NULL(image_size);
    M_REQUIRE_NON_NULL(imgfs_file);
    int ret = ERR_NONE;
    int indentifier = NOT_FOUND;

    for (size_t i = 0; i < imgfs_file->header.max_files; i++)
    {
        if (!strcmp(imgfs_file->metadata[i].img_id, img_id))
        {
            indentifier = FOUND;
            // Determine whether the image already exists in the requested resolution
            if (!imgfs_file->metadata[i].offset[resolution])
            {
                ret = lazily_resize(resolution, imgfs_file, i);
                if (ret != ERR_NONE)
                {
                    return ret;
                }
            }

            // Set file pointer to the correct position
            if (fseek(imgfs_file->file, imgfs_file->metadata[i].offset[resolution], SEEK_SET))
            {
                return ERR_IO;
            }

            // Allocate memory for the image in the given resolution
            *image_buffer = calloc(ONE_ELEMENT, imgfs_file->metadata[i].size[resolution]);
            if (*image_buffer == NULL)
            {
                return ERR_OUT_OF_MEMORY;
            }

            *image_size = imgfs_file->metadata[i].size[resolution];
            if (fread(*image_buffer, *image_size, ONE_ELEMENT, imgfs_file->file) != ONE_ELEMENT)
            {
                free(*image_buffer);
                return ERR_IO;
            }
        }
    }

    if (indentifier == NOT_FOUND)
    {
        return ERR_IMAGE_NOT_FOUND;
    }
    return ERR_NONE;
}
