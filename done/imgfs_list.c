#include "imgfs.h"
#include <stdio.h>
#include "util.h"
#include <json-c/json.h>

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
        json_object *jobj = json_object_new_object();
        if (jobj == NULL)
        {
            return ERR_RUNTIME;
        }
        json_object *jarray = json_object_new_array();
        if (jarray == NULL)
        {
            json_object_put(jobj);
            return ERR_RUNTIME;
        }

        for (uint32_t i = 0; i < imgfs_file->header.max_files; i++)
        {
            if (imgfs_file->metadata 
            != NULL && (imgfs_file->metadata)[i].is_valid)
            { // TODO : is valid
                json_object *jstring = json_object_new_string((imgfs_file->metadata)[i].img_id);
                if (jstring == NULL || json_object_array_add(jarray, jstring) < 0)
                {
                    json_object_put(jobj);
                    json_object_put(jarray);
                    if (jstring != NULL) json_object_put(jstring);
                    return ERR_RUNTIME;
                }
            }
        }

        if (json_object_object_add(jobj, "Images", jarray) < 0)
        {
            json_object_put(jobj);
            json_object_put(jarray);
            return ERR_RUNTIME;
        }
        
        *json = strdup(json_object_to_json_string(jobj));
        json_object_put(jobj);
        if (*json == NULL)
        {
            return ERR_RUNTIME;
        }

        return ERR_NONE;
    }
    else

        return ERR_IO;
}
