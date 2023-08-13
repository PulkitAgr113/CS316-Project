#ifndef TYPE_H
#define TYPE_H

#include <queue>
#include <string>
using namespace std;

class types_info {
    public:
        string basetype;
        int num_pointers;
        int arr_pointers;
        int tot_pointers;
        queue<int> arr_const;
        int arr_size;
        bool lval;
        bool isZero;
        types_info(string basetype);
        types_info(string basetype, int num_pointers, int arr_pointers, queue<int> arr_const);
        string full_type(bool decl = false);
        bool isBaseType();
        bool sameBaseType(class types_info* second_type);
        bool isCompatiblePointer(class types_info* second_type);
        bool isBasePointer();
        bool isVoidPointer();
        bool isCompletePointer();
        bool isStructPointer();
        bool isVoid();
        bool isInt();
        bool isFloat();
        bool isStruct();
};

#endif  // TYPE_H