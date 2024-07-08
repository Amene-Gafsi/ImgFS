#include "imgfs.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define ONE_ELEMENT 1

/**********************************************************************
 * Creates a new database for the imgfs format.
 ********************************************************************** */
int do_create(const char *imgfs_filename, struct imgfs_file *imgfs_file)
{
    // Check if arguments are valid
    M_REQUIRE_NON_NULL(imgfs_filename);
    M_REQUIRE_NON_NULL(imgfs_file);

    // Initialize the header
    strcpy(imgfs_file->header.name, CAT_TXT);
    imgfs_file->header.version = EMPTY;
    imgfs_file->header.nb_files = EMPTY;
    imgfs_file->header.unused_32 = EMPTY;
    imgfs_file->header.unused_64 = EMPTY;

    imgfs_file->metadata = calloc(imgfs_file->header.max_files, sizeof(struct img_metadata));
    if (imgfs_file->metadata == NULL)
        return ERR_OUT_OF_MEMORY;

    // Initialize is_valid field
    for (uint32_t i = 0; i < imgfs_file->header.max_files; ++i)
    {
        imgfs_file->metadata[i].is_valid = EMPTY;
    }

    size_t items_written = EMPTY;

    // Write the imgfs_file to the file, whose path is given by imgfs_filename, in the database
    FILE *imgfs = fopen(imgfs_filename, "wb");
    if (imgfs == NULL)
    {
        free(imgfs_file->metadata);
        imgfs_file->metadata = NULL;
        return ERR_INVALID_ARGUMENT;
    }

    imgfs_file->file = imgfs;

    if (fwrite(&(imgfs_file->header), sizeof(struct imgfs_header), ONE_ELEMENT, imgfs_file->file) != ONE_ELEMENT)
    {
        free(imgfs_file->metadata);
        imgfs_file->metadata = NULL;
        fclose(imgfs_file->file);
        return ERR_IO;
    }

    items_written++;
    int ret = ERR_NONE;

    if (fwrite(imgfs_file->metadata, sizeof(struct img_metadata), imgfs_file->header.max_files, imgfs_file->file) != imgfs_file->header.max_files)
    {
        free(imgfs_file->metadata);
        imgfs_file->metadata = NULL;
        fclose(imgfs_file->file);
        ret = ERR_IO;
    }

    if (ret == ERR_NONE)
    {
        items_written += imgfs_file->header.max_files;
    }

    // Reset pointer
    rewind(imgfs_file->file);

    // Print number of elements written
    printf("%zu item(s) written\n", items_written);
    return ret;
}
