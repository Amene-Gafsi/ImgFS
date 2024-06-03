## Image-oriented File System 

### Goal of the Project
The goal of this project is to build an image server inspired by and simplified from Haystack. 

### Progress
We have completed the entire project. The project compiles successfully using the `make` command.

#### Compilation
To compile the project, simply run:
```sh
make
```

### Special Features
There were issues when wanting to restart the server on the same port, requiring a wait time before reusing the port. To solve this problem, we added a feature that modifies the socket settings in the *tcp_server_init()* method in the *socket_layer.c* file. This feature can be enabled by defining the MACRO using the `-SOCKET_REUSE` flag in the Makefile.

#### Enabling Socket Reuse
To enable the socket reuse feature, you need to define the `-SOCKET_REUSE` flag in the Makefile.

Include the following in the Makefile:
```makefile
CFLAGS += -DSOCKET_REUSE
```



