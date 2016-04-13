/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   config.h
 * Author: roman
 *
 * Created on March 24, 2016, 3:59 PM
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

#ifndef CONFIG_H
#define CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

#define QUERY_MAX 1000
#define PRM_MAX 50    
    
std::map<std::string, int> operations = 
{{"execute", 1 << 0}, {"getattr", 1 << 1}, {"chmod", 1 << 2}, {"fsyncdir", 1 << 3}, {"truncate", 1 << 4},
{"chown", 1 << 5}, {"fsync", 1 << 6}, {"utime", 1 << 7}, {"readlink", 1 << 8}, {"unlink", 1 << 9},
{"symlink", 1 << 10}, {"rename", 1 << 11}, {"link", 1 << 12}, {"ftruncate", 1 << 13}, {"fgetattr", 1 << 14},
{"open", 1 << 15}, {"read", 1 << 16}, {"write", 1 << 17}, {"statfs", 1 << 18}, {"rmdir", 1 << 19},
{"mknod", 1 << 20}, {"mkdir", 1 << 21}, {"opendir", 1 << 22}, {"readdir", 1 << 23}, {"create", 1 << 24}};

#ifdef __cplusplus
}
#endif

#endif /* CONFIG_H */

