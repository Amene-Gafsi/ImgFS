#include "imgfs.h"
#include <stdio.h>
#include "util.h"

/**********************************************************************
 * Displays the imgFS metadata on stdout
 ********************************************************************** */
int do_list(const struct imgfs_file *imgfs_file, enum do_list_mode output_mode, char **json)
{
    // Check if arguments are valid
    M_REQUIRE_NON_NULL(imgfs_file);

    int images = NOT_FOUND;

    if (output_mode == STDOUT)
    {
        print_header(&(imgfs_file->header));

        for (uint32_t i = 0; i < imgfs_file->header.max_files; i++)
        {
            if (imgfs_file->metadata != NULL)
            {
                if ((imgfs_file->metadata)[i].is_valid)
                {
                    print_metadata(&(imgfs_file->metadata)[i]);
                    images++;
                }
            }
        }
        if (images == NOT_FOUND)
            printf("<< empty imgFS >>\n");
        return ERR_NONE;
    }
    else if (output_mode == JSON)
    {
        TO_BE_IMPLEMENTED();
    }
    else
        return ERR_IO;
}
