#include "image_content.h"
#include "imgfs.h"
#include <vips/vips.h>
#include <stdlib.h>
#include <stdio.h>

int lazily_resize(int resolution, struct imgfs_file *imgfs_file, size_t index)
{ // check if arguments are valid
    if (resolution != THUMB_RES && resolution != SMALL_RES && resolution != ORIG_RES)
        return ERR_INVALID_ARGUMENT;

    M_REQUIRE_NON_NULL(imgfs_file);
    M_REQUIRE_NON_NULL(imgfs_file->file);

    if (index < 0 && index >= imgfs_file->header.max_files)
        return ERR_INVALID_ARGUMENT;

    if (resolution == ORIG_RES)
        return ERR_NONE;

    struct img_metadata indexed_metadata = imgfs_file->metadata[index];

    // check if image already exists in given resolution
    if (indexed_metadata.size[resolution] != 0)
        return ERR_NONE;

    // Resize image
    uint16_t width = (resolution == THUMB_RES) ? imgfs_file->header.resized_res[THUMB_RES] : imgfs_file->header.resized_res[SMALL_RES];

    VipsImage *vips_orig_img = NULL;
    VipsImage *vips_resized_img = NULL;
    void *resized_img = NULL;

    void *orig_img = calloc(1, indexed_metadata.size[ORIG_RES]);
    if (orig_img == NULL)
        return ERR_OUT_OF_MEMORY;

    void *null_ptr = NULL;
    int ret = ERR_IO;

    if (fseek(imgfs_file->file, indexed_metadata.offset[ORIG_RES], SEEK_SET) == 0)
    {
        if (fread(orig_img, indexed_metadata.size[ORIG_RES], 1, imgfs_file->file) == 1)
        {
            if (vips_jpegload_buffer(orig_img, indexed_metadata.size[ORIG_RES], &vips_orig_img, null_ptr) == 0)
            {
                if (vips_thumbnail_image(vips_orig_img, &vips_resized_img, width, null_ptr) == 0)
                {
                    size_t len = NULL;
                    if (vips_jpegsave_buffer(vips_resized_img, &resized_img, &len, null_ptr) == 0)
                    {
                        if (fseek(imgfs_file->file, 0, SEEK_END) == 0)
                        {
                            if (fwrite(resized_img, len, 1, imgfs_file->file) == 1)
                            {
                                indexed_metadata.size[resolution] = len;
                                int end_of_file = ftell(imgfs_file->file); // ftell returns the current position in the file which is the end of the file because of the use of SEEK_END when writing
                                if (end_of_file != -1)
                                {
                                    indexed_metadata.offset[resolution] = end_of_file - len;
                                    if (fwrite(imgfs_file->metadata, sizeof(*(imgfs_file->metadata)), 1, imgfs_file->file) == 1)
                                    {
                                        ret = ERR_NONE;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    free(resized_img);
    free(orig_img);
    free(vips_orig_img);
    free(vips_resized_img);

    return ret;
}