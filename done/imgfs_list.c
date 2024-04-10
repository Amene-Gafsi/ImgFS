#include "imgfs.h"
#include <stdio.h>
#include "util.h"

int do_list(const struct imgfs_file *imgfs_file, enum do_list_mode output_mode, char **json)
{
    int exist_image = 0;
    if (output_mode == STDOUT)
    {
        print_header(&(imgfs_file->header));

        for (int i = 0; i <= imgfs_file->header.max_files; i++)
        {
            if ((imgfs_file->metadata)[i].is_valid)
            {
                print_metadata(&(imgfs_file->metadata)[i]);
                exist_image += 1;
            }
        }
        if (exist_image == 0)
            printf("<< empty imgFS >>");
        return ERR_NONE;
    }
    else if (output_mode == JSON)
    {
        TO_BE_IMPLEMENTED();
    }
    else
        return ERR_IO;
}