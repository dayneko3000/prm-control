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

#include "config.h"

using namespace std;

sqlite3 *db = NULL;
string rootdir, mountdir;

static int print_callback(void *notused, int coln, char **rows, char **colnm)
{
    cout << "| ";
    for (int i = 0; i < coln; i ++)
    {
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

static int callback(void *notused, int coln, char **rows, char **colnm)
{
    string * t = (string *)notused;
    for (int i = 0; i < coln; i ++)
        t[i] = string(rows[i]);
    return 0;
}

int get_op_num(const string op){
    
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

int configure(char *db_path)
{
    if (!(sqlite3_open(realpath(db_path, NULL), &db) == SQLITE_OK))
    {
        cout << "Wrong path to database" << endl;
        abort();
    }
    
    char *err = NULL;
    string query = "select * from dir_list;";
    string res[2] = {"",""};
    sqlite3_exec(db, query.c_str(), callback, res, &err);
    
    if (res[0] == "" || res[1] == "" || err != NULL)
    {
        cout << "Wrong path to database" << endl;
        abort();
    }
    mountdir = res[0];
    rootdir = res[1];
    
    if (mountdir[mountdir.length() - 1] != '/')
        mountdir += "/";
    if (rootdir[rootdir.length() - 1] != '/')
        rootdir += "/";
    
    return 0;
}

int check_path(string path)
{
    struct stat *stat_buf = new struct stat;
    if (lstat((rootdir + path).c_str(), stat_buf) != 0)
    {
        cout << "Wrong path" << endl;
        return 0;
    }
    
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
    for (int i = 0; i < prms.length(); i ++)
    {
        if (prms[i] != '0' && prms[i] != '1')
        {
            cout << "Wrong permission mask" << endl;
            return 0;
        }
    }
    
    return 1;
}

int show(int n, char **path){
    string query, query2;
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
    sqlite3_exec(db, query2.c_str(), print_callback, 0, &err);
    
    sqlite3_close(db);
}



int set(int n, char **argv)
{
    if (n < 4)
    {
        cout << "Not enough arguments" << endl;
        return 0;
    }
    
    string path(argv[1]), uid(argv[2]), prms(argv[3]);
    
    if (!check_path(path) || !check_uid(uid) || !check_prms(prms))
        return 0;
    
    string query = "insert or replace into prm_list values(\"" + path +"\", " + uid + ", \"" + prms + "\");";
    char *err = 0;
    if (!(sqlite3_exec(db, query.c_str(), 0, 0, &err) == SQLITE_OK))
    {
        cout << "Something went wrong" << endl;
        return 0;
    }
    
    //cout << "Success" << endl;
    return 0;
}

int get_new_prm(int prm, int bit, int op)
{
    if (op)
    {
        return prm | bit;
    }
    else
    {
        if (prm & bit == prm)
            return prm - bit;
        else
            return prm;
    }
}

int change(int n, char **argv)
{
    string path, uid, op;
    
    if (n < 3)
        return 0;
    
    path = string(argv[1]);
    uid = string(argv[2]);
    
    if (!check_path(path) || !check_uid(uid))
        return 0;
    
    for (int i = 3; i < n; i++)
    {
        op = string(argv[i]);
        int prm = op[0] == '+' ? 1 : 0;
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
            //res[op_num] = prm;
            char *new_prm = new char [100];
            sprintf(new_prm, "%d", get_new_prm(atoi(res), op_num, prm));
            query = "update prm_list set prms = \"" + string(new_prm) + "\" where path = \"" + path + "\" and uid = " + uid + ";";
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
            char *new_prm = new char [100];
            sprintf(new_prm, "%d", get_new_prm(atoi(res), op_num, prm));
            query = "insert or replace into prm_list values(\"" + path + "\", " + uid + ", \"" + string(new_prm) + "\");";
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
    
    map <string, int> comands = {{"--help", 0},
                                {"--h", 0},
                                {"--show", 2},
                                {"--set", 3},
                                {"--c", 4},
                                {"--change", 4}};
    
    configure(argv[0]);
    
    if (comands.find(string(argv[1])) == comands.end())
    {
        cout << "Undefined operation" << endl;
        return 0;
    }
    
    switch (comands[string(argv[1])])
    {
        case 0:
            return help();
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


