# Huawei Software Code Craft 2025
---
This project is Chen's work for the competition of Huawei Software Code Craft in March 2025. You can check the details about this competition [here](https://developer.huaweicloud.com/codecraft2025).

### Structure and Compile
Docker is recommended for the competition has some limits on the running environment of the craft, more specific, a Linux system with 4 CPUs and 8GB memory is necessary. File *Dockerfile* and *compose.yml* can be used to implement this environment. Use the following command to build and enter the docker.

```
docker compose up -d --build
docker exec -it huawei-chen-1 /bin/bash
```

This project is compiled by cmake. You can just run *make.sh* to compile this project in Docker container.

The committee provided with some test data and useful debug tools, the data are in *data* folder. *interactor* folder provide some exe for checking your running results with correct answer. You can simply run *run.sh* to start the program without check, or run *check.sh* to run and check the output of your program with the help of *run.py*. More specific informations about this competition can be found in *doc* folder (strongly recommended to read).

### Framework

This disk-simulator system consists of three levels:
- **Disk**, a circular linked list, contains a number of **Block**, are the bottom level of the storage framework.
- **StorageManager** is used to manage all disks, it locates on the middle level.
- **Schema** offers high level interface of operations, like *insert*, *delete* and *read*. There operations are passed level-by-level, from Schema to StorageManager, then to Disk.

There are three kinds of operations:

- **Delete** and **Write** are simply write or delete some entries into the storage.
- **Read** is the most important operation in this design. Details about it can be found in *doc* folder.

### Efficiency Analysis

Todo:

1. For now, the bottolneck of running time lies in the function **waitList_dist_update**. seems that no more optimize can be done except change the global logic of read.

2. The *Tag* of an object has not been used in this version of program, but it is forseen important for the total score.