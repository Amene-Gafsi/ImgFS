/**
 * @file imgfscmd_insert.c
 * @brief Provides a function that inserts an image in the imgFS file
 *
 * @author Morgane Magnin
 * @author Amene Gafsi
 */

#include "imgfs.h"
#include "image_content.h"
#include "image_dedup.h"
#include <string.h>

#define WIDTH_INDEX 0
#define HEIGHT_INDEX 1

/********************************************************************
 * Insert an image into the imgFS.
 *******************************************************************/
int do_insert(const char *image_buffer, size_t image_size, const char *img_id, struct imgfs_file *imgfs_file)
{
    M_REQUIRE_NON_NULL(image_buffer);
    M_REQUIRE_NON_NULL(img_id);
    M_REQUIRE_NON_NULL(imgfs_file);

    if (imgfs_file->header.nb_files >= imgfs_file->header.max_files)
    {
        return ERR_IMGFS_FULL;
    }

    for (uint32_t i = 0; i < imgfs_file->header.max_files; i++)
    {
        // Find an empty entry in the metadata table
        if (!imgfs_file->metadata[i].is_valid) // grader: check relies on the fact that NON_EMPTY=1, -0.5
        {
            if (SHA256((const unsigned char *)image_buffer, image_size, imgfs_file->metadata[i].SHA) == NULL)
            {
                return ERR_IO;
            }

            if (strcpy(imgfs_file->metadata[i].img_id, img_id) == NULL)
            // grader: you could use strncpy with sizeof(imgfs_file->metadata[i].img_id) - 1 or MAX_IMG_ID, -0.5
            {
                return ERR_IO;
            }

            // Initialize the height and width to be determined
            uint32_t height = 0;
            uint32_t width = 0;
            int ret = ERR_NONE;

            ret = get_resolution(&height, &width, image_buffer, image_size);
            if (ret != ERR_NONE)
            {
                return ret;
            }

            imgfs_file->metadata[i].orig_res[WIDTH_INDEX] = width;
            imgfs_file->metadata[i].orig_res[HEIGHT_INDEX] = height;

            ret = do_name_and_content_dedup(imgfs_file, i);
            if (ret != ERR_NONE)
            {
                return ret;
            }

            // Check if image was not duplicated
            if (imgfs_file->metadata[i].offset[ORIG_RES] == OFFSET_ZERO)
            {
                if (fseek(imgfs_file->file, 0, SEEK_END))
                {
                    return ERR_IO;
                }

                // Update the metadata
                imgfs_file->metadata[i].offset[ORIG_RES] = (uint64_t)ftell(imgfs_file->file);
                imgfs_file->metadata[i].offset[THUMB_RES] = EMPTY;
                imgfs_file->metadata[i].offset[SMALL_RES] = EMPTY;

                imgfs_file->metadata[i].size[ORIG_RES] = (uint32_t)image_size;
                imgfs_file->metadata[i].size[THUMB_RES] = EMPTY;
                imgfs_file->metadata[i].size[SMALL_RES] = EMPTY;

                if (fwrite(image_buffer, image_size, ONE_ELEMENT, imgfs_file->file) != ONE_ELEMENT)
                {
                    return ERR_IO;
                }
            }

            imgfs_file->metadata[i].is_valid = NON_EMPTY;

            // Update the header
            imgfs_file->header.nb_files++;
            imgfs_file->header.version++;

            // Write the header and the corresponding metadata to disk
            if (fseek(imgfs_file->file, OFFSET_ZERO, SEEK_SET))
            {
                return ERR_IO;
            }

            if (fwrite(&(imgfs_file->header), sizeof(struct imgfs_header), ONE_ELEMENT, imgfs_file->file) != ONE_ELEMENT)
            {
                return ERR_IO;
            }

            if (fseek(imgfs_file->file, i * sizeof(struct img_metadata), SEEK_CUR))
            {
                return ERR_IO;
            }
            if (fwrite(&(imgfs_file->metadata[i]), sizeof(struct img_metadata), ONE_ELEMENT, imgfs_file->file) != ONE_ELEMENT)
            {
                return ERR_IO;
            }
            break;
        }
    }
    return ERR_NONE;
}
