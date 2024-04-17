#include "imgfs.h"
#include <stdio.h> 

/**********************************************************************
 * Creates a new database for the imgfs format.
 ********************************************************************** */
int do_create(const char* imgfs_filename, struct imgfs_file* imgfs_file){
    M_REQUIRE_NON_NULL(imgfs_filename);
    M_REQUIRE_NON_NULL(imgfs_file);

    //initialise header  //TODO allocate memory before?
    strcpy(imgfs_file->header.name, CAT_TXT);
    imgfs_file->header.version = 0;
    imgfs_file->header.nb_files = 0;
    
    imgfs_file->metadata = calloc(imgfs_file->header.max_files, sizeof(struct img_metadata));
    if (imgfs_file->metadata == NULL) return ERR_OUT_OF_MEMORY;
    memset(imgfs_file->metadata, 0, sizeof(imgfs_file->metadata)); // make sure isvalid = 0


    //write imgfs_file to imgfs_filename in database
    FILE* imgfs = fopen(imgfs_filename, "wb");
    if (imgfs == NULL) return ERR_IO;

    if(fwrite(&(imgfs_file->header), sizeof(imgfs_file->header), 1, imgfs) != 1) return ERR_IO;
    if(fwrite(&(imgfs_file->metadata), sizeof(imgfs_file->metadata), 1, imgfs) != 1) return ERR_IO;

    if(fclose(imgfs)) return ERR_IO;

    // print number of elements written
    printf("%d item(s) written", 1 + imgfs_file->metadata->size); //TODO 

    return ERR_NONE;
}