#include "ast.hh"
int label = 0;
int lc_index = 0;
map<std::string, std::string> predefined {
    {"scanf", "void"},
    {"mod", "int"}
};
class types_info* t;

class types_info* exp_astnode::get_type_info() {
    return this -> types;
}

void exp_astnode::set_type_info(class types_info* types) {
    this -> types = types;
}

seq_astnode::seq_astnode(vector<class statement_astnode*> attr1) {
    this -> attr1 = attr1;
}

vector<class statement_astnode*> seq_astnode::get_attr1() {
    return this -> attr1;
}

void seq_astnode::gen(vector<string> &gen_code, vector<string> &rodata, map<string, class Symbols*> gst, map<string, class Symbols*> lst, string func_name, string return_type, map<std::string, class ST*> symtab_map) {
    for(class statement_astnode* s: this -> attr1) {
        s -> gen(gen_code, rodata, gst, lst, func_name, return_type, symtab_map);
    }
}

assignS_astnode::assignS_astnode(class assignE_exp* expr) {
    this -> attr1 = expr -> get_attr1();
    this -> attr2 = expr -> get_attr2();
}

void assignS_astnode::gen(vector<string> &gen_code, vector<string> &rodata, map<string, class Symbols*> gst, map<string, class Symbols*> lst, string func_name, string return_type, map<std::string, class ST*> symtab_map) {
    (this -> attr1) -> gen(gen_code, rodata, gst, lst, func_name, return_type, symtab_map);
    gen_code.push_back("\tpushl\t%ebx");
    (this -> attr2) -> gen(gen_code, rodata, gst, lst, func_name, return_type, symtab_map);
    t = (this -> attr1) -> get_type_info();
    if(t -> isStruct()) {
        int sz = gst[t -> basetype] -> get_size();
        gen_code.push_back("\tleal\t" + to_string(sz) + "(%esp), %ebx");
        gen_code.push_back("\tmovl\t(%ebx), %ebx");
        for(int i = sz-4; i >= 0; i -= 4) {
            gen_code.push_back("\tleal\t" + to_string(i) + "(%esp), %eax");
            gen_code.push_back("\tmovl\t(%eax), %eax");
            gen_code.push_back("\tmovl\t%eax, " + to_string(i) + "(%ebx)");
        }
        for(int i = 0; i <= sz; i += 4) {
            gen_code.push_back("\tpopl\t%ebx");
        }
        for(int i = 0; i < sz; i += 4) {
            gen_code.push_back("\tpopl\t%eax");
        }
    }
    else {
        gen_code.push_back("\tpopl\t%ebx");
        gen_code.push_back("\tmovl\t%eax, (%ebx)");
    }
}

return_expnode::return_expnode(class exp_astnode* attr1) {
    this -> attr1 = attr1;
}

void return_expnode::gen(vector<string> &gen_code, vector<string> &rodata, map<string, class Symbols*> gst, map<string, class Symbols*> lst, string func_name, string return_type, map<std::string, class ST*> symtab_map) {
    (this -> attr1) -> gen(gen_code, rodata, gst, lst, func_name, return_type, symtab_map);
    t = (this -> attr1) -> get_type_info();
    if(t -> isStruct()) {
        int sz = gst[t -> basetype] -> get_size();
        int off = 8;
        for(pair<string, class Symbols*> s: lst) {
            class Symbols* sym = s.second;
            if(!(sym -> is_local_var())) {
                off += sym -> get_size();
            }
        }
        gen_code.push_back("\tleal\t" + to_string(off) + "(%ebp), %ebx");
        for(int i = sz-4; i >= 0; i -= 4) {
            gen_code.push_back("\tleal\t" + to_string(i) + "(%esp), %eax");
            gen_code.push_back("\tmovl\t(%eax), %eax");
            gen_code.push_back("\tmovl\t%eax, " + to_string(i) + "(%ebx)");
        }
        for(int i = 0; i < sz; i += 4) {
            gen_code.push_back("\tpopl\t%ebx");
        }
    }
    int tot_local_size = 0;
    for(std::pair<std::string, class Symbols*> s: lst) {
        class Symbols* sym = s.second;
        if(sym -> is_local_var()) {
            tot_local_size += sym -> get_size();
        }
    }
    gen_code.push_back("\taddl\t$" + to_string(tot_local_size) + ", %esp");
    gen_code.push_back("\tleave");
    gen_code.push_back("\tret");
    gen_code.push_back("\t.size\t" + func_name + ", .-" + func_name);
} 

proccall_astnode::proccall_astnode(vector<class exp_astnode*> attr1, string call_name) {
    this -> attr1 = attr1;
    this -> call_name = call_name;
}

proccall_astnode::proccall_astnode(vector<class exp_astnode*> attr1, string call_name, string ro_string) {
    this -> attr1 = attr1;
    this -> call_name = call_name;
    this -> ro_string = ro_string;
}

void proccall_astnode::gen(vector<string> &gen_code, vector<string> &rodata, map<string, class Symbols*> gst, map<string, class Symbols*> lst, string func_name, string return_type, map<std::string, class ST*> symtab_map) {
    if(this -> call_name != "printf") {
        vector<class exp_astnode*> vec = this -> attr1;
        vec.erase(vec.begin());
        if(gst.find(this -> call_name) != gst.end()) {
            t = new class types_info(gst[this -> call_name] -> get_returntype());
        }
        else if(func_name == this -> call_name) {
            t = new class types_info(return_type);
        }
        else {
            t = new class types_info(predefined[this -> call_name]);
        }
        int space = 0;
        if(t -> isStruct()) {
            space = gst[t -> basetype] -> get_size();
            gen_code.push_back("\tsubl\t$" + to_string(space) + ", %esp");
        }
        int sz = 0;
        for(class exp_astnode* exp: vec) {
            exp -> gen(gen_code, rodata, gst, lst, func_name, return_type, symtab_map);
            t = exp -> get_type_info();
            if(!(t -> isStruct())) {
                gen_code.push_back("\tpushl\t%eax");
                sz += 4;
            }
            else {
                sz += gst[t -> basetype] -> get_size();
            }
        }
        gen_code.push_back("\tcall\t" + this -> call_name);
        gen_code.push_back("\taddl\t$" + to_string(sz + space) + ", %esp");
    }
    else {
        vector<class exp_astnode*> vec = this -> attr1;
        reverse(vec.begin(), vec.end());
        vec.pop_back();
        vec.pop_back();
        for(class exp_astnode* exp: vec) {
            exp -> gen(gen_code, rodata, gst, lst, func_name, return_type, symtab_map);
            gen_code.push_back("\tpushl\t%eax");
        }
        rodata.push_back(".LC" + to_string(lc_index) + ":");
        rodata.push_back("\t.string " + this -> ro_string);
        gen_code.push_back("\tpushl\t$.LC" + to_string(lc_index));
        gen_code.push_back("\tcall\tprintf");
        gen_code.push_back("\taddl\t$" + to_string(4*(vec.size()+1)) + ", %esp");
        lc_index += 1;
    }
}

if_astnode::if_astnode(class exp_astnode* attr1, class statement_astnode* attr2, class statement_astnode* attr3) {
    this -> attr1 = attr1;
    this -> attr2 = attr2;
    this -> attr3 = attr3;
}

void if_astnode::gen(vector<string> &gen_code, vector<string> &rodata, map<string, class Symbols*> gst, map<string, class Symbols*> lst, string func_name, string return_type, map<std::string, class ST*> symtab_map) {
    (this -> attr1) -> gen(gen_code, rodata, gst, lst, func_name, return_type, symtab_map);
    gen_code.push_back("\tcmpl\t$0, %eax");
    gen_code.push_back("\tje\t.L" + to_string(label));
    int instr = label;
    label += 1;
    (this -> attr2) -> gen(gen_code, rodata, gst, lst, func_name, return_type, symtab_map);
    gen_code.push_back("\tjmp\t.L" + to_string(label));
    int next_instr = label;
    label += 1;
    gen_code.push_back(".L" + to_string(instr) + ":");
    (this -> attr3) -> gen(gen_code, rodata, gst, lst, func_name, return_type, symtab_map);
    gen_code.push_back(".L" + to_string(next_instr) + ":");
}

while_astnode::while_astnode(class exp_astnode* attr1, class statement_astnode* attr2) {
    this -> attr1 = attr1;
    this -> attr2 = attr2;
}

void while_astnode::gen(vector<string> &gen_code, vector<string> &rodata, map<string, class Symbols*> gst, map<string, class Symbols*> lst, string func_name, string return_type, map<std::string, class ST*> symtab_map) {
    gen_code.push_back(".L" + to_string(label) + ":");
    int instr = label;
    label += 1;
    (this -> attr1) -> gen(gen_code, rodata, gst, lst, func_name, return_type, symtab_map);
    gen_code.push_back("\tcmpl\t$0, %eax");
    gen_code.push_back("\tje\t.L" + to_string(label));
    int next_instr = label;
    label += 1;
    (this -> attr2) -> gen(gen_code, rodata, gst, lst, func_name, return_type, symtab_map);
    gen_code.push_back("\tjmp\t.L" + to_string(instr));
    gen_code.push_back(".L" + to_string(next_instr) + ":");
}

for_astnode::for_astnode(class exp_astnode* attr1, class exp_astnode* attr2, class exp_astnode* attr3, class statement_astnode* attr4) {
    this -> attr1 = attr1;
    this -> attr2 = attr2;
    this -> attr3 = attr3;
    this -> attr4 = attr4;
}

void for_astnode::gen(vector<string> &gen_code, vector<string> &rodata, map<string, class Symbols*> gst, map<string, class Symbols*> lst, string func_name, string return_type, map<std::string, class ST*> symtab_map) {
    (this -> attr1) -> gen(gen_code, rodata, gst, lst, func_name, return_type, symtab_map);
    gen_code.push_back(".L" + to_string(label) + ":");
    int instr = label;
    label += 1;
    (this -> attr2) -> gen(gen_code, rodata, gst, lst, func_name, return_type, symtab_map);
    gen_code.push_back("\tcmpl\t$0, %eax");
    gen_code.push_back("\tje\t.L" + to_string(label));
    int another_instr = label;
    label += 1;
    gen_code.push_back("\tjmp\t.L" + to_string(label));
    int more_instr = label;
    label += 1;
    gen_code.push_back(".L" + to_string(label) + ":");
    int next_instr = label;
    label += 1;
    (this -> attr3) -> gen(gen_code, rodata, gst, lst, func_name, return_type, symtab_map);
    gen_code.push_back("\tjmp\t.L" + to_string(instr));
    gen_code.push_back(".L" + to_string(more_instr) + ":");
    (this -> attr4) -> gen(gen_code, rodata, gst, lst, func_name, return_type, symtab_map);
    gen_code.push_back("\tjmp\t.L" + to_string(next_instr));
    gen_code.push_back(".L" + to_string(another_instr) + ":");
}

op_binary_astnode::op_binary_astnode(string attr1, class exp_astnode* attr2, class exp_astnode* attr3) {
    this -> attr1 = attr1;
    this -> attr2 = attr2;
    this -> attr3 = attr3;
}

void op_binary_astnode::gen(vector<string> &gen_code, vector<string> &rodata, map<string, class Symbols*> gst, map<string, class Symbols*> lst, string func_name, string return_type, map<std::string, class ST*> symtab_map) {
    if(this -> attr1 == "OR_OP") {
        (this -> attr2) -> gen(gen_code, rodata, gst, lst, func_name, return_type, symtab_map);
        gen_code.push_back("\tcmpl\t$0, %eax");
        gen_code.push_back("\tjne\t.L" + to_string(label));
        int instr = label;
        label += 1;
        (this -> attr3) -> gen(gen_code, rodata, gst, lst, func_name, return_type, symtab_map);
        gen_code.push_back("\tcmpl\t$0, %eax");
        gen_code.push_back("\tjne\t.L" + to_string(instr));
        gen_code.push_back("\tmovl\t$0, %eax");
        gen_code.push_back("\tjmp\t.L" + to_string(label));
        int next_instr = label;
        label += 1;
        gen_code.push_back(".L" + to_string(instr) + ":");
        gen_code.push_back("\tmovl\t$1, %eax");
        gen_code.push_back(".L" + to_string(next_instr) + ":");
    }

    else if(this -> attr1 == "AND_OP") {
        (this -> attr2) -> gen(gen_code, rodata, gst, lst, func_name, return_type, symtab_map);
        gen_code.push_back("\tcmpl\t$0, %eax");
        gen_code.push_back("\tje\t.L" + to_string(label));
        int instr = label;
        label += 1;
        (this -> attr3) -> gen(gen_code, rodata, gst, lst, func_name, return_type, symtab_map);
        gen_code.push_back("\tcmpl\t$0, %eax");
        gen_code.push_back("\tje\t.L" + to_string(instr));
        gen_code.push_back("\tmovl\t$1, %eax");
        gen_code.push_back("\tjmp\t.L" + to_string(label));
        int next_instr = label;
        label += 1;
        gen_code.push_back(".L" + to_string(instr) + ":");
        gen_code.push_back("\tmovl\t$0, %eax");
        gen_code.push_back(".L" + to_string(next_instr) + ":");
    }
    else if(this -> attr1 == "EQ_OP_INT") {
        (this -> attr2) -> gen(gen_code, rodata, gst, lst, func_name, return_type, symtab_map);
        gen_code.push_back("\tpushl\t%eax");
        (this -> attr3) -> gen(gen_code, rodata, gst, lst, func_name, return_type, symtab_map);
        gen_code.push_back("\tpopl\t%ecx");
        gen_code.push_back("\tcmpl\t%ecx, %eax");
        gen_code.push_back("\tsete\t%al");
        gen_code.push_back("\tmovzbl\t%al, %eax");
    }
    else if(this -> attr1 == "NE_OP_INT") {
        (this -> attr2) -> gen(gen_code, rodata, gst, lst, func_name, return_type, symtab_map);
        gen_code.push_back("\tpushl\t%eax");
        (this -> attr3) -> gen(gen_code, rodata, gst, lst, func_name, return_type, symtab_map);
        gen_code.push_back("\tpopl\t%ecx");
        gen_code.push_back("\tcmpl\t%ecx, %eax");
        gen_code.push_back("\tsetne\t%al");
        gen_code.push_back("\tmovzbl\t%al, %eax");
    }
    else if(this -> attr1 == "LT_OP_INT") {
        (this -> attr2) -> gen(gen_code, rodata, gst, lst, func_name, return_type, symtab_map);
        gen_code.push_back("\tpushl\t%eax");
        (this -> attr3) -> gen(gen_code, rodata, gst, lst, func_name, return_type, symtab_map);
        gen_code.push_back("\tpopl\t%ecx");
        gen_code.push_back("\tcmpl\t%eax, %ecx");
        gen_code.push_back("\tsetl\t%al");
        gen_code.push_back("\tmovzbl\t%al, %eax");
    }
    else if(this -> attr1 == "GT_OP_INT") {
        (this -> attr2) -> gen(gen_code, rodata, gst, lst, func_name, return_type, symtab_map);
        gen_code.push_back("\tpushl\t%eax");
        (this -> attr3) -> gen(gen_code, rodata, gst, lst, func_name, return_type, symtab_map);
        gen_code.push_back("\tpopl\t%ecx");
        gen_code.push_back("\tcmpl\t%eax, %ecx");
        gen_code.push_back("\tsetg\t%al");
        gen_code.push_back("\tmovzbl\t%al, %eax");
    }
    else if(this -> attr1 == "LE_OP_INT") {
        (this -> attr2) -> gen(gen_code, rodata, gst, lst, func_name, return_type, symtab_map);
        gen_code.push_back("\tpushl\t%eax");
        (this -> attr3) -> gen(gen_code, rodata, gst, lst, func_name, return_type, symtab_map);
        gen_code.push_back("\tpopl\t%ecx");
        gen_code.push_back("\tcmpl\t%eax, %ecx");
        gen_code.push_back("\tsetle\t%al");
        gen_code.push_back("\tmovzbl\t%al, %eax");
    }
    else if(this -> attr1 == "GE_OP_INT") {
        (this -> attr2) -> gen(gen_code, rodata, gst, lst, func_name, return_type, symtab_map);
        gen_code.push_back("\tpushl\t%eax");
        (this -> attr3) -> gen(gen_code, rodata, gst, lst, func_name, return_type, symtab_map);
        gen_code.push_back("\tpopl\t%ecx");
        gen_code.push_back("\tcmpl\t%eax, %ecx");
        gen_code.push_back("\tsetge\t%al");
        gen_code.push_back("\tmovzbl\t%al, %eax");
    }
    else if(this -> attr1 == "PLUS_INT") {
        (this -> attr3) -> gen(gen_code, rodata, gst, lst, func_name, return_type, symtab_map);
        gen_code.push_back("\tpushl\t%eax");
        (this -> attr2) -> gen(gen_code, rodata, gst, lst, func_name, return_type, symtab_map);
        gen_code.push_back("\tpopl\t%ecx");
        t = (this -> attr2) -> get_type_info();
        if(!(t -> isBaseType())) {
            int mult = 1;
            if(t -> arr_size > 0) {
                queue<int> q = t -> arr_const;
                q.pop();
                while(q.size() > 0) {
                    int top = q.front();
                    mult *= top;
                    q.pop();
                }
            }
            if(t -> num_pointers > 0) mult *= 4;
            else {
                if(t -> basetype == "int") mult *= 4;
                else mult *= gst[t -> basetype] -> get_size();
            }
            gen_code.push_back("\timull\t$" + to_string(mult) + ", %ecx");
        }
        t = (this -> attr3) -> get_type_info();
        if(!(t -> isBaseType())) {
            int mult = 1;
            if(t -> arr_size > 0) {
                queue<int> q = t -> arr_const;
                q.pop();
                while(q.size() > 0) {
                    int top = q.front();
                    mult *= top;
                    q.pop();
                }
            }
            if(t -> num_pointers > 0) mult *= 4;
            else {
                if(t -> basetype == "int") mult *= 4;
                else mult *= gst[t -> basetype] -> get_size();
            }
            gen_code.push_back("\timull\t$" + to_string(mult) + ", %eax");
        }
        gen_code.push_back("\taddl\t%ecx, %eax");        
    }
    else if(this -> attr1 == "MINUS_INT") {
        (this -> attr3) -> gen(gen_code, rodata, gst, lst, func_name, return_type, symtab_map);
        gen_code.push_back("\tpushl\t%eax");
        (this -> attr2) -> gen(gen_code, rodata, gst, lst, func_name, return_type, symtab_map);
        gen_code.push_back("\tpopl\t%ecx");
        t = (this -> attr2) -> get_type_info();
        if(!(t -> isBaseType())) {
            t = (this -> attr3) -> get_type_info();
            if(!(t -> isBaseType())) {
                gen_code.push_back("\tsubl\t%ecx, %eax");
                int mult = 1;
                if(t -> arr_size > 0) {
                    queue<int> q = t -> arr_const;
                    q.pop();
                    while(q.size() > 0) {
                        int top = q.front();
                        mult *= top;
                        q.pop();
                    }
                }
                if(t -> num_pointers > 0) mult *= 4;
                else {
                    if(t -> basetype == "int") mult *= 4;
                    else mult *= gst[t -> basetype] -> get_size();
                }
                gen_code.push_back("\tcltd");
                gen_code.push_back("\tmovl\t$" + to_string(mult) + ", %ecx");
                gen_code.push_back("\tidivl\t%ecx");
            }
            else {
                int mult = 1;
                if(t -> arr_size > 0) {
                    queue<int> q = t -> arr_const;
                    q.pop();
                    while(q.size() > 0) {
                        int top = q.front();
                        mult *= top;
                        q.pop();
                    }
                }
                if(t -> num_pointers > 0) mult *= 4;
                else {
                    if(t -> basetype == "int") mult *= 4;
                    else mult *= gst[t -> basetype] -> get_size();
                }
                gen_code.push_back("\timull\t$" + to_string(mult) + ", %ecx");
                gen_code.push_back("\tsubl\t%ecx, %eax");
            }
        }
        else {
            gen_code.push_back("\tsubl\t%ecx, %eax");
        }
    }
    else if(this -> attr1 == "MULT_INT") {
        (this -> attr3) -> gen(gen_code, rodata, gst, lst, func_name, return_type, symtab_map);
        gen_code.push_back("\tpushl\t%eax");
        (this -> attr2) -> gen(gen_code, rodata, gst, lst, func_name, return_type, symtab_map);
        gen_code.push_back("\tpopl\t%ecx");
        gen_code.push_back("\timull\t%ecx, %eax");
    }
    else if(this -> attr1 == "DIV_INT") {
        (this -> attr3) -> gen(gen_code, rodata, gst, lst, func_name, return_type, symtab_map);
        gen_code.push_back("\tpushl\t%eax");
        (this -> attr2) -> gen(gen_code, rodata, gst, lst, func_name, return_type, symtab_map);
        gen_code.push_back("\tcltd");
        gen_code.push_back("\tpopl\t%ecx");
        gen_code.push_back("\tidivl\t%ecx");
    }
}

op_unary_astnode::op_unary_astnode(string attr1, class exp_astnode* attr2) {
    this -> attr1 = attr1;
    this -> attr2 = attr2;
}

void op_unary_astnode::gen(vector<string> &gen_code, vector<string> &rodata, map<string, class Symbols*> gst, map<string, class Symbols*> lst, string func_name, string return_type, map<std::string, class ST*> symtab_map) {
    if(this -> attr1 == "UMINUS") {
        (this -> attr2) -> gen(gen_code, rodata, gst, lst, func_name, return_type, symtab_map);
        gen_code.push_back("\tnegl\t%eax");
    }
    else if(this -> attr1 == "NOT") {
        (this -> attr2) -> gen(gen_code, rodata, gst, lst, func_name, return_type, symtab_map);
        gen_code.push_back("\tcmpl\t$0, %eax");
        gen_code.push_back("\tsete\t%al");
        gen_code.push_back("\tmovzbl\t%al, %eax");
    }
    else if(this -> attr1 == "ADDRESS") {
        (this -> attr2) -> gen(gen_code, rodata, gst, lst, func_name, return_type, symtab_map);
        gen_code.push_back("\tmovl\t%ebx, %eax");
        t = (this -> attr2) -> get_type_info();
        if(t -> isStruct()) {
            int sz = gst[t -> basetype] -> get_size();
            for(int i = 0; i < sz; i += 4) {
                gen_code.push_back("\tpopl\t%ecx");
            }
        }
    }
    else if(this -> attr1 == "DEREF") {
        (this -> attr2) -> gen(gen_code, rodata, gst, lst, func_name, return_type, symtab_map);
        t = (this -> attr2) -> get_type_info();
        if(t -> isStructPointer()) {
            gen_code.push_back("\tmovl\t%eax, %ebx");
            int sz = gst[t -> basetype] -> get_size();
            for(int i = sz-4; i >= 0; i -= 4) {
                gen_code.push_back("\tmovl\t" + to_string(i) + "(%ebx), %eax");
                gen_code.push_back("\tpushl\t%eax");
            }
        }
        else {
            gen_code.push_back("\tmovl\t%eax, %ebx");
            gen_code.push_back("\tmovl\t(%eax), %eax");
        }
    }
    else if(this -> attr1 == "PP") {
        (this -> attr2) -> gen(gen_code, rodata, gst, lst, func_name, return_type, symtab_map);
        t = (this -> attr2) -> get_type_info();
        gen_code.push_back("\tmovl\t%eax, %ecx");
        if(!(t -> isBaseType())) {
            gen_code.push_back("\taddl\t$4, %ecx");
        }
        else {
            gen_code.push_back("\taddl\t$1, %ecx");
        }
        gen_code.push_back("\tmovl\t%ecx, (%ebx)");
    }
    else if(this -> attr1 == "TO_INT") {
        (this -> attr2) -> gen(gen_code, rodata, gst, lst, func_name, return_type, symtab_map);
    }
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

void assignE_exp::gen(vector<string> &gen_code, vector<string> &rodata, map<string, class Symbols*> gst, map<string, class Symbols*> lst, string func_name, string return_type, map<std::string, class ST*> symtab_map) {
    (this -> attr1) -> gen(gen_code, rodata, gst, lst, func_name, return_type, symtab_map);
    gen_code.push_back("\tpushl\t%ebx");
    (this -> attr2) -> gen(gen_code, rodata, gst, lst, func_name, return_type, symtab_map);
    t = (this -> attr1) -> get_type_info();
    if(t -> isStruct()) {
        int sz = gst[t -> basetype] -> get_size();
        gen_code.push_back("\tleal\t" + to_string(sz) + "(%esp), %ebx");
        gen_code.push_back("\tmovl\t(%ebx), %ebx");
        for(int i = sz-4; i >= 0; i -= 4) {
            gen_code.push_back("\tleal\t" + to_string(i) + "(%esp), %eax");
            gen_code.push_back("\tmovl\t(%eax), %eax");
            gen_code.push_back("\tmovl\t%eax, " + to_string(i) + "(%ebx)");
        }
        for(int i = 0; i <= sz; i += 4) {
            gen_code.push_back("\tpopl\t%ebx");
        }
        for(int i = 0; i < sz; i += 4) {
            gen_code.push_back("\tpopl\t%eax");
        }
    }
    else {
        gen_code.push_back("\tpopl\t%ebx");
        gen_code.push_back("\tmovl\t%eax, (%ebx)");
    }    
}

funcall_astnode::funcall_astnode(vector<class exp_astnode*> attr1, string call_name) {
    this -> attr1 = attr1;
    this -> call_name = call_name;
}

void funcall_astnode::gen(vector<string> &gen_code, vector<string> &rodata, map<string, class Symbols*> gst, map<string, class Symbols*> lst, string func_name, string return_type, map<std::string, class ST*> symtab_map) {
    vector<class exp_astnode*> vec = this -> attr1;
    vec.erase(vec.begin());
    if(gst.find(this -> call_name) != gst.end()) {
        t = new class types_info(gst[this -> call_name] -> get_returntype());
    }
    else if(func_name == this -> call_name) {
        t = new class types_info(return_type);
    }
    else {
        t = new class types_info(predefined[this -> call_name]);
    }
    if(t -> isStruct()) {
        int space = gst[t -> basetype] -> get_size();
        gen_code.push_back("\tsubl\t$" + to_string(space) + ", %esp");
    }
    int sz = 0;
    for(class exp_astnode* exp: vec) {
        exp -> gen(gen_code, rodata, gst, lst, func_name, return_type, symtab_map);
        t = exp -> get_type_info();
        if(!(t -> isStruct())) {
            gen_code.push_back("\tpushl\t%eax");
            sz += 4;
        }
        else {
            sz += gst[t -> basetype] -> get_size();
        }
    }
    gen_code.push_back("\tcall\t" + this -> call_name);
    gen_code.push_back("\taddl\t$" + to_string(sz) + ", %esp");
}

floatconst_astnode::floatconst_astnode(float attr1) {
    this -> attr1 = attr1;
}

void floatconst_astnode::gen(vector<string> &gen_code, vector<string> &rodata, map<string, class Symbols*> gst, map<string, class Symbols*> lst, string func_name, string return_type, map<std::string, class ST*> symtab_map) {
    
}

intconst_astnode::intconst_astnode(int attr1) {
    this -> attr1 = attr1;
}

void intconst_astnode::gen(vector<string> &gen_code, vector<string> &rodata, map<string, class Symbols*> gst, map<string, class Symbols*> lst, string func_name, string return_type, map<std::string, class ST*> symtab_map) {
    gen_code.push_back("\tmovl\t$" + to_string(this -> attr1) + ", %eax");
}

string_astnode::string_astnode(string attr1) {
    this -> attr1 = attr1;
}

void string_astnode::gen(vector<string> &gen_code, vector<string> &rodata, map<string, class Symbols*> gst, map<string, class Symbols*> lst, string func_name, string return_type, map<std::string, class ST*> symtab_map) {
    
}

identifier_astnode::identifier_astnode(string attr1) {
    this -> attr1 = attr1;
}

string identifier_astnode :: get_attr1() {
    return this -> attr1;
}

void identifier_astnode::gen(vector<string> &gen_code, vector<string> &rodata, map<string, class Symbols*> gst, map<string, class Symbols*> lst, string func_name, string return_type, map<std::string, class ST*> symtab_map) {
    t = lst[this -> attr1] -> get_type_info();
    bool param = !(lst[this -> attr1] -> is_local_var());
    int off = lst[this -> attr1] -> get_offset();
    gen_code.push_back("\tleal\t" + to_string(off) + "(%ebp), %ebx");
    if(t -> arr_size > 0) {
        if(param) {
            gen_code.push_back("\tmovl\t(%ebx), %ebx");
        }
        gen_code.push_back("\tmovl\t%ebx, %eax");
    }
    else if(t -> isStruct()) {
        int sz = lst[this -> attr1] -> get_size();
        for(int i = sz-4; i >= 0; i -= 4) {
            gen_code.push_back("\tmovl\t" + to_string(off + i) + "(%ebp), %eax");
            gen_code.push_back("\tpushl\t%eax");
        }
    }
    else {
        gen_code.push_back("\tmovl\t(%ebx), %eax");
    }
}

member_astnode::member_astnode(class exp_astnode* attr1, class identifier_astnode* attr2) {
    this -> attr1 = attr1;
    this -> attr2 = attr2;
}

void member_astnode::gen(vector<string> &gen_code, vector<string> &rodata, map<string, class Symbols*> gst, map<string, class Symbols*> lst, string func_name, string return_type, map<std::string, class ST*> symtab_map) {
    (this -> attr1) -> gen(gen_code, rodata, gst, lst, func_name, return_type, symtab_map);
    t = (this -> attr1) -> get_type_info();
    int sz = gst[t -> basetype] -> get_size();
    int off = (symtab_map[t -> basetype] -> get_elems())[(this -> attr2) -> get_attr1()] -> get_offset();
    int new_sz = (symtab_map[t -> basetype] -> get_elems())[(this -> attr2) -> get_attr1()] -> get_size();
    t = (symtab_map[t -> basetype] -> get_elems())[(this -> attr2) -> get_attr1()] -> get_type_info();
    if(t -> isStruct()) {
        gen_code.push_back("\tleal\t" + to_string(off) + "(%ebx), %ecx");
        gen_code.push_back("\tleal\t" + to_string(off) + "(%esp), %ebx");
        for(int i = new_sz-4; i >= 0; i -= 4) {
            gen_code.push_back("\tmovl\t" + to_string(i) + "(%ebx), %eax");
            gen_code.push_back("\tpushl\t%eax");
        }
        for(int i = new_sz-4; i >= 0; i -= 4) {
            gen_code.push_back("\tleal\t" + to_string(i) + "(%esp), %eax");
            gen_code.push_back("\tleal\t" + to_string(sz + i) + "(%esp), %ebx");
            gen_code.push_back("\tmovl\t(%eax), %eax");
            gen_code.push_back("\tmovl\t%eax, (%ebx)");
        }
        for(int i = 0; i < sz; i += 4) {
            gen_code.push_back("\tpopl\t%eax");
        }
        gen_code.push_back("\tmovl\t%ecx, %ebx");
    }
    else {
        gen_code.push_back("\taddl\t$" + to_string(off) + ", %ebx");
        if(t -> arr_size > 0) {
            gen_code.push_back("\tmovl\t%ebx, %eax");
        }
        else {
            gen_code.push_back("\tleal\t" + to_string(off) + "(%esp), %eax");
            gen_code.push_back("\tmovl\t(%eax), %eax");
        }
        for(int i = 0; i < sz; i += 4) {
            gen_code.push_back("\tpopl\t%ecx");
        }
    }
}

arrow_astnode::arrow_astnode(class exp_astnode* attr1, class identifier_astnode* attr2) {
    this -> attr1 = attr1;
    this -> attr2 = attr2;
}

void arrow_astnode::gen(vector<string> &gen_code, vector<string> &rodata, map<string, class Symbols*> gst, map<string, class Symbols*> lst, string func_name, string return_type, map<std::string, class ST*> symtab_map) {
    (this -> attr1) -> gen(gen_code, rodata, gst, lst, func_name, return_type, symtab_map);
    t = (this -> attr1) -> get_type_info();
    if(t -> arr_size == 0) {
        gen_code.push_back("\tmovl\t(%ebx), %ebx");
    }
    int sz = (symtab_map[t -> basetype] -> get_elems())[(this -> attr2) -> get_attr1()] -> get_size();
    int off = (symtab_map[t -> basetype] -> get_elems())[(this -> attr2) -> get_attr1()] -> get_offset();
    t = (symtab_map[t -> basetype] -> get_elems())[(this -> attr2) -> get_attr1()] -> get_type_info();
    gen_code.push_back("\taddl\t$" + to_string(off) + ", %ebx");
    if(t -> isStruct()) {
        for(int i = sz-4; i >= 0; i -= 4) {
            gen_code.push_back("\tmovl\t" + to_string(i) + "(%ebx), %eax");
            gen_code.push_back("\tpushl\t%eax");
        }
    }
    else if(t -> arr_size > 0) {
        gen_code.push_back("\tmovl\t%ebx, %eax");
    }
    else {
        gen_code.push_back("\tmovl\t(%ebx), %eax");
    }
}

arrayref_astnode::arrayref_astnode(class exp_astnode* attr1, class exp_astnode* attr2) {
    this -> attr1 = attr1;
    this -> attr2 = attr2;
}

void arrayref_astnode::gen(vector<string> &gen_code, vector<string> &rodata, map<string, class Symbols*> gst, map<string, class Symbols*> lst, string func_name, string return_type, map<std::string, class ST*> symtab_map) {
    (this -> attr1) -> gen(gen_code, rodata, gst, lst, func_name, return_type, symtab_map);
    gen_code.push_back("\tpushl\t%eax");
    (this -> attr2) -> gen(gen_code, rodata, gst, lst, func_name, return_type, symtab_map);
    t = (this -> attr1) -> get_type_info();
    queue<int> q = t -> arr_const;
    int mult = 1;
    int np = t -> num_pointers;
    if(t -> arr_pointers > 0) {
        while(q.size() > 0) {
            int top = q.front();
            mult *= top;
            q.pop();
        }   
    }
    else if(t -> arr_size > 0) {
        q.pop();
        while(q.size() > 0) {
            int top = q.front();
            mult *= top;
            q.pop();
        }
    }
    else {
        np -= 1;
    }
    if(np > 0) mult *= 4;
    else {
        if(t -> basetype == "int") mult *= 4;
        else mult *= gst[t -> basetype] -> get_size();
    }
    gen_code.push_back("\timull\t$" + to_string(mult) + ", %eax");
    gen_code.push_back("\tpopl\t%ebx");
    gen_code.push_back("\taddl\t%eax, %ebx");
    if((t -> basetype).substr(0,6) == "struct" && (t -> tot_pointers + t -> arr_size == 1)) {
        int sz = gst[t -> basetype] -> get_size();
        for(int i = sz - 4; i >= 0; i -= 4) {
            gen_code.push_back("\tmovl\t" + to_string(i) + "(%ebx), %eax");
            gen_code.push_back("\tpushl\t%eax");
        }
    }
    else if(t -> arr_pointers + t -> arr_size > 1) {
        gen_code.push_back("\tmovl\t%ebx, %eax");
    }
    else {
        gen_code.push_back("\tmovl\t(%ebx), %eax");
    }
}