#include "image_content.h"
#include "imgfs.h"

int lazily_resize(int resolution, struct imgfs_file *imgfs_file, size_t index)
{ // check if arguments are valid
    if (resolution != THUMB_RES && resolution != SMALL_RES && resolution != ORIG_RES)
        return ERR_INVALID_ARGUMENT;
        
    M_REQUIRE_NON_NULL(imgfs_file);

    if (index < 0 && index >= imgfs_file->header.max_files)
        return ERR_INVALID_ARGUMENT;
    
    if (resolution == ORIG_RES) return ERR_NONE;

    // check if image already exists in given resolution
    if (imgfs_file->metadata->size[resolution] != 0) return ERR_NONE;

    // Resize image
    uint16_t width = (resolution == THUMB_RES) ? imgfs_file->header.resized_res[THUMB_RES] : imgfs_file->header.resized_res[SMALL_RES];

    int ret = vips_thumbnail_image()
    

}