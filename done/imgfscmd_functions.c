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
    if(argv == NULL) return ERR_INVALID_ARGUMENT;

    if(argc != 1) return ERR_INVALID_COMMAND;

    const char *file_name = argv[0];

    if (file_name == NULL) return ERR_INVALID_ARGUMENT;

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

    TO_BE_IMPLEMENTED();
    return NOT_IMPLEMENTED;
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
