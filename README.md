# ImgFS: Image-oriented File System

## Project Background
This project involves developing a large program in C with a "system" theme. The focus is on building a command-line utility to manage images within a specific format file system inspired by Facebook's "Haystack" system. The goal is to handle a high volume of images efficiently, with support for multiple resolutions and deduplication of identical images.

## Project Goals
- Implement a simplified version of the Haystack-inspired image server.
- Develop basic functions: list data, add images, delete images, extract images in different resolutions (original, small, thumbnail).
- Initially expose these functions through a command line interface (CLI).
- Extend the functionality to include a web server for image distribution using the HTTP protocol.

## General Description

### Context
Social networks manage millions of images, posing challenges for usual file systems. The "Haystack" approach, used by Facebook, addresses these challenges by storing multiple images in a single file, managing different resolutions automatically, and deduplicating identical images.

### Key Features
1. **Automatic Management of Multiple Resolutions**: Store different resolutions of the same image efficiently.
2. **Deduplication**: Avoid duplicating identical images using a hash function (SHA-256).

### Goals
The image server provides the following capabilities:
1. List data (metadata, image list)
2. Add a new image
3. Delete an image
4. Extract an image in a specific resolution (original, small, thumbnail)

## Data Format

### ImgFS File Structure
An `imgfs` file consists of three parts:
1. **Header**: Contains configuration data, created during `imgfs` creation.
2. **Metadata Array**: Describes the metadata of each image.
3. **Images**: Stored contiguously in the file.

### Data Structures

#### Header (`struct imgfs_header`)
- **name**: Name of the database.
- **version**: Database version, incremented after each insertion/deletion.
- **nb_files**: Current number of images.
- **max_files**: Maximum number of images.
- **resized_res**: Resolutions for "thumbnail" and "small" images.
- **unused_32**: Unused 32-bit field for future use.
- **unused_64**: Unused 64-bit field for future use.

#### Metadata (`struct img_metadata`)
- **img_id**: Unique identifier for the image.
- **SHA**: Image hash code.
- **orig_res**: Resolution of the original image.
- **size**: Memory sizes of images at different resolutions.
- **offset**: Positions in the file for images at various resolutions.
- **is_valid**: Indicates whether the image is in use.
- **unused_16**: Unused 16-bit field for future use.

#### ImgFS File (`struct imgfs_file`)
- **file**: File pointer to the image database file.
- **header**: General information of the image database.
- **metadata**: Dynamic array of image metadata.

## How to Run

### Command Line Interface (CLI)
A CLI tool is developed to:
- List images
- Add new images
- Delete images
- Extract images in specified resolutions

### Web Server
A web server is built to distribute images over the network using the HTTP protocol.

![image](https://github.com/user-attachments/assets/7a33e356-764b-4ef5-b0bb-1e40b87e014f)

Figure 1: Web Interface for ImgFS Project


## Compilation
To compile the project, simply run:

```bash
make
```

## Special Features
There were issues when wanting to restart the server on the same port, requiring a wait time before reusing the port. To solve this problem, we added a feature that modifies the socket settings in the `tcp_server_init()` method in the `socket_layer.c` file. This feature can be enabled by defining the MACRO using the `-SOCKET_REUSE` flag in the `Makefile`.

### Enabling Socket Reuse
To enable the socket reuse feature, you need to define the `-SOCKET_REUSE` flag in the `Makefile`.

Include the following in the `Makefile`:

```makefile
CFLAGS += -DSOCKET_REUSE
```

## Conclusion
This project provides hands-on experience in building a large-scale image management system in C, covering various aspects from command-line utilities to web server implementation.
