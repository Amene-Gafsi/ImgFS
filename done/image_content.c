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

/**
 * @brief Free memory allocated to the original and resized images, as well as their corresponding VipsImage
 *
 * @param original_image Pointer to the original image
 * @param resized_image Pointer to the resized image
 * @param original_vipsimage Pointer to the original VipsImage
 * @param resized_vipsimage Pointer to the resized VipsImage
 *
 * @return some error code
 */
int free_images(void *original_image, void *resized_image, VipsImage *original_vipsimage, VipsImage *resized_vipsimage)
{
    free(original_image);
    free(resized_image);
    g_object_unref(original_vipsimage);
    g_object_unref(resized_vipsimage);
    return ERR_NONE;
}

/**********************************************************************
 * Resize the image to the given resolution, if needed.
 ********************************************************************** */
int lazily_resize(int resolution, struct imgfs_file *imgfs_file, size_t index)
{ // Check if arguments are valid
    if (resolution != THUMB_RES && resolution != SMALL_RES && resolution != ORIG_RES)
        return ERR_INVALID_ARGUMENT;

    M_REQUIRE_NON_NULL(imgfs_file);
    M_REQUIRE_NON_NULL(imgfs_file->file);

    if (index < 0 || index >= imgfs_file->header.max_files){
        return ERR_INVALID_IMGID;}

    if (!imgfs_file->metadata[index].is_valid){
        return ERR_INVALID_IMGID;}

    if (resolution == ORIG_RES){
        return ERR_NONE;}

    // Check if image already exists in given resolution
    if (imgfs_file->metadata[index].size[resolution]){
        return ERR_NONE;}
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
        free_images(orig_img, resized_img, vips_orig_img, vips_resized_img);
        return ERR_IO;
    }

    if (fwrite(resized_img, len, ONE_ELEMENT, imgfs_file->file) != ONE_ELEMENT)
    {
        free_images(orig_img, resized_img, vips_orig_img, vips_resized_img);
        return ERR_IO;
    }

    // Update the metadata with the correct size and offset, then write it back to the file
    imgfs_file->metadata[index].size[resolution] = (uint32_t)len;
    long end_of_file = ftell(imgfs_file->file);

    if (end_of_file == ERR_FTELL)
    {
        free_images(orig_img, resized_img, vips_orig_img, vips_resized_img);
        return ERR_IO;
    }

    imgfs_file->metadata[index].offset[resolution] = (uint64_t)end_of_file - len;

    if (fseek(imgfs_file->file, sizeof(imgfs_file->header), SEEK_SET))
    {
        free_images(orig_img, resized_img, vips_orig_img, vips_resized_img);
        return ERR_IO;
    }

    if (fwrite(imgfs_file->metadata, sizeof(struct img_metadata), imgfs_file->header.max_files, imgfs_file->file) != imgfs_file->header.max_files)
    {
        free_images(orig_img, resized_img, vips_orig_img, vips_resized_img);
        return ERR_IO;
    }

    return free_images(orig_img, resized_img, vips_orig_img, vips_resized_img);
}

/**********************************************************************
 * Gets the resolution of an image.
 ********************************************************************** */
int get_resolution(uint32_t *height, uint32_t *width,
                   const char *image_buffer, size_t image_size)
{    
    M_REQUIRE_NON_NULL(height);
    M_REQUIRE_NON_NULL(width);
    M_REQUIRE_NON_NULL(image_buffer);

    VipsImage* original = NULL;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
    const int err = vips_jpegload_buffer((void*) image_buffer, image_size,
                                         &original, NULL);
#pragma GCC diagnostic pop
    if (err != ERR_NONE) return ERR_IMGLIB;
    
    *height = (uint32_t) vips_image_get_height(original);
    *width  = (uint32_t) vips_image_get_width (original);
    
    g_object_unref(VIPS_OBJECT(original));
    return ERR_NONE;
}

