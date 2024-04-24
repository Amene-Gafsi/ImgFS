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
    
    printf("imgfscmd [COMMAND] [ARGUMENTS]\n");
    printf("  help: displays this help.\n");
    printf("  list <imgFS_filename>: list imgFS content.\n");
    printf("  create <imgFS_filename> [options]: create a new imgFS.\n");
    printf("      options are:\n");
    printf("          -max_files <MAX_FILES>: maximum number of files.\n");
    printf("                                  default value is 128\n");
    printf("                                  maximum value is 4294967295\n");
    printf("          -thumb_res <X_RES> <Y_RES>: resolution for thumbnail images.\n");
    printf("                                  default value is 64x64\n");
    printf("                                  maximum value is 128x128\n");
    printf("          -small_res <X_RES> <Y_RES>: resolution for small images.\n");
    printf("                                  default value is 256x256\n");
    printf("                                  maximum value is 512x512\n");
    printf("  delete <imgFS_filename> <imgID>: delete image imgID from imgFS.\n");

    return ERR_NONE;
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
    int return_value = ERR_NONE;

    struct imgfs_file file_to_create;
    memset(&file_to_create, 0, sizeof(file_to_create));

    return_value = do_open(file_name, "rb", &file_to_create);
    if (return_value != ERR_NONE)
        return return_value;

    return_value = do_list(&file_to_create, STDOUT, NULL);
    if (return_value != ERR_NONE)
        return return_value;

    do_close(&file_to_create);

    return return_value;
}

/**********************************************************************
 * Prepares and calls do_create command.
 ********************************************************************** */
int do_create_cmd(int argc, char **argv)
{
    //puts("Create");
    /* **********************************************************************
     * TODO WEEK 08: WRITE YOUR CODE HERE (and change the return if needed).
     * **********************************************************************
     */
    M_REQUIRE_NON_NULL(argv);
    if (argc == 0)
        return ERR_NOT_ENOUGH_ARGUMENTS;

    const char *imgfs_filename = argv[0];
    uint32_t max_files = default_max_files;
    uint16_t thumb_width = default_thumb_res, thumb_height = default_thumb_res,
             small_width = default_small_res, small_height = default_small_res;

    if (argc > 1)
    {
        for (int i = 1; i < argc; i++)
        {
            if (!strcmp(argv[i], "-max_files"))
            {
                if (i + 1 >= argc)
                    return ERR_NOT_ENOUGH_ARGUMENTS;
                max_files= atouint32(argv[i + 1]);
                i += 1;
            }
            else if (!strcmp(argv[i], "-thumb_res"))
            {
                if (i + 2 >= argc)
                    return ERR_NOT_ENOUGH_ARGUMENTS;
                thumb_width = atouint16(argv[i + 1]);
                thumb_height = atouint16(argv[i + 2]);
                i = i + 2;
            }
            else if (!strcmp(argv[i], "-small_res"))
            {
                if (i + 2 >= argc)
                    return ERR_NOT_ENOUGH_ARGUMENTS;
                small_width = atouint16(argv[i + 1]);
                small_height = atouint16(argv[i + 2]);
                i = i + 2;
            }
            else
                return ERR_INVALID_ARGUMENT;
        }
    }
    if (max_files == 0) return ERR_INVALID_ARGUMENT;
    if (max_files > default_max_files) return ERR_MAX_FILES;
    if (thumb_width <= 0 || thumb_width > MAX_THUMB_RES || thumb_height <= 0 || thumb_height > MAX_THUMB_RES) return ERR_RESOLUTIONS;
    if (small_width <= 0 || small_width > MAX_SMALL_RES || small_height <= 0 || small_height > MAX_SMALL_RES) return ERR_RESOLUTIONS;

    struct imgfs_header header = {.max_files = max_files,
                                  .resized_res = {thumb_width, thumb_height, small_width, small_height}};

    struct imgfs_file imgfs_file;
    imgfs_file.header = header;

    int return_value = do_create(imgfs_filename, &imgfs_file);
    if (return_value == ERR_NONE) do_close(&imgfs_file);
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
    M_REQUIRE_NON_NULL(argv);
    if(argc != 2) return ERR_NOT_ENOUGH_ARGUMENTS;

    const char* filename = argv[0];
    const char* img_ID = argv[1];
    if((img_ID == NULL) || (strlen(img_ID) > MAX_IMG_ID)) return ERR_INVALID_IMGID;

    struct imgfs_file imgfs_file;
    
    int return_value = ERR_NONE;
    return_value = do_open(filename, "r+b", &imgfs_file);
    if (return_value != ERR_NONE) return return_value;
    
    return_value = do_delete(img_ID, &imgfs_file);
    do_close(&imgfs_file);


    return return_value;
}
