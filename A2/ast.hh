#ifndef AST_H
#define AST_H

#include <vector>
#include <string>
#include <iostream>
#include "type.hh"
using namespace std;

class abstract_astnode
    {
        public:
            virtual void print() = 0;
    };

class statement_astnode : public abstract_astnode
    {
        public:
            virtual void print() = 0;
            
    };

class exp_astnode : public abstract_astnode
    {
        class types_info* types;
        public:
            virtual void print() = 0;
            class types_info* get_type_info();
            void set_type_info(class types_info* types);
    };  

class ref_astnode : public exp_astnode
    {
        public:
            virtual void print() = 0;
            
    };

class nostatement : public statement_astnode
    {
        public:
            nostatement() {};
            void print() {}
    };

class empty_astnode : public statement_astnode
    {
        public:
            empty_astnode() {};
            void print();
            
    };

class seq_astnode : public statement_astnode
    {
        vector<class statement_astnode*> attr1;
        public:
            seq_astnode(vector<class statement_astnode*> attr1);
            vector<class statement_astnode*> get_attr1();
            void print();
            
    };

class assignS_astnode : public statement_astnode
    {
        class exp_astnode* attr1;
        class exp_astnode* attr2;
        public:
            assignS_astnode(class assignE_exp* expr);
            void print();
            
    };

class return_expnode : public statement_astnode
    {
        class exp_astnode* attr1;
        public:
            return_expnode(class exp_astnode* attr1);
            void print();
            
    };

class proccall_astnode : public statement_astnode
    {
        vector<class exp_astnode*> attr1;
        public:
            proccall_astnode(vector<class exp_astnode*> attr1);
            void print();
            
    };

class if_astnode : public statement_astnode
    {
        class exp_astnode* attr1;
        class statement_astnode* attr2;
        class statement_astnode* attr3;
        public:
            if_astnode(class exp_astnode* attr1, class statement_astnode* attr2, class statement_astnode* attr3);
            void print();
            
    };

class while_astnode : public statement_astnode
    {
        class exp_astnode* attr1;
        class statement_astnode* attr2;
        public:
            while_astnode(class exp_astnode* attr1, class statement_astnode* attr2);
            void print();
            
    };

class for_astnode : public statement_astnode
    {
        class exp_astnode* attr1;
        class exp_astnode* attr2;
        class exp_astnode* attr3;
        class statement_astnode* attr4;
        public:
            for_astnode(class exp_astnode* attr1, class exp_astnode* attr2, class exp_astnode* attr3, class statement_astnode* attr4);
            void print();    
    };

class op_binary_astnode : public exp_astnode
    {
        string attr1;
        class exp_astnode* attr2;
        class exp_astnode* attr3;
        public:
            op_binary_astnode(string attr1, class exp_astnode* attr2, class exp_astnode* attr3);
            void print();
    };

class op_unary_astnode : public exp_astnode
    {
        string attr1;
        class exp_astnode* attr2;
        public:
            op_unary_astnode(string attr1, class exp_astnode* attr2);
            void print();
    };

class assignE_exp : public exp_astnode
    {
        class exp_astnode* attr1;
        class exp_astnode* attr2;
        public:
            assignE_exp(class exp_astnode* attr1, class exp_astnode* attr2);
            class exp_astnode* get_attr1();
            class exp_astnode* get_attr2();
            void print();
    };

class funcall_astnode : public exp_astnode
    {
        vector<class exp_astnode*> attr1;
        public:
            funcall_astnode(vector<class exp_astnode*> attr1);
            void print();
    };

class floatconst_astnode : public exp_astnode
    {
        float attr1;
        public:
            floatconst_astnode(float attr1);
            void print();
    };

class intconst_astnode : public exp_astnode
    {
        int attr1;
        public:
            intconst_astnode(int attr1);
            void print();
    };

class string_astnode : public exp_astnode
    {
        string attr1;
        public:
            string_astnode(string attr1);
            void print();
    };

class identifier_astnode : public ref_astnode
    {
        string attr1;
        public:
            identifier_astnode(string attr1);
            void print();
    };

class member_astnode : public ref_astnode
    {
        class exp_astnode* attr1;
        class identifier_astnode* attr2;
        public:
            member_astnode(class exp_astnode* attr1, class identifier_astnode* attr2);
            void print();
    };

class arrow_astnode : public ref_astnode
    {
        class exp_astnode* attr1;
        class identifier_astnode* attr2;
        public:
            arrow_astnode(class exp_astnode* attr1, class identifier_astnode* attr2);
            void print();
    };

class arrayref_astnode : public ref_astnode
    {
        class exp_astnode* attr1;
        class exp_astnode* attr2;
        public:
            arrayref_astnode(class exp_astnode* attr1, class exp_astnode* attr2);
            void print();
    };

#endif  // AST_H