#include "symbtab.hh"

void ST::addSymbol(class Symbols* s) {
    this -> sym[s -> get_name()] = s;
}

map<string, class Symbols*> ST::get_elems() {
    return this -> sym;
}

void ST::printst() {
    int len = (this -> sym).size();
    int ind = 1;
    cout << "[\n";
    for(pair<string, class Symbols*> s: this -> sym) {
        (s.second) -> print();
        if(ind != len) cout << ",\n";
        ind += 1;
    }
    cout << "]\n";
}

Symbols::Symbols(string name, string type, string scope, int size, int offset, string returntype, class types_info* types) {
    this -> name = name;
    this -> type = type;
    this -> scope = scope;
    this -> size = size;
    this -> offset = offset;
    this -> returntype = returntype;
    this -> types = types;
}

void Symbols::change_offset(int offset) {
    this -> offset = offset;
}

int Symbols::get_size() {
    return this -> size;
}

int Symbols::get_offset() {
    return this -> offset;
}

string Symbols::get_name() {
    return this -> name;
}

string Symbols::get_type() {
    return this -> type;
}

string Symbols::get_returntype() {
    return this -> returntype;
}

class types_info* Symbols::get_type_info() {
    return this -> types;
}

void Symbols::print() {
    cout << "[\"" + this -> name << "\", \"" + this -> type + "\", \"" + this -> scope + "\", " << this -> size << ", ";
    if(this -> type == "struct") cout << "\"-\", \"" + returntype + "\"\n]\n";
    else cout << this -> offset << ", \"" + returntype + "\"\n]\n";
}