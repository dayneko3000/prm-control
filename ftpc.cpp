/*
  Copyright (C) 2016 Roman Y. Dayneko, <dayneko3000@gmail.com>

  This program can be distributed under the terms of the GNU GPLv3.
  See the file COPYING.
*/

#include "ftpc.h"

using namespace std;

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
    
    char *err = nullptr;
    string query = "select id, prms from user_prm_list where file_id = " + string(rows[0]) + ";";
    sqlite3_exec(db, query.c_str(), print_prm_callback, nullptr, &err);
    cout << "groups:" << endl;
    query = "select id, prms from group_prm_list where file_id = " + string(rows[0]) + ";";
    sqlite3_exec(db, query.c_str(), print_prm_callback, nullptr, &err);
    
    cout << endl;
    
    return 0;
}


static int callback(void *notused, int coln, char **rows, char **colnm)
{
    string *t = static_cast<string *>(notused);
    for (int i = 0; i < coln; i ++)
        t[i] = string(rows[i]);
    return 0;
}

int get_operation_number(const string operation)
{
    if (operations.find(operation) == operations.end())
        return -1;
    else
        return operations[operation];
}

int help()
{
    cout << "[command <command args>]\n" << 
            "commands:\n"
            "--h --help:                                                   printing this message\n\n" <<
            "--cfg --configure [path to FTDB]                              configure FTPC for working with current FTFS\n\n" <<
            "--show [\"\"|\"all\"|file/directory path]:                        printing information db contains\n\n" <<
            "--set [option, <option args>]:                                set permissions of file by permission mask\n\toptions:\n" << 
            "\t-ow [file/directory path, permission mask]:           set permissions for file owner\n" <<
            "\t-gow [file/directory path, permission mask]:          set permissions for file group\n" <<
            "\t-oth [file/directory path, permission mask]:          set permissions for other\n" <<
            "\t-g [file/directory path, gid, permission mask]:       set individual permissions for group\n" <<
            "\t-u [file/directory path, uid, permission mask]:       set individual permissions for user\n" <<
            "example: --set /home/user/Desktop/FTDB.db -u /abc.txt 1000 67108863\n\n" <<
            "--c --change [option, <option args>]:                         add/remove permission by type\n\toptions:\n" <<
            "\t-ow [file/directory path, <+-permission type>]:       change permissions of owner\n" <<
            "\t-gow [file/directory path, <+-permission type>]:      change permissions of file group\n" <<
            "\t-oth [file/directory path, <+-permission type>]:      change permissions of other\n" <<
            "\t-g [file/directory path, gid, <+-permission type>]:   change individual permissions of group\n" <<
            "\t-u [file/directory path, uid, <+-permission type>]:   change individual permissions of user\n" <<
            "example: --c /home/user/Desktop/FTDB.db -u /abc.txt 1000 +write -rename -delete\n" <<
            "\n";
    return 0;
}

int configure(string db_path)
{
    ofstream out("config.txt");
    
    if (!(sqlite3_open(realpath(db_path.c_str(), nullptr), &db) == SQLITE_OK))
    {
        cout << "Wrong path to database" << endl;
        return 0;
    }
    
    char *err = nullptr;
    string query = "select path from config where id = 1;";
    string res[2] = {"",""};
    sqlite3_exec(db, query.c_str(), callback, res, &err);
    
    if (res[0] == "" || err != nullptr)
    {
        cout << "Wrong path to database" << endl;
        sqlite3_close(db);
        out.close();
        return 0;
    }
    
    out << realpath(db_path.c_str(), nullptr);
    sqlite3_close(db);
    out.close();
}

int config()
{
    string db_path = "";
    ifstream in("config.txt");
    
    in >> db_path;
    in.close();
    
    if (db_path == "")
    {
       cout << "You must configure firstly" << endl;
       return 0;
    }
    char *db_realpath = realpath(db_path.c_str(), nullptr);
    if (!(sqlite3_open(db_realpath, &db) == SQLITE_OK))
    {
        cout << "Something went wrong. Try to reconfigure." << endl;
        return 0;
    }
    free(db_realpath);
//    char *err = nullptr;
//    string query = "select path from config where id = 1;";
//    string res[2] = {"",""};
//    sqlite3_exec(db, query.c_str(), callback, res, &err);
//    
//    if (res[0] == "" || err != nullptr)
//    {
//        cout << "Something went wrong. Try to reconfigure." << endl;
//        return 0;
//    }
//    mountdir = res[0];
//    
//    query = "select path from config where id = 2;";
//    sqlite3_exec(db, query.c_str(), callback, res, &err);
//    
//    rootdir = res[0];
    
    setuid(1);
    
    return 0;
}

int get_new_permissions(int prm, int bit, int operation)
{
    if (operation)
        return prm | bit;
    else
        if ((prm & bit) != 0)
            return prm - bit;
        else
            return prm;
}

int get_file_list_field(const string file, const string field)
{
    int result = 0;
    char *err = nullptr, query[QUERY_MAX];
    string *res = new string[1];
    
    sprintf(query, "SELECT %s FROM file_list WHERE path = \"%s\";", field.c_str(), file.c_str());
    sqlite3_exec(db, query, callback, res, &err);
    result = atoi(res[0].c_str());
    
    delete [] res;
    
    return result;
}

int check_chmod_permission(string path){
    int uid, gid;
    char *err = nullptr;
    string *res = new string [1];
    string query = "SELECT EXISTS(SELECT * FROM user_prm_list WHERE (file_id = (SELECT file_id FROM file_list where path = \"" + path +"\")));";
    sqlite3_exec(db, query.c_str(), callback, res, &err);
    if (res[0] == "1"){
        query = "SELECT prms FROM user_prm_list WHERE (file_id = (SELECT file_id FROM file_list where path = \"" + path +"\"));";
        sqlite3_exec(db, query.c_str(), callback, res, &err);
        int prm = atoi(res[0].c_str());
        delete [] res;
        if ((prm & operations["chmod"]))
            return true;
        else
            return false;
    }
    
    query = "SELECT EXISTS(SELECT * FROM group_prm_list WHERE (file_id = (SELECT file_id FROM file_list where path = \"" + path +"\")));";
    sqlite3_exec(db, query.c_str(), callback, res, &err);
    if (res[0] == "1"){
        query = "SELECT prms FROM group_prm_list WHERE (file_id = (SELECT file_id FROM file_list where path = \"" + path +"\"));";
        sqlite3_exec(db, query.c_str(), callback, res, &err);
        int prm = atoi(res[0].c_str());
        delete [] res;
        if ((prm & operations["chmod"]))
            return true;
        else
            return false;
    }
    
    delete [] res;
    
    uid = get_file_list_field(path, "uid");
    if (uid == (int)getuid()){
        int prm = get_file_list_field(path, "owner_prms");
        if ((prm & operations["chmod"]))
            return true;
        else
            return false;
    }
    
    gid = get_file_list_field(path, "gid");
    if (gid == (int)getgid()){
        int prm = get_file_list_field(path, "group_prms");
        if ((prm & operations["chmod"]))
            return true;
        else
            return false;
    }
    
    int prm = get_file_list_field(path, "other_prms");
        if ((prm & operations["chmod"]))
            return true;
        else
            return false;
}

int check_path(string path)
{
    int result;
    char *err = nullptr;
    string *res = new string[1];
    string query = "SELECT EXISTS(SELECT * FROM file_list where path = \"" + path +"\");";
    
    sqlite3_exec(db, query.c_str(), callback, res, &err);
    result = res[0] == "1";
    
    delete [] res;
    
    return result;
}

int check_uid_string(string uid)
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

int check_permission_string(string prms)
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

int get_mask(int prm)
 {
     int result = 0;
     if ((prm & (READ | GETATTR | READDIR | FGETATTR)) != 0)
         result += 4;
     if ((prm & (WRITE | UNLINK | RENAME | LINK | MKNOD | MKDIR | RMDIR | CREATE | SYMLINK | UTIME)) != 0)
         result += 2;
     if ((prm & (EXECUTE | OPENDIR)) != 0)
         result += 1;
     return result;
 }

int get_new_mode(string file){
    char query[QUERY_MAX], *err = NULL;
    int result = 0, g_prms = 0, u_prms = 0, o_prms = 0;
    
    result = get_file_list_field(file, "mode") & MODE_MASK;
    u_prms = get_file_list_field(file, "owner_prms");
    g_prms = get_file_list_field(file, "group_prms");
    o_prms = get_file_list_field(file, "other_prms");
    
    result += (get_mask(u_prms) << 6) + (get_mask(g_prms) << 3) + get_mask(o_prms);
    
    return result;
}

int show(int n, char **path){
    string query;
    char *err = 0;
    
    if (n < 1 || string(path[0]) == "all")
        query = "select id, path, uid, gid, owner_prms, group_prms, other_prms from file_list;";
    else
    {
        string p(path[0]);
        
        if (!check_path(p))
            return 0;
        
        query = "select id, path, uid, gid, owner_prms, group_prms, other_prms from file_list where path =\"" + p + "\";";
    }
    
    (sqlite3_exec(db, query.c_str(), print_callback, nullptr, &err) == SQLITE_OK);
    
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
    
    if (!check_chmod_permission(path))
        return 0;
    
    if (!check_path(path) || !check_permission_string(prms))
        return 0;
    
    string query;
    
    if (mode == "-ow")
        query = "update file_list set owner_prms = " + prms + " where path = \"" + path + "\";";
    else if(mode == "-gow")
        query = "update file_list set group_prms = " + prms + " where path = \"" + path + "\";";
    else if(mode == "-oth")
        query = "update file_list set other_prms = " + prms + " where path = \"" + path + "\";";
    else 
    {
        if (n < 4)
        {
            cout << "Not enough arguments" << endl;
            return 0;
        }
        
        uid = string(argv[3]);
        if(mode == "-g" && check_uid_string(uid))
            query = "insert or replace into group_prm_list (file_id, id, prms) values((select id from file_list where path = \"" + path + "\"), " + uid + ", " + prms + ");";
        else if (mode == "-u" && check_uid_string(uid))
            query = "insert or replace into user_prm_list (file_id, id, prms) values((select id from file_list where path = \"" + path + "\"), " + uid + ", " + prms + ");";
        else 
            return 0;
    }
    
    char *err = 0;
    if (!(sqlite3_exec(db, query.c_str(), nullptr, nullptr, &err) == SQLITE_OK))
    {
        cout << "Something went wrong" << endl;
        return 0;
    }
    
    char *update_query = new char [QUERY_MAX];
    sprintf(update_query, "update file_list set mode = %d where path = \"%s\"", get_new_mode(path), path.c_str());
    
    sqlite3_exec(db, update_query, nullptr, nullptr, &err);
    
    delete [] update_query;
    sqlite3_close(db);
    
    return 0;
}



int change(int n, char **argv)
{
    string path, id, table, field, ownerField;
    
    if (n < 3)
        return 0;
    
    path = string(argv[1]);
    
    if (!check_path(path))
        return 0;
    
    if (!check_chmod_permission(path))
        return 0;
    
    id = string(argv[0]);
    
    table = "file_list";
    if (id == "-ow")
        field = "owner_prms";
    else if (id == "-gow")
        field = "group_prms";
    else if(id == "-oth")
        field = "other_prms";
    else 
        if (n >= 4)
        {
            if (id == "-g")
            {
                table = "group_prm_list";
                field = "group_prms";
                ownerField = "gid";
            }
            else
            {
                if (id != "-u")
                    return 0;
                table = "user_prm_list";
                field = "owner_prms";
                ownerField = "uid";
            }
            id = argv[2];
        }
        else 
            return 0;
    
    if (table == "file_list"){
        int new_permissions = get_file_list_field(path.c_str(), field.c_str());
        
        for (int i = 2; i < n; i ++){
            string prm = argv[i];
            prm.erase(0, 1);
            if (get_operation_number(prm) == -1)
                continue;
            
            int operation = argv[i][0] == '+';
            int operation_bit = operations[prm];
            new_permissions = get_new_permissions(new_permissions, operation_bit, operation);
        }
            
        char *query = new char[QUERY_MAX], *err = nullptr;
        sprintf(query, "update file_list set %s = %d where path = \"%s\"", field.c_str(), new_permissions, path.c_str());
        sqlite3_exec(db, query, nullptr, nullptr, &err);

        sprintf(query, "update file_list set mode = %d where path = \"%s\"", get_new_mode(path), path.c_str());
        sqlite3_exec(db, query, nullptr, nullptr, &err);
        delete [] query;
        
    }else{
        int new_permissions;
        char *query = new char[QUERY_MAX], *err = nullptr;
        string *res = new string[1];
        sprintf(query, "select exists(select * from %s where file_id = (select id from file_list where path = \"%s\") and id = %s);", 
                table.c_str(), path.c_str(), id.c_str());
        sqlite3_exec(db, query, callback, res, &err);
        if (res[0] == "1"){
            sprintf(query, "select prms from %s where file_id = (select id from file_list where path = \"%s\") and id = %s;", 
                    table.c_str(), path.c_str(), id.c_str());
            sqlite3_exec(db, query, callback, res, &err);
            new_permissions = atoi(res[0].c_str());
        }
        else
        {
            if (get_file_list_field(path.c_str(), ownerField.c_str()) == atoi(id.c_str()))
                new_permissions = get_file_list_field(path.c_str(), field.c_str());
            else
                new_permissions = get_file_list_field(path.c_str(), "other_prms");
        }
        
        for (int i = 3; i < n; i ++){
            string prm = argv[i];
            prm.erase(0, 1);
            if (get_operation_number(prm) == -1)
                continue;
            
            int operation = argv[i][0] == '+';
            int operation_bit = operations[prm];
            new_permissions = get_new_permissions(new_permissions, operation_bit, operation);
        }
            
        sprintf(query, "insert or replace into %s (file_id, id, prms) values((select id from file_list where path = \"%s\"), %s, %d)", table.c_str(), path.c_str(), id.c_str(), new_permissions);
        sqlite3_exec(db, query, nullptr, nullptr, &err);

        delete [] query;
        delete [] res;
    }
    sqlite3_close(db);
    return 0;
}