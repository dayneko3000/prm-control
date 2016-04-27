/*
  Copyright (C) 2016 Roman Y. Dayneko, <dayneko3000@gmail.com>

  This program can be distributed under the terms of the GNU GPLv3.
  See the file COPYING.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>
#include <fstream>
#include <map>
#include <sqlite3.h>
#include <exception>
#include <sys/stat.h>
#include <sys/types.h>
#include <bitset>
#include <unistd.h>
#include <mntent.h>

#ifndef CONFIG_H
#define CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif
    
static sqlite3 *db = nullptr;    
//std::string rootdir, mountdir;

#define PREMISSIONS_SIZE 30
#define QUERY_MAX 1000
#define PATH_MAX 500
#define PRM_MAX 50
#define DEEP_MAX 200
#define UID_MAX 50
#define EXIST_MAX 2
    
#define MODE_MASK 65024   

#define FULL_PRMS 67108863
#define READ_ONLY_PRMS 29746687

#define EXECUTE (1 << 0)
#define GETATTR (1 << 1)
#define CHMOD (1 << 2)
#define FSYNCDIR (1 << 3)
#define TRUNCATE (1 << 4)
#define CHOWN (1 << 5)
#define FSYNC (1 << 6)
#define UTIME (1 << 7)
#define READLINK (1 << 8)
#define UNLINK (1 << 9)
#define SYMLINK (1 << 10)
#define RENAME (1 << 11)
#define LINK (1 << 12)
#define FTRUNCATE (1 << 13)
#define FGETATTR (1 << 14)
#define OPEN (1 << 15)
#define READ (1 << 16)
#define WRITE (1 << 17)
#define STATFS (1 << 18)
#define RMDIR (1 << 19)
#define MKNOD (1 << 20)
#define MKDIR (1 << 21)
#define OPENDIR (1 << 22)
#define READDIR (1 << 23)
#define CREATE (1 << 24)
    
static std::map<std::string, int> operations = 
    {{"execute", 1 << 0}, 
    {"getattr", 1 << 1}, 
    {"chmod", 1 << 2}, 
    {"fsyncdir", 1 << 3}, 
    {"truncate", 1 << 4},
    {"chown", 1 << 5}, 
    {"fsync", 1 << 6}, 
    {"utime", 1 << 7}, 
    {"readlink", 1 << 8}, 
    {"unlink", 1 << 9},
    {"symlink", 1 << 10}, 
    {"rename", 1 << 11}, 
    {"link", 1 << 12}, 
    {"ftruncate", 1 << 13}, 
    {"fgetattr", 1 << 14},
    {"open", 1 << 15}, 
    {"read", 1 << 16}, 
    {"write", 1 << 17}, 
    {"statfs", 1 << 18}, 
    {"rmdir", 1 << 19},
    {"mknod", 1 << 20}, 
    {"mkdir", 1 << 21}, 
    {"opendir", 1 << 22}, 
    {"readdir", 1 << 23}, 
    {"create", 1 << 24}};

static std::map <std::string, int> comands = 
    {{"--help", 0},
    {"--h", 0},
    {"--cfg", 1},
    {"--configure", 1},
    {"--show", 2},
    {"--set", 3},
    {"--c", 4},
    {"--change", 4}};

int help();
int configure(std::string);
int config();
int show(int, char **);
int set(int, char **);
int change(int, char **);

#ifdef __cplusplus
}
#endif

#endif /* CONFIG_H */

