/**
 * @file imgfscmd.c
 * @brief imgFS command line interpreter for imgFS core commands.
 *
 * Image Filesystem Command Line Tool
 *
 * @author Mia Primorac
 */

#include "imgfs.h"
#include "imgfscmd_functions.h"
#include "util.h" // for _unused

#include <stdlib.h>
#include <string.h>
#include <vips/vips.h>

#define NB_COMMANDS 6
#define FIRST_ARG 1

typedef int (*command)(int argc, char *argv[]);
typedef struct
{
    const char *command_name;
    command command;
} command_mapping;

command_mapping commands[NB_COMMANDS] = {{"list", do_list_cmd},
                                         {"create", do_create_cmd},
                                         {"insert", do_insert_cmd},
                                         {"read", do_read_cmd},
                                         {"delete", do_delete_cmd},
                                         {"help", help}};

/*******************************************************************************
 * MAIN
 */
int main(int argc, char *argv[])
{
    int ret = 0;

    if (VIPS_INIT(argv[0]))
    {
        // Handle initialization failure
        ret = ERR_IMGLIB;
        fprintf(stderr, "ERROR: %s\n", ERR_MSG(ret));
        return ret;
    }

    if (argc < 2)
    {
        ret = ERR_NOT_ENOUGH_ARGUMENTS;
    }
    else
    {
        /* **********************************************************************
         * TODO WEEK 07: THIS PART SHALL BE EXTENDED.
         * **********************************************************************
         */

        char *current_command = argv[FIRST_ARG];
        int command = NOT_FOUND;

        // Iterate through all commands, and if valid call corresponding function
        for (int i = 0; i < NB_COMMANDS; i++)
        {
            if (!strcmp(current_command, commands[i].command_name))
            {
                command = FOUND;
                ret = commands[i].command(argc - 2, argv + 2); // Call function with one argument corresponding to the second argument of the current process
            }
        }
        if (command == NOT_FOUND)
        {
            ret = ERR_INVALID_COMMAND;
        }
        // argc--;  argv++; // skips command call name
    }

    if (ret)
    {
        fprintf(stderr, "ERROR: %s\n", ERR_MSG(ret));
        help(argc, argv);
    }
    vips_shutdown();
    return ret;
}
