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
                header.version++; // Modify the copy
                header.nb_files--;
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

    // Changes are made first to the metadata (memory, then disk), then to the header if successful
    free(imgfs_file->metadata);  // Free previous metadata
    imgfs_file->metadata = metadata;

    if (!fseek(imgfs_file->file, sizeof(struct imgfs_header), SEEK_SET))
    {
        if (fwrite(metadata, sizeof(struct img_metadata), imgfs_file->header.max_files, imgfs_file->file) == imgfs_file->header.max_files)
        {
            imgfs_file->header = header; // If the write succeeds (correct open mode), paste the copy in the imgfs_file
            if (!fseek(imgfs_file->file, 0, SEEK_SET))
            {
                if (fwrite(&(header), sizeof(struct imgfs_header), ONE_ELEMENT, imgfs_file->file) == ONE_ELEMENT)
                {
                    return ERR_NONE;
                }
            }
        }
    }
    return ERR_IO;
}
