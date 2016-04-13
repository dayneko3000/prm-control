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
    for (int i = 1; i < uid.length(); i++)
    {
        if (!(isdigit(uid[i])))
        {
            cout << "Wrong uid" << endl;
            return 0;
        }
    }
    
    return 1;
}

int check_prms(string prms)
{
//    if (prms.length() != 30)
//    {
//        cout << "Wrong permission mask" << endl;
//        return 0;
//    }
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
    string query;
    char *err = 0;
    
    if (n < 2 || string(path[1]) == "all")
        query = "select id, path, uid, gid, owner_prms, group_prms, other_prms from file_list;";
    else
    {
        string p(path[1]);
        
        if (!check_path(p))
            return 0;
        
        query = "select id, path, uid, gid, owner_prms, group_prms, other_prms from file_list where path =\"" + p + "\";";
    }
    
    if (!(sqlite3_exec(db, query.c_str(), print_callback, NULL, &err) == SQLITE_OK))
    {
        cout << "Wrong data base path" << endl;
        return 0;
    }
    
    sqlite3_close(db);
}



int set(int n, char **argv)
{
    if (n < 4)
    {
        cout << "Not enough arguments" << endl;
        return 0;
    }
    
    string mode(argv[1]), path(argv[2]), uid(argv[3]), prms(argv[4]);
    
    if (!check_path(path) || !check_prms(prms))
        return 0;
//    char prm[30];
//    sprintf(prm, "%d",  bitset<30>(prms).to_ulong());
    string query;
    //query = "insert or replace into prm_list values(\"" + path +"\", " + uid + ", " + prms + ");";
    if (mode == "-ow")
        query = "update file_list set owner_prms = " + prms + " where path = " + path + ";";
    else if(mode == "-gow")
        query = "update file_list set group_prms = " + prms + " where path = " + path + ";";
    else if(mode == "-oth")
        query = "update file_list set other_prms = " + prms + " where path = " + path + ";";
    else if(mode == "-g" && check_uid(uid))
        query = "insert or replace into group_prm_list (file_id, id, prms) values((select id from file_list where path = \"" + path + "\"), " + uid + ", " + prms + ");";
    else if (check_uid(uid))
        query = "insert or replace into user_prm_list (file_id, id, prms) values((select id from file_list where path = \"" + path + "\"), " + uid + ", " + prms + ");";
    else 
        return 0;
    
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
    {
        return prm | bit;
    }
    else
    {
        if ((prm & bit) != 0)
            return prm - bit;
        else
            return prm;
    }
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
    
    path = string(argv[2]);
    id = string(argv[1]);
    
    table = "file_list";
    if (id == "-owner")
        field = "owner_prms";
    else if (id == "-group")
        field = "group_prms";
    else if(id == "-other")
        field = "other_prms";
    else if (n >= 5){
        if (id == "-g"){
            table = "group_prm_list";
            field = "group_prms";
            tfield = "gid";
        }else{
            table = "user_prm_list";
            field = "owner_prms";
            tfield = "uid";
        }
        id = argv[3];
    }else return 0;
    
    if (table == "file_list"){
        for (int i = 3; i < n; i ++){
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
        for (int i = 4; i < n; i ++){
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
            }else{
                if (get_field(path.c_str(), tfield.c_str()) == atoi(id.c_str())){
                    old_permissions = get_field(path.c_str(), field.c_str());
                }else{
                    old_permissions = get_field(path.c_str(), "other_prms");
                }
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
    
    argv++;
    argc--;
    
    if (comands.find(string(argv[0])) == comands.end())
    {
        cout << "Undefined operation" << endl;
        return 0;
    }
    
    switch (comands[string(argv[0])])
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


