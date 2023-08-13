#ifndef SYMBTAB_H
#define SYMBTAB_H

#include <map>
#include <iostream>
#include "type.hh"
using namespace std;

class ST {
    map<string, class Symbols*> sym;
    public:
        ST() {}
        void addSymbol(class Symbols* s);
        map<string, class Symbols*> get_elems();
        void printst();
};

class Symbols {
    string name;
    string type;
    string scope;
    int size;
    int offset;
    string returntype;
    class types_info* types;
    public:
        Symbols(string name, string type, string scope, int size, int offset, string returntype, class types_info* types);
        void change_offset(int offset);
        int get_size();
        int get_offset();
        string get_name();
        string get_type();
        string get_returntype();
        class types_info* get_type_info();
        void print();
};

#endif  // SYMBTAB_H