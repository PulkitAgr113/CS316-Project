#include "ast.hh"

class types_info* exp_astnode::get_type_info() {
    return this -> types;
}

void exp_astnode::set_type_info(class types_info* types) {
    this -> types = types;
}

void empty_astnode::print() {
    cout << "\"empty\"\n";
}

seq_astnode::seq_astnode(vector<class statement_astnode*> attr1) {
    this -> attr1 = attr1;
}

vector<class statement_astnode*> seq_astnode::get_attr1() {
    return this -> attr1;
}

void seq_astnode::print() {
    int sz = (this -> attr1).size();
    int ind = 1;
    cout << "{\"seq\": [\n";
    for(class statement_astnode* s: this -> attr1) {
        s -> print();
        if(ind != sz) cout << ",\n";
        ind ++;
    }
    cout << "]\n}\n";
}

assignS_astnode::assignS_astnode(class assignE_exp* expr) {
    this -> attr1 = expr -> get_attr1();
    this -> attr2 = expr -> get_attr2();
}

void assignS_astnode::print() {
    cout << "{ \"assignS\": {\n";
    cout <<"\"left\":\n";
    (this -> attr1) -> print();
    cout << ",\n";
    cout << "\"right\":\n";
    (this -> attr2) -> print();
    cout << "}\n}\n";
}

return_expnode::return_expnode(class exp_astnode* attr1) {
    this -> attr1 = attr1;
}

void return_expnode::print() {
    cout << "{ \"return\":\n";
    (this -> attr1) -> print();
    cout << "}\n";
}

proccall_astnode::proccall_astnode(vector<class exp_astnode*> attr1) {
    this -> attr1 = attr1;
}

void proccall_astnode::print() {
    cout << "{ \"proccall\": {\n";
    cout << "\"fname\":\n";
    (this -> attr1)[0] -> print();
    cout << ",\n\"params\": [\n";
    int sz = (this -> attr1).size();
    for(int ind = 1; ind < sz; ind ++) {
        class exp_astnode* s = (this -> attr1)[ind];
        s -> print();
        if(ind != sz-1) cout << ",\n";
    }
    cout << "]\n}\n}\n";
}

if_astnode::if_astnode(class exp_astnode* attr1, class statement_astnode* attr2, class statement_astnode* attr3) {
    this -> attr1 = attr1;
    this -> attr2 = attr2;
    this -> attr3 = attr3;
}

void if_astnode::print() {
    cout << "{ \"if\": {\n";
    cout << "\"cond\":\n";
    (this -> attr1) -> print();
    cout << ",\n\"then\": \n";
    (this -> attr2) -> print();
    cout << ",\n\"else\": \n";
    (this -> attr3) -> print();
    cout << "}\n}\n";
}

while_astnode::while_astnode(class exp_astnode* attr1, class statement_astnode* attr2) {
    this -> attr1 = attr1;
    this -> attr2 = attr2;
}

void while_astnode::print() {
    cout << "{ \"while\": {\n";
    cout << "\"cond\":\n";
    (this -> attr1) -> print();
    cout << ",\n\"stmt\": \n";
    (this -> attr2) -> print();
    cout << "}\n}\n";
}

for_astnode::for_astnode(class exp_astnode* attr1, class exp_astnode* attr2, class exp_astnode* attr3, class statement_astnode* attr4) {
    this -> attr1 = attr1;
    this -> attr2 = attr2;
    this -> attr3 = attr3;
    this -> attr4 = attr4;
}

void for_astnode::print() {
    cout << "{ \"for\": {\n";
    cout << "\"init\":\n";
    (this -> attr1) -> print();
    cout << ",\n\"guard\":\n";
    (this -> attr2) -> print();
    cout << ",\n\"step\":\n";
    (this -> attr3) -> print();
    cout << ",\n\"body\": \n";
    (this -> attr4) -> print();
    cout << "}\n}\n";
}

op_binary_astnode::op_binary_astnode(string attr1, class exp_astnode* attr2, class exp_astnode* attr3) {
    this -> attr1 = attr1;
    this -> attr2 = attr2;
    this -> attr3 = attr3;
}

void op_binary_astnode::print() {
    cout << "{ \"op_binary\": {\n";
    cout << "\"op\": \"" << this -> attr1 << "\"\n";
    cout << ",\n\"left\":\n";
    (this -> attr2) -> print();
    cout << ",\n\"right\":\n";
    (this -> attr3) -> print();
    cout << "}\n}\n";
}

op_unary_astnode::op_unary_astnode(string attr1, class exp_astnode* attr2) {
    this -> attr1 = attr1;
    this -> attr2 = attr2;
}

void op_unary_astnode::print() {
    cout << "{ \"op_unary\": {\n";
    cout << "\"op\": \"" << this -> attr1 << "\"\n";
    cout << ",\n\"child\":\n";
    (this -> attr2) -> print();
    cout << "}\n}\n";
}

assignE_exp::assignE_exp(class exp_astnode* attr1, class exp_astnode* attr2) {
    this -> attr1 = attr1;
    this -> attr2 = attr2;
}

class exp_astnode* assignE_exp::get_attr1() {
    return this -> attr1;
}

class exp_astnode* assignE_exp::get_attr2() {
    return this -> attr2;
}

void assignE_exp::print() {
    cout << "{ \"assignE\": {\n";
    cout <<"\"left\":\n";
    (this -> attr1) -> print();
    cout << ",\n";
    cout << "\"right\":\n";
    (this -> attr2) -> print();
    cout << "}\n}\n";
}

funcall_astnode::funcall_astnode(vector<class exp_astnode*> attr1) {
    this -> attr1 = attr1;
}

void funcall_astnode::print() {
    cout << "{ \"funcall\": {\n";
    cout << "\"fname\":\n";
    (this -> attr1)[0] -> print();
    cout << ",\n\"params\": [\n";
    int sz = (this -> attr1).size();
    for(int ind = 1; ind < sz; ind ++) {
        class exp_astnode* s = (this -> attr1)[ind];
        s -> print();
        if(ind != sz-1) cout << ",\n";
    }
    cout << "]\n}\n}\n";
}

floatconst_astnode::floatconst_astnode(float attr1) {
    this -> attr1 = attr1;
}

void floatconst_astnode::print() {
    cout << "{\n" << "\"floatconst\": " << this -> attr1 << "}\n";
}

intconst_astnode::intconst_astnode(int attr1) {
    this -> attr1 = attr1;
}

void intconst_astnode::print() {
    cout << "{\n" << "\"intconst\": " << this -> attr1 << "}\n";
}

string_astnode::string_astnode(string attr1) {
    this -> attr1 = attr1;
}

void string_astnode::print() {
    cout << "{\n" << "\"stringconst\": " << this -> attr1 << "}\n";
}

identifier_astnode::identifier_astnode(string attr1) {
    this -> attr1 = attr1;
}

void identifier_astnode::print() {
    cout << "{\n" << "\"identifier\": \"" << this -> attr1 << "\"}\n";
}

member_astnode::member_astnode(class exp_astnode* attr1, class identifier_astnode* attr2) {
    this -> attr1 = attr1;
    this -> attr2 = attr2;
}

void member_astnode::print() {
    cout << "{ \"member\": {\n";
    cout << "\"struct\":\n";
    (this -> attr1) -> print();
    cout << ",\n\"field\":\n";
    (this -> attr2) -> print();
    cout << "}\n}\n";
}

arrow_astnode::arrow_astnode(class exp_astnode* attr1, class identifier_astnode* attr2) {
    this -> attr1 = attr1;
    this -> attr2 = attr2;
}

void arrow_astnode::print() {
    cout << "{ \"arrow\": {\n";
    cout << "\"pointer\":\n";
    (this -> attr1) -> print();
    cout << ",\n\"field\":\n";
    (this -> attr2) -> print();
    cout << "}\n}\n";
}

arrayref_astnode::arrayref_astnode(class exp_astnode* attr1, class exp_astnode* attr2) {
    this -> attr1 = attr1;
    this -> attr2 = attr2;
}

void arrayref_astnode::print() {
    cout << "{ \"arrayref\": {\n";
    cout << "\"array\":\n";
    (this -> attr1) -> print();
    cout << ",\n\"index\":\n";
    (this -> attr2) -> print();
    cout << "}\n}\n";
}