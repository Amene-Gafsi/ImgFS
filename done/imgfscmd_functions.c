/**
 * @file imgfscmd_functions.c
 * @brief imgFS command line interpreter for imgFS core commands.
 *
 * @author Mia Primorac
 */

#include "imgfs.h"
#include "imgfscmd_functions.h"
#include "util.h" // for _unused

#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

// default values
static const uint32_t default_max_files = 128;
static const uint16_t default_thumb_res = 64;
static const uint16_t default_small_res = 256;

// max values
static const uint16_t MAX_THUMB_RES = 128;
static const uint16_t MAX_SMALL_RES = 512;

/**********************************************************************
 * Displays some explanations.
 ********************************************************************** */
int help(int useless _unused, char **useless_too _unused)
{
    /* **********************************************************************
     * TODO WEEK 08: WRITE YOUR CODE HERE.
     * **********************************************************************
     */

    TO_BE_IMPLEMENTED();
    return NOT_IMPLEMENTED;
}

/**********************************************************************
 * Opens imgFS file and calls do_list().
 ********************************************************************** */
int do_list_cmd(int argc, char **argv)
{
    /* **********************************************************************
     * TODO WEEK 07: WRITE YOUR CODE HERE.
     * **********************************************************************
     */
    if (argv == NULL)
        return ERR_INVALID_ARGUMENT;

    if (argc != 1)
        return ERR_INVALID_COMMAND;

    const char *file_name = argv[0];

    if (file_name == NULL)
        return ERR_INVALID_ARGUMENT;

    struct imgfs_file file_to_create;
    memset(&file_to_create, 0, sizeof(file_to_create));

    if (do_open(file_name, "rb", &file_to_create) != ERR_NONE)
        return ERR_IO;

    if (do_list(&file_to_create, STDOUT, NULL) != ERR_NONE)
        return ERR_IO; // TODO Json
    do_close(&file_to_create);

    return ERR_NONE;
}

/**********************************************************************
 * Prepares and calls do_create command.
 ********************************************************************** */
int do_create_cmd(int argc, char **argv)
{

    puts("Create");
    /* **********************************************************************
     * TODO WEEK 08: WRITE YOUR CODE HERE (and change the return if needed).
     * **********************************************************************
     */
    struct imgfs_file imgfs_file;
    memset(&imgfs_file, 0, sizeof(imgfs_file));
    const char *imgfs_filename = NULL;
    uint32_t max_files = -1;
    uint16_t thumb_width = -1;
    uint16_t thumb_height = -1;
    uint16_t small_width = -1;
    uint16_t small_height = -1;

    for (size_t i = 0; i < argc; i++)
    {
        if (!strcmp(argv[i], "imgFS_filename"))
        {
            imgfs_filename = argv[i + 1];
            i = i + 1;
        }
        else if (!strcmp(argv[i], "max_files"))
        {
            max_files = atouint32(argv[i + 1]);
            i = i + 1;
        }
        else if (!strcmp(argv[i], "thumb_res"))
        {
            thumb_width = atouint16(argv[i + 1]);
            thumb_height = atouint16(argv[i + 2]);
            i = i + 2;
        }
        else if (!strcmp(argv[i], "small_res"))
        {
            small_width = atouint16(argv[i + 1]);
            small_height = atouint16(argv[i + 2]);
            i = i + 2;
        }
    }
    int return_value = 0;
    if (imgfs_filename != NULL)
    {
        if (max_files > 0)
        {
            int entered = 0;
            if (((thumb_width > 0) && (thumb_height > 0)))
            {
                entered++;
                imgfs_file.header.max_files = max_files;
                imgfs_file.header.resized_res[0] = thumb_width;
                imgfs_file.header.resized_res[1] = thumb_height;
            }

            if (((small_width > 0) && (small_height > 0)))
            {
                entered++;
                imgfs_file.header.max_files = max_files;
                imgfs_file.header.resized_res[2] = small_width;
                imgfs_file.header.resized_res[3] = small_height;
            }

            if (entered == 0)
                return_value = (thumb_width == 0 || thumb_height == 0 || small_width = 0 || small_height = 0) ? ERR_RESOLUTIONS : ERR_NOT_ENOUGH_ARGUMENTS;
        }
        else
            return_value = (max_files == 0) ? ERR_MAX_FILES : ERR_NOT_ENOUGH_ARGUMENTS;
    }
    else
        return_value = ERR_NOT_ENOUGH_ARGUMENTS;

    if (return_value == ERR_NONE)
        return_value = do_create(imgfs_filename, imgfs_file);
    return return_value;
}

/**********************************************************************
 * Deletes an image from the imgFS.
 */
int do_delete_cmd(int argc, char **argv)
{
    /* **********************************************************************
     * TODO WEEK 08: WRITE YOUR CODE HERE (and change the return if needed).
     * **********************************************************************
     */

    TO_BE_IMPLEMENTED();
    return NOT_IMPLEMENTED;
}
