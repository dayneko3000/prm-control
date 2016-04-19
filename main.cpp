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

static int print_prm_callback(void *notused, int coln, char **rows, char **colnm)
{
    cout << "| ";
    for (int i = 0; i < coln; i ++)
    {
//        if (string(colnm[i]) == "prms")
//            cout << string(colnm[i]) + " = " + string(rows[i]) + " = "  + bitset<27>(atoi(rows[i])).to_string() << " |";
//        else
            cout << string(colnm[i]) + " = " + string(rows[i]) + " | ";
    }
    cout << endl;
    
    return 0;
}

static int print_callback(void *notused, int coln, char **rows, char **colnm)
{
    cout << "| ";
    for (int i = 0; i < coln; i ++)
    {
//        if (string(colnm[i]) == "owner_prms" || string(colnm[i]) == "group_prms" || string(colnm[i]) == "other_prms")
//            cout << string(colnm[i]) + " = " + string(rows[i]) + " = "  + bitset<27>(atoi(rows[i])).to_string() << " |";
//        else
            cout << string(colnm[i]) + " = " + string(rows[i]) + " | ";
    }
    cout << endl << "users:" << endl;
    
    char *err = NULL;
    string query = "select id, prms from user_prm_list where file_id = " + string(rows[0]) + ";";
    sqlite3_exec(db, query.c_str(), print_prm_callback, NULL, &err);
    cout << "groups:" << endl;
    query = "select id, prms from group_prm_list where file_id = " + string(rows[0]) + ";";
    sqlite3_exec(db, query.c_str(), print_prm_callback, NULL, &err);
    
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

int get_op_num(const string op)
{
    if (operations.find(op) == operations.end())
        return -1;
    else
        return operations[op];
}

int help()
{
    cout << "[command <command args>]\n" << 
            "commands:\n"
            "--h --help:                                                   printing this message\n" <<
            "--show [path to database, \"\"|\"all\"|file/directory path]:      printing information db contains\n" <<
            "--set [path to database, option, <option args>]:              set permissions of file by permission mask\n\toptions:\n" << 
            "\t-ow [file/directory path, permission mask]:           set permissions for file owner\n" <<
            "\t-gow [file/directory path, permission mask]:          set permissions for file group\n" <<
            "\t-oth [file/directory path, permission mask]:          set permissions for other\n" <<
            "\t-g [file/directory path, gid, permission mask]:       set individual permissions for group\n" <<
            "\t-u [file/directory path, uid, permission mask]:       set individual permissions for user\n" <<
            "example: --set /home/user/Desktop/FTDB.db -u /abc.txt 1000 67108863\n\n" <<
            "--c --change [path to database, option, <option args>]:       add/remove permission by type\n\toptions:\n" <<
            "\t-ow [file/directory path, <+-permission type>]:       change permissions of owner\n" <<
            "\t-gow [file/directory path, <+-permission type>]:      change permissions of file group\n" <<
            "\t-oth [file/directory path, <+-permission type>]:      change permissions of other\n" <<
            "\t-g [file/directory path, gid, <+-permission type>]:   change individual permissions of group\n" <<
            "\t-u [file/directory path, uid, <+-permission type>]:   change individual permissions of user\n" <<
            "example: --c /home/user/Desktop/FTDB.db -u /abc.txt 1000 +write -rename -delete\n" <<
            "\n";
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
    string query = "select path from config where id = 1;";
    string res[2] = {"",""};
    sqlite3_exec(db, query.c_str(), callback, res, &err);
    
    if (res[0] == "" || err != NULL)
    {
        cout << "Wrong path to database" << endl;
        abort();
    }
    mountdir = res[0];
    
    query = "select path from config where id = 2;";
    sqlite3_exec(db, query.c_str(), callback, res, &err);
    
    rootdir = res[0];
    
    setuid(1);
    
    return 0;
}

int checkPath(string path)
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
    for (int i = 1; i < uid.length(); i++)
    {
        if (!(isdigit(uid[i])))
        {
            cout << "Wrong id" << endl;
            return 0;
        }
    }
    
    return 1;
}

int checkPermissions(string prms)
{
    for (int i = 0; i < prms.length(); i ++)
    {
        if (!isdigit(prms[i]))
        {
            cout << "Wrong permission mask" << endl;
            return 0;
        }
    }
    
    return 1;
}

int show(int n, char **path){
    string query;
    char *err = 0;
    
    if (n < 1 || string(path[0]) == "all")
        query = "select id, path, uid, gid, owner_prms, group_prms, other_prms from file_list;";
    else
    {
        string p(path[0]);
        
        if (!checkPath(p))
            return 0;
        
        query = "select id, path, uid, gid, owner_prms, group_prms, other_prms from file_list where path =\"" + p + "\";";
    }
    
    (sqlite3_exec(db, query.c_str(), print_callback, NULL, &err) == SQLITE_OK);
    
    sqlite3_close(db);
}

int set(int n, char **argv)
{
    if (n < 3)
    {
        cout << "Not enough arguments" << endl;
        return 0;
    }
    
    string mode(argv[0]), path(argv[1]), uid, prms(argv[2]);
    
    if (!checkPath(path) || !checkPermissions(prms))
        return 0;
    string query;
    //query = "insert or replace into prm_list values(\"" + path +"\", " + uid + ", " + prms + ");";
    if (mode == "-ow")
        query = "update file_list set owner_prms = " + prms + " where path = " + path + ";";
    else if(mode == "-gow")
        query = "update file_list set group_prms = " + prms + " where path = " + path + ";";
    else if(mode == "-oth")
        query = "update file_list set other_prms = " + prms + " where path = " + path + ";";
    else 
    {
        if (n < 4)
        {
            cout << "Not enough arguments" << endl;
            return 0;
        }
        
        uid = string(argv[3]);
        if(mode == "-g" && check_uid(uid))
            query = "insert or replace into group_prm_list (file_id, id, prms) values((select id from file_list where path = \"" + path + "\"), " + uid + ", " + prms + ");";
        else if (mode == "-u" && check_uid(uid))
            query = "insert or replace into user_prm_list (file_id, id, prms) values((select id from file_list where path = \"" + path + "\"), " + uid + ", " + prms + ");";
        else 
            return 0;
    }
    
    char *err = 0;
    if (!(sqlite3_exec(db, query.c_str(), NULL, NULL, &err) == SQLITE_OK))
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
        return prm | bit;
    else
        if ((prm & bit) != 0)
            return prm - bit;
        else
            return prm;
}

int get_field(const char *file, const char *field)
{
    int result = 0;
    char *err = NULL, *res = NULL, query[QUERY_MAX];

    res = (char *)malloc(sizeof(char) * PRM_MAX);
    
    sprintf(query, "SELECT %s FROM file_list WHERE path = \"%s\"", field, file);
    sqlite3_exec(db, query, prm_callback, res, &err);
    result = atoi(res);
    
    free(res);
    
    return result;
}

int change(int n, char **argv)
{
    string path, id, table, field, tfield;
    
    if (n < 3)
        return 0;
    
    path = string(argv[1]);
    id = string(argv[0]);
    
    table = "file_list";
    if (id == "-ow")
        field = "owner_prms";
    else if (id == "-gown")
        field = "group_prms";
    else if(id == "-oth")
        field = "other_prms";
    else 
        if (n >= 5)
        {
            if (id == "-g")
            {
                table = "group_prm_list";
                field = "group_prms";
                tfield = "gid";
            }
            else
            {
                table = "user_prm_list";
                field = "owner_prms";
                tfield = "uid";
            }
            id = argv[2];
        }
        else 
            return 0;
    
    if (table == "file_list"){
        for (int i = 2; i < n; i ++){
            string prm = argv[i];
            prm.erase(0, 1);
            if (operations.find(prm) == operations.end())
                continue;
            
            int op = argv[i][0] == '+';
            int op_bit = operations[prm];
            int old_permissions = get_field(path.c_str(), field.c_str());
            
            char *query = new char[QUERY_MAX], *err = NULL;
            sprintf(query, "update file_list set %s = %d where path = \"%s\"", field.c_str(), get_new_prm(old_permissions, op_bit, op), path.c_str());
            sqlite3_exec(db, query, NULL, NULL, &err);
            delete [] query;
        }
    }else{
        for (int i = 3; i < n; i ++){
            string prm = argv[i];
            prm.erase(0, 1);
            if (operations.find(prm) == operations.end())
                continue;
            int op = argv[i][0] == '+';
            int op_bit = operations[prm];
            int old_permissions;
            
            char *query = new char[QUERY_MAX], *res = new char[PRM_MAX], *err = NULL;
            sprintf(query, "select exists(select * from %s where file_id = (select id from file_list where path = \"%s\") and id = %s);", table.c_str(), path.c_str(), id.c_str());
            sqlite3_exec(db, query, prm_callback, res, &err);
            if (res[0] == '1'){
                sprintf(query, "select prms from %s where file_id = (select id from file_list where path = \"%s\") and id = %s;", table.c_str(), path.c_str(), id.c_str());
                sqlite3_exec(db, query, prm_callback, res, &err);
                old_permissions = atoi(res);
            }
            else
            {
                if (get_field(path.c_str(), tfield.c_str()) == atoi(id.c_str()))
                    old_permissions = get_field(path.c_str(), field.c_str());
                else
                    old_permissions = get_field(path.c_str(), "other_prms");
            }
            
            sprintf(query, "insert or replace into %s (file_id, id, prms) values((select id from file_list where path = \"%s\"), %s, %d)", table.c_str(), path.c_str(), id.c_str(), get_new_prm(old_permissions, op_bit, op));
            sqlite3_exec(db, query, NULL, NULL, &err);
            
            delete [] query;
            delete [] res;
        }
    }
    
    return 0;
}

/*
 * 
 */
int main(int argc, char **argv) 
{
    if (argc < 1)
        return 0;
    
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
            configure(argv[2]);
            return show(argc - 2, argv + 2);
        case 3:
            configure(argv[2]);
            return set(argc - 2, argv + 2);
        case 4:
            configure(argv[2]);
            return change(argc - 2, argv + 2);
        default:
            return 0;
    }
    
    return 0;
}


