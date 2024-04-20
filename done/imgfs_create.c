#include "imgfs.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/**********************************************************************
 * Creates a new database for the imgfs format.
 ********************************************************************** */
int do_create(const char *imgfs_filename, struct imgfs_file *imgfs_file)
{
    M_REQUIRE_NON_NULL(imgfs_filename);
    M_REQUIRE_NON_NULL(imgfs_file);
    const size_t ALL__HEADER = 1;
    const uint32_t ALL_METADATA = imgfs_file->header.max_files;

    strcpy(imgfs_file->header.name, CAT_TXT);

    imgfs_file->header.version = EMPTY;
    imgfs_file->header.nb_files = EMPTY;
    imgfs_file->header.unused_32 = EMPTY;
    imgfs_file->header.unused_64 = EMPTY;

    imgfs_file->metadata = calloc(ALL_METADATA, sizeof(struct img_metadata));
    if (imgfs_file->metadata == NULL)
        return ERR_OUT_OF_MEMORY;
    size_t items_written = EMPTY;

    // write imgfs_file to imgfs_filename in database
    FILE *imgfs = fopen(imgfs_filename, "wb");
    if (imgfs == NULL)
        return ERR_INVALID_ARGUMENT;

    imgfs_file->file = imgfs;

    if (fwrite(&(imgfs_file->header), sizeof(imgfs_file->header), ALL__HEADER, imgfs_file->file) != ALL__HEADER)
    {
        fclose(imgfs_file->file);
        return ERR_IO;
    }

    items_written++;

    if (fwrite(imgfs_file->metadata, sizeof(*(imgfs_file->metadata)), ALL_METADATA, imgfs_file->file) != ALL_METADATA)
    {
        fclose(imgfs_file->file);
        return ERR_IO;
    }

    items_written++; //TODO correct ?

    // reset pointer
    rewind(imgfs_file->file);

    // print number of elements written
    printf("%zu item(s) written\n", items_written);
    return ERR_NONE;
}
