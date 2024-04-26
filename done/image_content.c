#include "image_content.h"
#include "imgfs.h"
#include <vips/vips.h>
#include <stdlib.h>
#include <stdio.h>

// Index of the width value for different resolutions
#define THUMB_RES_WIDTH_INDEX 0
#define SMALL_RES_WIDTH_INDEX 2

// Error if the ftell function fails
#define ERR_FTELL -1

#define OFFSET_ZERO 0

/**********************************************************************
 * Resize the image to the given resolution, if needed.
 ********************************************************************** */
int lazily_resize(int resolution, struct imgfs_file *imgfs_file, size_t index)
{ // Check if arguments are valid
    if (resolution != THUMB_RES && resolution != SMALL_RES && resolution != ORIG_RES)
        return ERR_INVALID_ARGUMENT;

    M_REQUIRE_NON_NULL(imgfs_file);
    M_REQUIRE_NON_NULL(imgfs_file->file);

    if (index < 0 || index >= imgfs_file->header.max_files)
        return ERR_INVALID_IMGID;

    if (!imgfs_file->metadata[index].is_valid)
        return ERR_INVALID_IMGID;

    if (resolution == ORIG_RES)
        return ERR_NONE;

    // Check if image already exists in given resolution
    if (imgfs_file->metadata[index].size[resolution])
        return ERR_NONE;

    // Find the correct width according to the resolution
    uint16_t width = (resolution == THUMB_RES) ? imgfs_file->header.resized_res[THUMB_RES_WIDTH_INDEX] : imgfs_file->header.resized_res[SMALL_RES_WIDTH_INDEX];

    if (fseek(imgfs_file->file, (long)imgfs_file->metadata[index].offset[ORIG_RES], SEEK_SET))
    {
        return ERR_IO;
    }

    // Resize the original image to the requested resolution and free allocated memory in case of error
    void *orig_img = calloc(1, imgfs_file->metadata[index].size[ORIG_RES]);
    if (fread(orig_img, imgfs_file->metadata[index].size[ORIG_RES], ONE_ELEMENT, imgfs_file->file) != ONE_ELEMENT)
    {
        free(orig_img);
        return ERR_IO;
    }

    VipsImage *vips_orig_img = NULL;
    if (vips_jpegload_buffer(orig_img, imgfs_file->metadata[index].size[ORIG_RES], &vips_orig_img, NULL))
    {
        free(orig_img);
        return ERR_IO;
    }

    VipsImage *vips_resized_img = NULL;
    if (vips_thumbnail_image(vips_orig_img, &vips_resized_img, width, NULL))
    {
        free(orig_img);
        g_object_unref(vips_orig_img);
        return ERR_IO;
    }

    size_t len = 0;
    void *resized_img = NULL;
    if (vips_jpegsave_buffer(vips_resized_img, &resized_img, &len, NULL))
    {
        free(orig_img);
        g_object_unref(vips_orig_img);
        g_object_unref(vips_resized_img);
        return ERR_IO;
    }

    // Write the resized image at the end of the file
    if (fseek(imgfs_file->file, OFFSET_ZERO, SEEK_END))
    {
        free(orig_img);
        free(resized_img);
        g_object_unref(vips_orig_img);
        g_object_unref(vips_resized_img);
        return ERR_IO;
    }

    if (fwrite(resized_img, len, ONE_ELEMENT, imgfs_file->file) != ONE_ELEMENT)
    {
        free(orig_img);
        free(resized_img);
        g_object_unref(vips_orig_img);
        g_object_unref(vips_resized_img);
        return ERR_IO;
    }

    // Update the metadata with the correct size and offset, then write it back to the file
    imgfs_file->metadata[index].size[resolution] = (uint32_t)len;
    long end_of_file = ftell(imgfs_file->file);

    if (end_of_file == ERR_FTELL)
    {
        free(orig_img);
        free(resized_img);
        g_object_unref(vips_orig_img);
        g_object_unref(vips_resized_img);
        return ERR_IO;
    }

    imgfs_file->metadata[index].offset[resolution] = (uint64_t)end_of_file - len;

    if (fseek(imgfs_file->file, sizeof(imgfs_file->header), SEEK_SET))
    {
        free(orig_img);
        free(resized_img);
        g_object_unref(vips_orig_img);
        g_object_unref(vips_resized_img);
        return ERR_IO;
    }

    if (fwrite(imgfs_file->metadata, sizeof(struct img_metadata), imgfs_file->header.max_files, imgfs_file->file) != imgfs_file->header.max_files) // TODO maxfiles ou 1
    {
        free(orig_img);
        free(resized_img);
        g_object_unref(vips_orig_img);
        g_object_unref(vips_resized_img);
        return ERR_IO;
    }

    free(orig_img);
    free(resized_img);
    g_object_unref(vips_orig_img);
    g_object_unref(vips_resized_img);
    return ERR_NONE;
}
