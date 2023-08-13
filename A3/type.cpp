#include "type.hh"

types_info::types_info(string basetype) {
    this -> basetype = basetype;
    this -> num_pointers = 0;
    this -> arr_pointers = 0;
    this -> tot_pointers = this -> num_pointers + this -> arr_pointers;
    queue<int> constants;
    this -> arr_const = constants;
    this -> arr_size = 0;
    this -> lval = false;
    this -> isZero = false;
}

types_info::types_info(string basetype, int num_pointers, int arr_pointers, queue<int> arr_const) {
    this -> basetype = basetype;
    this -> num_pointers = num_pointers;
    this -> arr_pointers = arr_pointers;
    this -> tot_pointers = this -> num_pointers + this -> arr_pointers;
    this -> arr_const = arr_const;
    this -> arr_size = (this -> arr_const).size();
    this -> lval = false;
    this -> isZero = false;
}

string types_info::full_type(bool decl) {
    string type = this -> basetype;
    for(int ptr = 0; ptr < this -> num_pointers; ptr ++) {
        type += "*";
    }
    queue<int> curr_arr_const = this -> arr_const;
    if(this -> arr_pointers != 0) {
        if(curr_arr_const.size() > 0 && this -> basetype != "void") {
            type += "(*)";
        }
        else {
            type += "*";
        }
    }
    if(decl) {
        while(curr_arr_const.size() > 0) {
            int inner_const = curr_arr_const.front();
            curr_arr_const.pop();
            type += "[" + to_string(inner_const) + "]";
        }
    }
    else {
        if(curr_arr_const.size() > 0 && this -> arr_pointers == 0) {
            curr_arr_const.pop();
            if(curr_arr_const.size() > 0) {
                type += "(*)";
            }
            else {
                type += "*";
            }
        }
        while(curr_arr_const.size() > 0) {
            int inner_const = curr_arr_const.front();
            curr_arr_const.pop();
            type += "[" + to_string(inner_const) + "]";
        }
    }

    return type;
}

bool types_info::isBaseType() {
    return (this -> tot_pointers == 0) && (this -> arr_size == 0);
}

bool types_info::sameBaseType(class types_info* second_type) {
    bool compatible = (this -> isBaseType()) && (second_type -> isBaseType());
    compatible &= (this -> basetype == second_type -> basetype);
    compatible |= (this -> isInt()) && (second_type -> isFloat());
    compatible |= (this -> isFloat()) && (second_type -> isInt());
    return compatible;
}

bool types_info::isCompatiblePointer(class types_info* second_type) {
    bool compatible = !(this -> isBaseType()) && !(second_type -> isBaseType());
    compatible &= (this -> basetype == second_type -> basetype);
    if(this -> arr_size == second_type -> arr_size) {
        compatible &= (this -> full_type() == second_type -> full_type());
    }
    else if(this -> arr_size == second_type -> arr_size + 1) {
        compatible &= (this -> full_type() == second_type -> full_type(true));
    }
    else if(this -> arr_size == second_type -> arr_size - 1) {
        compatible &= (this -> full_type(true) == second_type -> full_type());
    }
    else compatible = false;
    return compatible;
}

bool types_info::isBasePointer() {
    return (this -> num_pointers != 0) && (this -> arr_pointers == 0) && (this -> arr_size == 0);
}

bool types_info::isVoidPointer() {
    return (this -> basetype == "void") && (this -> num_pointers == 1) && (this -> arr_pointers == 0) && (this -> arr_size == 0);
}

bool types_info::isCompletePointer() {
    return !(this -> isVoidPointer()) && !(this -> isBaseType());
}

bool types_info::isStructPointer() {
    return ((this -> basetype).substr(0,6) == "struct") && (this -> tot_pointers + this -> arr_size == 1) && (this -> arr_pointers == 0);
}

bool types_info::isVoid() {
    return (this -> basetype == "void") && (this -> isBaseType());
}

bool types_info::isInt() {
    return (this -> basetype == "int") && (this -> isBaseType());
}

bool types_info::isFloat() {
    return (this -> basetype == "float") && (this -> isBaseType());
}

bool types_info::isStruct() {
    return ((this -> basetype).substr(0,6) == "struct") && (this -> isBaseType());
}