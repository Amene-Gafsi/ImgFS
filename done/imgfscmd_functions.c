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

#define FILE_NAME_INDEX 0
#define IMG_ID_INDEX 1

// The number of arguments for max_files, thumb_res and small_res
#define MAX_FILES_ARG 1
#define WIDTH_AND_HEIGHT_ARGS 2

#define TWO_ELEMENTS 2


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
    printf("                                  default value is %" PRIu32 "\n", default_max_files);
    printf("                                  maximum value is 4294967295\n");
    printf("          -thumb_res <X_RES> <Y_RES>: resolution for thumbnail images.\n");
    printf("                                  default value is %" PRIu16 "x%" PRIu16 "\n", default_thumb_res, default_thumb_res);
    printf("                                  maximum value is %" PRIu16 "x%" PRIu16 "\n", MAX_THUMB_RES, MAX_THUMB_RES);
    printf("          -small_res <X_RES> <Y_RES>: resolution for small images.\n");
    printf("                                  default value is %" PRIu16 "x%" PRIu16 "\n", default_small_res, default_small_res);
    printf("                                  maximum value is %" PRIu16 "x%" PRIu16 "\n", MAX_SMALL_RES, MAX_SMALL_RES);
    printf("  read   <imgFS_filename> <imgID> [original|orig|thumbnail|thumb|small]:\n");
    printf("      read an image from the imgFS and save it to a file.\n");
    printf("      default resolution is \"original\".\n");
    printf("  insert <imgFS_filename> <imgID> <filename>: insert a new image in the imgFS.\n");
    printf("  delete <imgFS_filename> <imgID>: delete image imgID from imgFS.\n");

    return ERR_NONE;
    }

/********************************************************************
 * Create a new image name
 *******************************************************************/
static void create_name(const char *img_id, int resolution, char **new_name)
{
    *new_name = calloc(1, MAX_IMGFS_NAME + NULL_TERMINATOR); 
    if (*new_name == NULL)
    {
        return;
    }

    const char *resolution_suffix;

    switch (resolution)
    {
    case THUMB_RES:
        resolution_suffix = "_thumb";
        break;

    case SMALL_RES:
        resolution_suffix = "_small";
        break;

    case ORIG_RES:
        resolution_suffix = "_orig";
        break;

    default:
        free(*new_name);
        break;
    }

    // Create a temporary buffer for the concatenation
    char temp[MAX_IMGFS_NAME + NULL_TERMINATOR];  // +1 for the null terminator
    if(snprintf(temp, sizeof(temp), "%s%s.jpg", img_id, resolution_suffix) < 0){
        free(*new_name);
    } 
    
    // Copy the result to the new_name
    strcpy(*new_name, temp);
    if (*new_name == NULL)
    {
        free(*new_name);
    }
}

/********************************************************************
 * Write an image to disk
 *******************************************************************/
static int write_disk_image(const char *filename, const char *image_buffer, uint32_t image_size)
{
    M_REQUIRE_NON_NULL(filename);
    M_REQUIRE_NON_NULL(image_buffer);

    FILE *file = fopen(filename, "wb");
    if (file == NULL)
    {
        return ERR_IO;
    }

    if (fwrite(image_buffer, image_size, ONE_ELEMENT, file) != ONE_ELEMENT)
    {
        fclose(file);
        return ERR_IO;
    }

    fclose(file);
    return ERR_NONE;
}

/********************************************************************
 * Read the image from disk
 *******************************************************************/
static int read_disk_image(const char *path, char **image_buffer, uint32_t *image_size)
{
    M_REQUIRE_NON_NULL(path);
    M_REQUIRE_NON_NULL(image_size);

    FILE *file = fopen(path, "rb");
    if (file == NULL)
    {
        return ERR_IO;
    }

    // Find file size  
    if(fseek(file, 0, SEEK_END)){
        fclose(file);
        return ERR_IO;
    }
    
    long size = ftell(file);
    if (size < 0)
    {
        fclose(file);
        return ERR_IO;
    }
    *image_size = (uint32_t)ftell(file);

    // Reset position
    if(fseek(file, 0, SEEK_SET)){
        fclose(file);
        return ERR_IO;
    }

    *image_buffer = calloc(ONE_ELEMENT, *image_size);
    if (*image_buffer == NULL)
    {
        fclose(file);
        return ERR_OUT_OF_MEMORY;
    }

    if (fread(*image_buffer, *image_size, ONE_ELEMENT, file) != ONE_ELEMENT)
    {
        fclose(file);
        free(*image_buffer);
        return ERR_IO;
    }

    fclose(file);
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

    // Check if arguments are valid
    M_REQUIRE_NON_NULL(argv);

    if (argc != ONE_ELEMENT)
        return ERR_INVALID_COMMAND;

    const char *file_name = argv[FILE_NAME_INDEX];
    M_REQUIRE_NON_NULL(file_name);

    int ret = ERR_NONE;

    // Create a new imgfs_file and initialize its content to zero
    struct imgfs_file file_to_create;
    memset(&file_to_create, 0, sizeof(file_to_create));

    ret = do_open(file_name, "rb", &file_to_create);
    if (ret != ERR_NONE)
        return ret;

    ret = do_list(&file_to_create, STDOUT, NULL);
    if (ret != ERR_NONE)
        return ret;

    do_close(&file_to_create);

    return ret;
}

/**********************************************************************
 * Prepares and calls do_create command.
 ********************************************************************** */
int do_create_cmd(int argc, char **argv)
{
    // puts("Create");
    /* **********************************************************************
     * TODO WEEK 08: WRITE YOUR CODE HERE (and change the return if needed).
     * **********************************************************************
     */

    // Check if arguments are valid
    M_REQUIRE_NON_NULL(argv);
    if (argc == EMPTY)
        return ERR_NOT_ENOUGH_ARGUMENTS;

    const char *imgfs_filename = argv[FILE_NAME_INDEX];
    uint32_t max_files = default_max_files;
    uint16_t thumb_width = default_thumb_res, thumb_height = default_thumb_res,
             small_width = default_small_res, small_height = default_small_res;

    if (argc > ONE_ELEMENT)
    {
        for (int i = 1; i < argc; i++)
        {
            if (!strcmp(argv[i], "-max_files"))
            {
                // Make sure that the "max_files" value is present
                if (i + MAX_FILES_ARG >= argc)
                    return ERR_NOT_ENOUGH_ARGUMENTS;
                max_files = atouint32(argv[i + 1]);
                i += ONE_ELEMENT; // Skip the next argument, since it's the max_files value
            }
            else if (!strcmp(argv[i], "-thumb_res"))
            {
                // Make sure that the "thumb_res" values are present
                if (i + WIDTH_AND_HEIGHT_ARGS >= argc)
                    return ERR_NOT_ENOUGH_ARGUMENTS;
                thumb_width = atouint16(argv[i + 1]);  // Get width value
                thumb_height = atouint16(argv[i + 2]); // Get height value
                i += TWO_ELEMENTS;
            }
            else if (!strcmp(argv[i], "-small_res"))
            {
                // Make sure that the "small_res" values are present
                if (i + WIDTH_AND_HEIGHT_ARGS >= argc)
                    return ERR_NOT_ENOUGH_ARGUMENTS;
                small_width = atouint16(argv[i + 1]);
                small_height = atouint16(argv[i + 2]);
                i += TWO_ELEMENTS;
            }
            else
                return ERR_INVALID_ARGUMENT;
        }
    }
    if (max_files == 0)
        return ERR_INVALID_ARGUMENT;
    if (max_files > default_max_files)
        return ERR_MAX_FILES;
    if (thumb_width <= 0 || thumb_width > MAX_THUMB_RES || thumb_height <= 0 || thumb_height > MAX_THUMB_RES)
        return ERR_RESOLUTIONS;
    if (small_width <= 0 || small_width > MAX_SMALL_RES || small_height <= 0 || small_height > MAX_SMALL_RES)
        return ERR_RESOLUTIONS;

    struct imgfs_header header = {.max_files = max_files,
                                  .resized_res = {thumb_width, thumb_height, small_width, small_height}};

    struct imgfs_file imgfs_file;
    imgfs_file.header = header;

    int return_value = do_create(imgfs_filename, &imgfs_file);
    if (return_value == ERR_NONE)
        do_close(&imgfs_file);
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

    // Check if arguments are valid
    M_REQUIRE_NON_NULL(argv);
    if (argc != TWO_ELEMENTS)
        return ERR_NOT_ENOUGH_ARGUMENTS;

    const char *filename = argv[FILE_NAME_INDEX];
    const char *img_ID = argv[IMG_ID_INDEX];
    if ((img_ID == NULL) || (strlen(img_ID) > MAX_IMG_ID))
        return ERR_INVALID_IMGID;

    struct imgfs_file imgfs_file;
    int ret = ERR_NONE;

    ret = do_open(filename, "r+b", &imgfs_file);
    if (ret != ERR_NONE)
        return ret;

    ret = do_delete(img_ID, &imgfs_file);
    do_close(&imgfs_file);

    return ret;
}

/********************************************************************
 * Read an image from the imgFS.
 *******************************************************************/
int do_read_cmd(int argc, char **argv)
{
    M_REQUIRE_NON_NULL(argv);
    if (argc != 2 && argc != 3)
        return ERR_NOT_ENOUGH_ARGUMENTS;

    const char *const img_id = argv[1];

    const int resolution = (argc == 3) ? resolution_atoi(argv[2]) : ORIG_RES;
    if (resolution == -1)
        return ERR_RESOLUTIONS;

    struct imgfs_file myfile;
    zero_init_var(myfile);
    int error = do_open(argv[0], "rb+", &myfile);
    if (error != ERR_NONE)
        return error;

    char *image_buffer = NULL;
    uint32_t image_size = 0;
    error = do_read(img_id, resolution, &image_buffer, &image_size, &myfile);
    do_close(&myfile);
    if (error != ERR_NONE)
    {
        return error;
    }

    // Extracting to a separate image file.
    char *tmp_name = NULL;
    create_name(img_id, resolution, &tmp_name);
    if (tmp_name == NULL)
        return ERR_OUT_OF_MEMORY;
    error = write_disk_image(tmp_name, image_buffer, image_size);
    free(tmp_name);
    free(image_buffer);

    return error;
}

/********************************************************************
 * Insert an image into the imgFS.
 *******************************************************************/
int do_insert_cmd(int argc, char **argv)
{
    M_REQUIRE_NON_NULL(argv);
    if (argc != 3)
        return ERR_NOT_ENOUGH_ARGUMENTS;

    struct imgfs_file myfile;
    zero_init_var(myfile);
    int error = do_open(argv[0], "rb+", &myfile);
    if (error != ERR_NONE)
        return error;

    char *image_buffer = NULL;
    uint32_t image_size;

    // Reads image from the disk.
    error = read_disk_image(argv[2], &image_buffer, &image_size);
    if (error != ERR_NONE)
    {
        do_close(&myfile);
        return error;
    }

    error = do_insert(image_buffer, image_size, argv[1], &myfile);
    free(image_buffer);
    do_close(&myfile);
    return error;
}


