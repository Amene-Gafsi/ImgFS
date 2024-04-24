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
#include <vips/vips8>  //TODO 

#define NB_COMMANDS 4

/*******************************************************************************
 * MAIN
 */
typedef int (*command)(int argc, char *argv[]);
typedef struct
{
    const char *command_name;
    command command;
} command_mapping;

int main(int argc, char *argv[])
{
    VIPS_INIT(argv[0]);
    int ret = 0;

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
        const int FOUND = 1, NOT_FOUND = 0;
        command_mapping list = {"list", do_list_cmd};
        command_mapping create = {"create", do_create_cmd};
        command_mapping help_command = {"help", help};
        command_mapping delete = {"delete", do_delete_cmd};

        command_mapping commands[NB_COMMANDS] = {list, create, help_command, delete};
        char *current_command = argv[1];
        int command = NOT_FOUND;
        for (int i = 0; i < NB_COMMANDS; i++)
        {
            if (!strcmp(current_command, commands[i].command_name))
            {
                command = FOUND;
                ret = commands[i].command(argc - 2, argv + 2); // call function with one argument corresponding to the second argument of the current process
            }
        }
        if (command == NOT_FOUND)
        {
            ret = ERR_INVALID_COMMAND;
        }
    }

    if (ret)
    {
        fprintf(stderr, "ERROR: %s\n", ERR_MSG(ret));
        help(argc, argv);
    }
    vips_shutdown();
    return ret;
}
