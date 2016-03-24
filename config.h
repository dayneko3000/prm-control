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

#ifndef CONFIG_H
#define CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

    
    
std::map<std::string, int> operations = {{"getattr", 1 << 0},{"readlink", 1 << 1},{"mknod", 1 << 2},{"mkdir", 1 << 3},{"unlink", 1 << 4},
                            {"rmdir", 1 << 5},{"symlink", 1 << 6},{"rename", 1 << 7},{"link", 1 << 8},{"chmod", 1 << 9},
                            {"chown", 1 << 10},{"truncate", 1 << 11},{"utime", 1 << 12},{"open", 1 << 13},{"read", 1 << 14},
                            {"write", 1 << 15},{"statfs", 1 << 16},{"flush", 1 << 17},{"release", 1 << 18},{"fsync", 1 << 19},
                            {"opendir", 1 << 20},{"readdir", 1 << 21},{"releasedir", 1 << 22},{"fsyncdir", 1 << 23},{"init", 1 << 24},
                            {"destroy", 1 << 25},{"access", 1 << 26},{"create", 1 << 27},{"ftruncate", 1 << 28},{"fgetattr", 1 << 29}};

#ifdef __cplusplus
}
#endif

#endif /* CONFIG_H */

