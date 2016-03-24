/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   main.cpp
 * Author: roman
 *
 * Created on March 15, 2016, 3:35 PM
 */



#include <cstdlib>
#include <string>
#include <iostream>
#include <fstream>
#include <map>
#include <sqlite3.h>
#include <exception>
#include <sys/stat.h>
#include <sys/types.h>

using namespace std;

static int print_callback(void *notused, int coln, char **rows, char **colnm)
{
    cout << "| ";
    for (int i = 0; i < coln; i ++)
    {
        if (string(colnm[i]) == "path")
        {
            string temp(rows[i]);
            ifstream in;
            in.open("config.txt", fstream::in);
            string rootdir, mountdir;
            in >> rootdir >> mountdir;
            in.close();
            temp.erase(0, rootdir.size());
            temp = mountdir + temp;
            cout << string(colnm[i]) + " = " + temp + " | ";
        }
        else
            cout << string(colnm[i]) + " = " + string(rows[i]) + " | ";
    }
    cout << endl;
    
    return 0;
}

static int prm_callback(void *notused, int coln, char **rows, char **colnm)
{
    char *temp = (char*)notused;
    sprintf(temp, "%s", rows[coln - 1]);
    
    return 0;
}

int get_op_num(const string op){
    map<string, int> operations = {{"getattr", 0},{"readlink", 1},{"mknod", 2},{"mkdir", 3},{"unlink", 4},
                            {"rmdir", 5},{"symlink", 6},{"rename", 7},{"link", 8},{"chmod", 9},
                            {"chown", 10},{"truncate", 11},{"utime", 12},{"open", 13},{"read", 14},
                            {"write", 15},{"statfs", 16},{"flush", 17},{"release", 18},{"fsync", 19},
                            {"opendir", 20},{"readdir", 21},{"releasedir", 22},{"fsyncdir", 23},{"init", 24},
                            {"destroy", 25},{"access", 26},{"create", 27},{"ftruncate", 28},{"fgetattr", 29}};
    if (operations.find(op) == operations.end())
        return -1;
    else
        return operations[op];
}

int help()
{
    cout << "--h --help: printing this message" <<
            "--configure: \n Command receives absolutely path to root, mount directories \nand path to data base created during mounting FACFS in same directory with mounting program \nas arguments for making a config file in program directory.\n" <<
            "You have to --configure every time you move this program \nor make change in paths.\n" <<
            "--show:\n The command receives absolute or relative to mount directory \npath of file or directory in mount directory as argument for printing \n" <<
            "information of permissions belongs to given file or directory. \nAlso command could take \"all\" as argument for printing all information data base contains.\n" <<
            "--set:\n That command you could use for setting individual permissions \nor changing owner and not owner permissions by sending a permission mask.\nCommand receives three arguments in strong order.\nAbsolute or relative to mount directory path of file \nor directory in mount directory as first argument,\n" <<
            "uid of user as second and permission mask as third. \nYou could use -4 and -5 as uid for setting owner and not owner permissions. \nFor directories path with \"/\" in the end is responsible for permissions of operations inside the directory \nwhereas path with no \"/\" is for permissions of operaions with the directory\n" <<
            "Permission mask is string that contains only plus and minus symbols and must be 30 symbols long.\nCommand is stable to wrong arguments and lets you know what wrong you do.\n" <<
            "--c --change\n Receives path, uid, list of \"+-permission type\"\n\n";
    return 0;
}

int configure(int n, char **paths)
{
    if (n < 4)
    {
        cout << "Not enough arguments to config." << endl;
        return 0;
    }
    
    ofstream out;
    out.open("config.txt", fstream::out);
    string rootdir(paths[1]), mountdir(paths[2]), db(paths[3]);
    if (rootdir[rootdir.length() - 1] != '/')
        rootdir += '/';
    if (mountdir[mountdir.length() - 1] != '/')
        mountdir += '/';
    out << rootdir << endl << mountdir << endl << db;
    out.close();
    
    cout << "Configuration successful" << endl;
    return 0;
}

int check_path(string &path)
{
    ifstream in;
    in.open("config.txt", fstream::in);
    string rootdir, mountdir;
    in >> rootdir >> mountdir;
    if (rootdir == "" || mountdir == "")
    {
        cout << "You have to configure firstly" << endl;
        return 0;
    }
    in.close();
    
    struct stat *stat_buf = new struct stat;
    if (lstat(path.c_str(), stat_buf) != 0)
    {
        cout << "Wrong path" << endl;
        return 0;
    }
    
    string::size_type pos = path.find(mountdir);
    if (!(pos == string::npos))
        path.erase(pos, mountdir.size());
    else
    {
        mountdir.erase(mountdir.length() - 1);
        pos = path.find(mountdir);
        if (!(pos == string::npos))
            path.erase(pos, mountdir.size());
        else
        {
            cout << "Wrong path" << endl;
            return 0;
        }
    }
    path = rootdir + path;
    
    delete stat_buf;
    
    return 1;
}

int check_uid(string uid)
{
    for (int i = 0; i < uid.length(); i++)
    {
        if (!(isdigit(uid[i])||(i == 0 && uid[i] == '-')))
        {
            cout << "Wrong uid" << endl;
            return 0;
        }
    }
    return 1;
}

int check_prms(string prms)
{
    if (prms.length() != 30)
    {
        cout << "Wrong prms" << endl;
        
        return 0;
    }
    for (int i = 0; i < prms.length(); i ++)
    {
        if (prms[i] != '+' && prms[i] != '-')
        {
            cout << "Wrong prms" << endl;
            
            return 0;
        }
    }
    
    return 1;
}

int db_open(sqlite3 *&db)
{
    ifstream in;
    string rootdir, mountdir, dbpath;
    
    in.open("config.txt", fstream::in);
    in >> rootdir >> mountdir >> dbpath;
    if (rootdir == "" || mountdir == "" || dbpath == "")
    {
        cout << "You have to configure firstly" << endl;
        
        return 0;
    }
    
    in.close();
    
    if (sqlite3_open(dbpath.c_str(), &db))
    {
        cout << "There is an error with opening data base. Please try to reconfigure";
        
        return 0;
    }
    
    return 1;
}

int show(int n, char **path){
    sqlite3 *db = NULL;
    
    if (!db_open(db))
        return 0;
    
    string query, query2("");
    char *err = 0;
    
    if (n < 2 || string(path[1]) == "all")
        query = "select * from prm_list;";
    else
    {
        string p(path[1]);
        
        if (!check_path(p))
            return 0;
        
        query = "select * from prm_list where path =\"" + p + "\";";
        query2 = "select * from prm_list where path =\"" + p + "/\";";
    }
    
    if (!(sqlite3_exec(db, query.c_str(), print_callback, 0, &err) == SQLITE_OK))
    {
        cout << "Wrong data base path in config.txt. Please try to reconfigure" << endl;
        return 0;
    }
    
    if (query2 != "")
        sqlite3_exec(db, query2.c_str(), print_callback, 0, &err);
    
    sqlite3_close(db);
}



int set(int n, char **argv)
{
    sqlite3 *db;
    
    if (n < 4)
    {
        cout << "Not enough arguments" << endl;
        return 0;
    }
    
    string path(argv[1]), uid(argv[2]), prms(argv[3]);
    
    if (!check_path(path) || !check_uid(uid) || !check_prms(prms) || !db_open(db))
        return 0;
    
    string query = "insert or replace into prm_list values(\"" + path +"\", " + uid + ", \"" + prms + "\");";
    char *err = 0;
    if (!(sqlite3_exec(db, query.c_str(), 0, 0, &err) == SQLITE_OK))
    {
        cout << "Something went wrong" << endl;
        return 0;
    }
    
    //cout << "Success" << endl;
    sqlite3_close(db);
    
    return 0;
}

int change(int n, char **argv)
{
    sqlite3 *db = NULL;
    string path, uid, op;
    
    if (n < 3)
        return 0;
    
    path = string(argv[1]);
    uid = string(argv[2]);
    
    if (!check_path(path) || !check_uid(uid) || !db_open(db))
        return 0;
    
    for (int i = 3; i < n; i++)
    {
        op = string(argv[i]);
        char prm = op[0];
        op.erase(0, 1);
        int op_num = get_op_num(op);
        
        if(op_num == -1 || (prm != '+' && prm != '-'))
            continue;
        
        string query = "select exists(select * from prm_list where path = \"" + path + "\" and uid = " + uid + ");";
        char * res = new char[500], *err = 0;
        if (sqlite3_exec(db, query.c_str(), prm_callback, res, &err) != SQLITE_OK)
        {
            continue;
        }
        
        if (res[0] == '1')
        {
            query = "select * from prm_list where path = \"" + path + "\" and uid = " + uid + ";";
            sqlite3_exec(db, query.c_str(), prm_callback, res, &err);
            res[op_num] = prm;
            query = "update prm_list set prms = \"" + string(res) + "\" where path = \"" + path + "\" and uid = " + uid + ";";
            sqlite3_exec(db, query.c_str(), 0, 0, &err);
        }
        else
        {
            struct stat *statbuf = new struct stat;
            lstat(argv[1], statbuf);
            if (statbuf->st_uid == atoi(uid.c_str()))
                query = "select * from prm_list where path = \"" + path + "\" and uid = -4;";
            else
                query = "select * from prm_list where path = \"" + path + "\" and uid = -5;";
            
            delete statbuf;
            
            sqlite3_exec(db, query.c_str(), prm_callback, res, &err);
            res[op_num] = prm;
            query = "insert or replace into prm_list values(\"" + path + "\", " + uid + ", \"" + res + "\");";
            sqlite3_exec(db, query.c_str(), 0, 0, &err);
        }
        
        delete [] res;
    }
    
    return 0;
}

/*
 * 
 */
int main(int argc, char **argv) 
{
    argv ++;
    argc --;
    if (argc < 1)
        return 0;
    
    map <string, int> comands = {{"--configure", 1},
                                {"--help", 0},
                                {"--h", 0},
                                {"--show", 2},
                                {"--set", 3},
                                {"--c", 4},
                                {"--change", 4}};
    if (comands.find(string(argv[0])) == comands.end())
    {
        cout << "Undefined operation" << endl;
        return 0;
    }
    switch (comands[string(argv[0])])
    {
        case 0:
            return help();
        case 1:
            return configure(argc, argv);
        case 2:
            return show(argc, argv);
        case 3:
            return set(argc, argv);
        case 4:
            return change(argc, argv);
        default:
            return 0;
    }
    
    return 0;
}


