# General

This package demonstrates the use of gRPC to store and retrieve potentialy large files. It comprises a client and a server.
To upload a file the client has to specified a numeric id and a name, and the server saves it by the
given name. To retrieve the file, the server needs to specify the name.

# Prerequisites

Building requires
* a fairly recent installation of Google RPC
* Protobuf that supports version 3 of the format
* grpc_cpp_plugin
* G++ that supports C++ 14
* bash ≥ 3.2 to run the demo script

The program was tested on MacOS 10.4 ("Mojave"). It should work on other POSIX systems as well.

# Building

Run:
```bash
make clean && make -j4  # Use a higher number of jobs if you prefer
```

# Running

Choose a file on your system to use for the demonsration. The file has to be readable.
It will not be otherwise affected. To run the demo script

```bash
./demo YOUR_FILE
```

This will launch the server, upload the file and then retrieve it, and compare the resulting files
to the original file.

To run manually:
* Launch the server. Example command to run it in the background:
```bash
( cd data && exec ../file_exchange_server ) &
```
* Launch the client in a working directory other than the one where the server runs e.g.:
```bash
# Upload to the server
./file_exchange_client put 1 ~/Downloads/SOME_FILE
# Get the file into the working directory of the file_exchange_client
./file_exchange_client get 1
# Attempt to get a non-existing file, which will cause an error
./file_exchange_client get 9
```