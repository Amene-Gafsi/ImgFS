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

    // Make a copy of header and metadata
    struct imgfs_header header = imgfs_file->header;
    struct img_metadata *metadata = calloc(imgfs_file->header.max_files, sizeof(struct img_metadata));

    memcpy(metadata, imgfs_file->metadata, imgfs_file->header.max_files * sizeof(struct img_metadata));
    if (metadata == NULL)
    {
        return ERR_OUT_OF_MEMORY; // Ensure memory allocation succeeded
    }

    for (uint32_t i = 0; i < imgfs_file->header.max_files; i++)
    {
        if (!strcmp(imgfs_file->metadata[i].img_id, img_id))
        {
            if (metadata[i].is_valid == NON_EMPTY)
            {
                image = FOUND;
                header.version += 1; // Modify the copy
                header.nb_files -= 1;
                metadata[i].is_valid = EMPTY;
                break;
            }
        }
    }

    if (image == NOT_FOUND)
    {
        free(metadata);
        return ERR_IMAGE_NOT_FOUND;
    }

    // Reset pointer
    rewind(imgfs_file->file);

    if (fwrite(&(header), sizeof(struct imgfs_header), 1, imgfs_file->file) == 1)
    {
        if (fwrite(metadata, sizeof(struct img_metadata), imgfs_file->header.max_files, imgfs_file->file) == imgfs_file->header.max_files)
        {
            imgfs_file->header = header; // If the write succeeds (correct open mode), paste the copy in the imgfs_file
            free(imgfs_file->metadata);
            imgfs_file->metadata = metadata;
            return ERR_NONE;
        }
    }

    free(metadata);
    return ERR_IO;
}
