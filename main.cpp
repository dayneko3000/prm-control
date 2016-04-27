/*
  Copyright (C) 2016 Roman Y. Dayneko, <dayneko3000@gmail.com>

  This program can be distributed under the terms of the GNU GPLv3.
  See the file COPYING.
*/

#include "ftpc.h"

using namespace std;

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
        case 1:
            return configure(argv[2]);
        case 2:
            config();
            return show(argc - 2, argv + 2);
        case 3:
            config();
            return set(argc - 2, argv + 2);
        case 4:
            config();
            return change(argc - 2, argv + 2);
        default:
            return 0;
    }
}