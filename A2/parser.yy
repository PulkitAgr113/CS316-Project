%skeleton "lalr1.cc"
%require  "3.0.1"

%defines 
%define api.namespace {IPL}
%define api.parser.class {Parser}

%define parse.trace

%code requires{
   namespace IPL {
      class Scanner;
   }

   #include "type.hh"
   #include "symbtab.hh"
   #include "ast.hh"
   #include "location.hh"


  // # ifndef YY_NULLPTR
  // #  if defined __cplusplus && 201103L <= __cplusplus
  // #   define YY_NULLPTR nullptr
  // #  else
  // #   define YY_NULLPTR 0
  // #  endif
  // # endif

}
 
%printer { std::cerr << $$; } STRUCT
%printer { std::cerr << $$; } IF
%printer { std::cerr << $$; } ELSE
%printer { std::cerr << $$; } VOID
%printer { std::cerr << $$; } INT
%printer { std::cerr << $$; } FLOAT
%printer { std::cerr << $$; } WHILE
%printer { std::cerr << $$; } FOR
%printer { std::cerr << $$; } RETURN
%printer { std::cerr << $$; } OR_OP
%printer { std::cerr << $$; } AND_OP
%printer { std::cerr << $$; } EQ_OP
%printer { std::cerr << $$; } NE_OP
%printer { std::cerr << $$; } LE_OP
%printer { std::cerr << $$; } GE_OP
%printer { std::cerr << $$; } INC_OP
%printer { std::cerr << $$; } PTR_OP
%printer { std::cerr << $$; } IDENTIFIER
%printer { std::cerr << $$; } INT_CONSTANT
%printer { std::cerr << $$; } FLOAT_CONSTANT
%printer { std::cerr << $$; } STRING_LITERAL
%printer { std::cerr << $$; } OTHERS


%parse-param { Scanner  &scanner  }
%locations
%code{
   #include <iostream>
   #include <cstdlib>
   #include <fstream>
   #include <string>
   #include <map>
   #include <stack>
   #include <queue>  
   
   #include "scanner.hh"
   map<std::string, class ST*> symtab_map;
   map<std::string, class abstract_astnode*> ast_map;
   class ST* gst = new class ST();
   class ST* lst = new class ST();
   std::string func_name = "";
   std::string var_name;
   int num_pointers = 0;
   queue<int> arr_const;
   std::string base_type;
   std::string derived_type;
   std::string return_type = "";
   class types_info* types;
   class types_info* first_type;
   class types_info* second_type;
   int var_size;
   stack<class Symbols*> params;
   map<std::string, queue<class types_info*> > param_type_map;
   map<std::string, class Symbols*> temp_st;
   queue<class types_info*> param_types;
   queue<class types_info*> temp_param_list;
   int prev_param_offset = 12;
   int prev_var_offset = 0;
   map<std::string, std::string> predefined {
          {"printf", "void"},
          {"scanf", "void"},
          {"mod", "int"}
   };
   int line_no = 1;

#undef yylex
#define yylex IPL::Parser::scanner.yylex

}




%define api.value.type variant
%define parse.assert

%start translation_unit


%token <std::string> STRUCT
%token <std::string> IF
%token <std::string> ELSE
%token <std::string> VOID
%token <std::string> INT
%token <std::string> FLOAT
%token <std::string> WHILE
%token <std::string> FOR
%token <std::string> RETURN
%token <std::string> OR_OP
%token <std::string> AND_OP
%token <std::string> EQ_OP
%token <std::string> NE_OP
%token <std::string> LE_OP
%token <std::string> GE_OP
%token <std::string> INC_OP
%token <std::string> PTR_OP
%token <std::string> IDENTIFIER
%token <std::string> INT_CONSTANT
%token <std::string> FLOAT_CONSTANT
%token <std::string> STRING_LITERAL
%token <std::string> OTHERS
%token '!' '&' '.' '+' '-' '*' '/' '(' ')' ',' '[' ']' '<' '>' '=' '{' '}' ';' 

%nterm <int> translation_unit struct_specifier function_definition fun_declarator
%nterm <int> parameter_list parameter_declaration declarator declarator_arr compound_statement
%nterm <class seq_astnode*> statement_list
%nterm <class statement_astnode*> statement assignment_statement procedure_call iteration_statement selection_statement
%nterm <class exp_astnode*> expression logical_and_expression equality_expression relational_expression primary_expression
%nterm <class exp_astnode*> additive_expression unary_expression multiplicative_expression postfix_expression
%nterm <int>  declaration_list declaration declarator_list
%nterm <class assignE_exp*> assignment_expression
%nterm <vector <class exp_astnode*> > expression_list 
%nterm <std::string> type_specifier unary_operator
%%
translation_unit:
     struct_specifier
     {

     }

     | function_definition
     {
          
     }

     | translation_unit struct_specifier
     {
          
     }

     | translation_unit function_definition
     {
          
     }
     ;

struct_specifier: 
     STRUCT IDENTIFIER '{' declaration_list '}' ';'
     { 
          std::string name = $1 + " " + $2;
          temp_st = gst -> get_elems();
          if(temp_st.find(name) != temp_st.end()) {
               error(@$, name + " has a previous definition");
          }
          int size = 0;
          for(std::pair<std::string, class Symbols*> s: lst -> get_elems()) {
               class Symbols* sym = s.second;
               size += sym -> get_size();
               sym -> change_offset(-(sym -> get_offset()));
          }
          types = new class types_info("-");
          class Symbols* structSym = new class Symbols(name, "struct", "global", size, 0, "-", types);
	     gst -> addSymbol(structSym);
          symtab_map[name] = lst;
          lst = new class ST();
          return_type = "";
          prev_var_offset = 0;
     } 
     ;

function_definition: 
     type_specifier fun_declarator compound_statement
     {
          types = new class types_info($1);
          class Symbols* funcSym = new class Symbols(func_name, "fun", "global", 0, 0, $1, types); 
          gst -> addSymbol(funcSym);
          symtab_map[func_name] = lst;
          param_type_map[func_name] = param_types;
          lst = new class ST();
          prev_var_offset = 0;
          func_name = "";
          return_type = "";
          param_types = queue<class types_info*>();
     }
     ;

type_specifier:
     VOID
     {
          $$ = $1;
          base_type = $$;
          if(return_type == "") {
               return_type = $$;
          }
     }

     | INT
     {
          $$ = $1;
          base_type = $$;
          if(return_type == "") {
               return_type = $$;
          }
     }

     | FLOAT
     {
          $$ = $1;
          base_type = $$;
          if(return_type == "") {
               return_type = $$;
          }
     }

     | STRUCT IDENTIFIER
     {
          $$ = $1 + " " + $2;
          base_type = $$;
          if(return_type == "") {
               return_type = $$;
          }
     }

fun_declarator:
     IDENTIFIER '(' parameter_list ')'
     { 
          temp_st = gst -> get_elems();
          if(temp_st.find($1) != temp_st.end()) {
               error(@$, "The function \"" + $1 + "\" has a previous definition");
          }
          func_name = $1;
          while(params.size() > 0) {
               class Symbols* s = params.top();
               params.pop();
               s -> change_offset(prev_param_offset);
               prev_param_offset += s -> get_size();
               lst -> addSymbol(s);
          }
          prev_param_offset = 12;
     }

     | IDENTIFIER '(' ')'
     { 
          temp_st = gst -> get_elems();
          if(temp_st.find($1) != temp_st.end()) {
               error(@$, "The function \"" + $1 + "\" has a previous definition");
          }
          func_name = $1;
     } 
     ;

parameter_list:
     parameter_declaration
     {
          
     }
     
     | parameter_list ',' parameter_declaration
     { 
	     
     } 
     ;

parameter_declaration: 
     type_specifier declarator
     {
          if($1 == "void" && num_pointers == 0) {
               error(@$, "Cannot declare the type of a parameter as \"void\"");
          }
          else {
               base_type = $1;
               if(num_pointers > 0) var_size = 4;
               else if(base_type == "int" || base_type == "float") var_size = 4;
               else {
                    temp_st = gst -> get_elems();
                    if(temp_st.find(base_type) == temp_st.end()) {
                         error(@$, "\"" + base_type + "\" is not defined");
                    }
                    var_size = temp_st[base_type] -> get_size();
               }
               var_size *= $2;
               types = new class types_info(base_type, num_pointers, 0, arr_const);
               param_types.push(types);
               derived_type = types -> full_type(true);
               class Symbols* paramSym = new class Symbols(var_name, "var", "param", var_size, 0, derived_type, types);
               num_pointers = 0;
               arr_const = queue<int>();
               params.push(paramSym);
          }
     }
     ;

declarator_arr:
     IDENTIFIER
     {
          temp_st = lst -> get_elems();
          if(temp_st.find($1) != temp_st.end()) {
               error(@$, "\"" + $1 + "\" has a previous declaration");
          }
          var_name = $1;
          $$ = 1;
     }

     | declarator_arr '[' INT_CONSTANT ']'
     {
          if(stoi($3) < 0) {
               error(@$, "Size of array is negative");
          }
          arr_const.push(stoi($3));
          $$ = $1 * stoi($3);
     }
     ;

declarator:
     declarator_arr
     {
          $$ = $1;
     }

     | '*' declarator
     {
          num_pointers += 1;
          $$ = $2;
     }
     ;

compound_statement: 
     '{' '}'
     {
          class nostatement* ns = new class nostatement();
          vector<class statement_astnode*> vec;
          vec.push_back(ns);    
          ast_map[func_name] = new class seq_astnode(vec);
     }

     | '{' statement_list '}'
     {
          ast_map[func_name] = $2;
     }

     | '{' declaration_list '}'
     {
          class nostatement* ns = new class nostatement();
          vector<class statement_astnode*> vec;
          vec.push_back(ns);    
          ast_map[func_name] = new class seq_astnode(vec);
     }

     | '{' declaration_list statement_list '}'
     {
          ast_map[func_name] = $3;
     }
     ;

statement_list: 
     statement
     {
          vector<class statement_astnode*> vec;
          vec.push_back($1);    
          $$ = new class seq_astnode(vec);
     }

     | statement_list statement
     {
          vector<class statement_astnode*> vec = $1 -> get_attr1();
          vec.push_back($2);
          $$ = new class seq_astnode(vec);
     }
     ;

statement: 
     ';'
     {
          $$ = new class empty_astnode();
     }

     | '{' statement_list '}'
     {
          $$ = $2;
     }

     | selection_statement
     {
          $$ = $1;
     }

     | iteration_statement
     {
          $$ = $1;
     }

     | assignment_statement
     {
          $$ = $1;
     }
     
     | procedure_call
     {
          $$ = $1;
     }
     
     | RETURN expression ';'
     {
          first_type = $2 -> get_type_info();
          if((first_type -> isInt()) && (return_type == "float")) {
               class op_unary_astnode* change_type = new class op_unary_astnode("TO_FLOAT", $2);
               types = new class types_info("float");
               change_type -> set_type_info(types);
               $$ = new class return_expnode(change_type);
          }
          else if((first_type -> isFloat()) && (return_type == "int")) {
               class op_unary_astnode* change_type = new class op_unary_astnode("TO_INT", $2);
               types = new class types_info("int");
               change_type -> set_type_info(types);
               $$ = new class return_expnode(change_type);
          }
          else if(first_type -> basetype == return_type) $$ = new class return_expnode($2);
          else error(@$, "Incompatible type \"" + first_type -> full_type() + "\" returned, expected \"" + return_type + "\"");
     }
     ;

assignment_expression: 
     unary_expression '=' expression
     {
          first_type = $1 -> get_type_info();
          second_type = $3 -> get_type_info();
          if(!(first_type -> lval)) {
               error(@$, "Left operand of assignment should have an lvalue");
          }
          else {
               bool assignable = first_type -> isCompatiblePointer(second_type) || first_type -> sameBaseType(second_type);
               assignable |= !(first_type -> isBaseType()) && second_type -> isVoidPointer();
               assignable |= first_type -> isVoidPointer() && !(second_type -> isBaseType());
               assignable |= !(first_type -> isBaseType()) && second_type -> isZero;
               assignable |= first_type -> isZero && !(second_type -> isBaseType());
               if(!assignable) {
                    error(@$, "Incompatible assignment when assigning to type \"" + first_type -> full_type() + "\" from type \"" + second_type -> full_type() + "\"");
               }
               else {
                    if((first_type -> isInt()) && (second_type -> isFloat())) {
                         class op_unary_astnode* change_type = new class op_unary_astnode("TO_INT", $3);
                         types = new class types_info("int");
                         change_type -> set_type_info(types);
                         $$ = new class assignE_exp($1, change_type);
                    }
                    else if((first_type -> isFloat()) && (second_type -> isInt())) {
                         class op_unary_astnode* change_type = new class op_unary_astnode("TO_FLOAT", $3);
                         types = new class types_info("float");
                         change_type -> set_type_info(types);
                         $$ = new class assignE_exp($1, change_type);
                    }
                    else $$ = new class assignE_exp($1, $3);     
               }
          }
     }   
     ;

assignment_statement: 
     assignment_expression ';'
     {
          $$ = new class assignS_astnode($1);
     }
     ;

procedure_call: 
     IDENTIFIER '(' ')' ';'
     {
          temp_st = gst -> get_elems();
          if(temp_st.find($1) == temp_st.end() && predefined.find($1) == predefined.end() && func_name != $1) {
               error(@$, "Procedure \"" + $1 + "\" not declared");
          }
          else if(temp_st.find($1) != temp_st.end() && temp_st[$1] -> get_type() != "fun") {
               error(@$, "Called object \"" + $1 + "\" is not a procedure");
          }
          if(temp_st.find($1) != temp_st.end()) {
               temp_param_list = param_type_map[$1];
          }
          else if(func_name == $1) {
               temp_param_list = param_types;
          }
          if(predefined.find($1) == predefined.end()) {
               if(temp_param_list.size() > 0) {
                    error(@$, "\"" + $1 + "\" called with too few arguments");
               }
          }
          class identifier_astnode* iden = new class identifier_astnode($1);
          vector<class exp_astnode*> vec;
          vec.push_back(iden);
          $$ = new class proccall_astnode(vec);
     }

     | IDENTIFIER '(' expression_list ')' ';'
     {
          temp_st = gst -> get_elems();
          if(temp_st.find($1) == temp_st.end() && predefined.find($1) == predefined.end() && func_name != $1) {
               error(@$, "Procedure \"" + $1 + "\" not declared");
          }
          else if(temp_st.find($1) != temp_st.end() && temp_st[$1] -> get_type() != "fun") {
               error(@$, "Called object \"" + $1 + "\" is not a procedure");
          }
          if(temp_st.find($1) != temp_st.end()) {
               temp_param_list = param_type_map[$1];
          }
          else if(func_name == $1) {
               temp_param_list = param_types;
          }
          class identifier_astnode* iden = new class identifier_astnode($1);
          if(temp_st.find($1) != temp_st.end() || func_name == $1) {
               if($3.size() < temp_param_list.size()) {
                    error(@$, "\"" + $1 + "\" called with too few arguments");
               }
               else if($3.size() > temp_param_list.size()) {
                    error(@$, "\"" + $1 + "\" called with too many arguments");
               }
               vector<class exp_astnode*> vec;
               vec.push_back(iden);
               for(class exp_astnode* exp: $3) {
                    first_type = temp_param_list.front();
                    second_type = exp -> get_type_info();
                    bool assignable = first_type -> isCompatiblePointer(second_type) || first_type -> sameBaseType(second_type);
                    assignable |= !(first_type -> isBaseType()) && second_type -> isVoidPointer();
                    assignable |= first_type -> isVoidPointer() && !(second_type -> isBaseType());
                    assignable |= !(first_type -> isBaseType()) && second_type -> isZero;
                    temp_param_list.pop();
                    if(assignable) {
                         if((first_type -> isInt()) && (second_type -> isFloat())) {
                              class op_unary_astnode* change_type = new class op_unary_astnode("TO_INT", exp);
                              types = new class types_info("int");
                              change_type -> set_type_info(types);
                              vec.push_back(change_type);
                         }
                         else if((first_type -> isFloat()) && (second_type -> isInt())) {
                              class op_unary_astnode* change_type = new class op_unary_astnode("TO_FLOAT", exp);
                              types = new class types_info("float");
                              change_type -> set_type_info(types);
                              vec.push_back(change_type);
                         }
                         else vec.push_back(exp);
                    }
                    else {
                         error(@$, "Expected \"" + first_type -> full_type() + "\" but argument is of type \"" + second_type -> full_type() + "\"");
                    }
               }
               $$ = new class proccall_astnode(vec);
          }
          else {
               vector<class exp_astnode*> vec;
               vec.push_back(iden);
               for(class exp_astnode* exp: $3) {
                    vec.push_back(exp);
               }
               $$ = new class proccall_astnode(vec);
          }
     }
     ;

expression: 
     logical_and_expression
     {
          $$ = $1;
     }

     | expression OR_OP logical_and_expression
     {
          first_type = $1 -> get_type_info();
          second_type = $3 -> get_type_info();
          bool no_error = first_type -> isInt() || first_type -> isFloat() || !(first_type -> isBaseType());
          no_error &= second_type -> isInt() || second_type -> isFloat() || !(second_type -> isBaseType());
          if(no_error) {
               $$ = new class op_binary_astnode("OR_OP", $1, $3);
               types = new class types_info("int");
               $$ -> set_type_info(types);
          }
          else {
               error(@$, "Invalid operand of ||, not scalar or pointer");
          }
     }
     ;

logical_and_expression: 
     equality_expression
     {
          $$ = $1;
     }

     | logical_and_expression AND_OP equality_expression
     {
          first_type = $1 -> get_type_info();
          second_type = $3 -> get_type_info();
          bool no_error = first_type -> isInt() || first_type -> isFloat() || !(first_type -> isBaseType());
          no_error &= second_type -> isInt() || second_type -> isFloat() || !(second_type -> isBaseType());
          if(no_error) {
               $$ = new class op_binary_astnode("AND_OP", $1, $3);
               types = new class types_info("int");
               $$ -> set_type_info(types);
          }
          else {
               error(@$, "Invalid operand of &&, not scalar or pointer");
          }
     }
     ;

equality_expression:
     relational_expression
     {
          $$ = $1;
     }

     | equality_expression EQ_OP relational_expression
     {
          first_type = $1 -> get_type_info();
          second_type = $3 -> get_type_info();
          bool no_error = true;
          if(first_type -> isFloat() && second_type -> isInt()) {
               class op_unary_astnode* change_type = new class op_unary_astnode("TO_FLOAT", $3);
               types = new class types_info("float");
               change_type -> set_type_info(types);
               $$ = new class op_binary_astnode("EQ_OP_FLOAT", $1, change_type);
          }
          else if(first_type -> isInt() && second_type -> isFloat()) {
               class op_unary_astnode* change_type = new class op_unary_astnode("TO_FLOAT", $1);
               types = new class types_info("float");
               change_type -> set_type_info(types);
               $$ = new class op_binary_astnode("EQ_OP_FLOAT", change_type, $3);
          }
          else if(first_type -> isInt() && second_type -> isInt()) {
               $$ = new class op_binary_astnode("EQ_OP_INT", $1, $3);
          }
          else if(first_type -> isFloat() && second_type -> isFloat()) {
               $$ = new class op_binary_astnode("EQ_OP_FLOAT", $1, $3);
          }
          else if(first_type -> isCompatiblePointer(second_type)) {
               $$ = new class op_binary_astnode("EQ_OP_INT", $1, $3);
          }
          else if(!(first_type -> isBaseType()) && second_type -> isZero) {
               $$ = new class op_binary_astnode("EQ_OP_INT", $1, $3);
          }
          else if(first_type -> isZero && !(second_type -> isBaseType())) {
               $$ = new class op_binary_astnode("EQ_OP_INT", $1, $3);
          }
          else no_error = false;
          if(no_error) {
               types = new class types_info("int");
               $$ -> set_type_info(types);
          }
          else {
               error(@$, "Invalid operands types for binary == , \"" + first_type -> full_type() + "\" and \"" + second_type -> full_type() + "\"");
          }
     }

     | equality_expression NE_OP relational_expression
     {
          first_type = $1 -> get_type_info();
          second_type = $3 -> get_type_info();
          bool no_error = true;
          if(first_type -> isFloat() && second_type -> isInt()) {
               class op_unary_astnode* change_type = new class op_unary_astnode("TO_FLOAT", $3);
               types = new class types_info("float");
               change_type -> set_type_info(types);
               $$ = new class op_binary_astnode("NE_OP_FLOAT", $1, change_type);
          }
          else if(first_type -> isInt() && second_type -> isFloat()) {
               class op_unary_astnode* change_type = new class op_unary_astnode("TO_FLOAT", $1);
               types = new class types_info("float");
               change_type -> set_type_info(types);
               $$ = new class op_binary_astnode("NE_OP_FLOAT", change_type, $3);
          }
          else if(first_type -> isInt() && second_type -> isInt()) {
               $$ = new class op_binary_astnode("NE_OP_INT", $1, $3);
          }
          else if(first_type -> isFloat() && second_type -> isFloat()) {
               $$ = new class op_binary_astnode("NE_OP_FLOAT", $1, $3);
          }
          else if(first_type -> isCompatiblePointer(second_type)) {
               $$ = new class op_binary_astnode("NE_OP_INT", $1, $3);
          }
          else if(!(first_type -> isBaseType()) && second_type -> isZero) {
               $$ = new class op_binary_astnode("NE_OP_INT", $1, $3);
          }
          else if(first_type -> isZero && !(second_type -> isBaseType())) {
               $$ = new class op_binary_astnode("NE_OP_INT", $1, $3);
          }
          else no_error = false;
          if(no_error) {
               types = new class types_info("int");
               $$ -> set_type_info(types);
          }
          else {
               error(@$, "Invalid operands types for binary != , \"" + first_type -> full_type() + "\" and \"" + second_type -> full_type() + "\"");
          }
     }
     ;

relational_expression: 
     additive_expression
     {
          $$ = $1;
     }

     | relational_expression '<' additive_expression
     {
          first_type = $1 -> get_type_info();
          second_type = $3 -> get_type_info();
          bool no_error = true;
          if(first_type -> isFloat() && second_type -> isInt()) {
               class op_unary_astnode* change_type = new class op_unary_astnode("TO_FLOAT", $3);
               types = new class types_info("float");
               change_type -> set_type_info(types);
               $$ = new class op_binary_astnode("LT_OP_FLOAT", $1, change_type);
          }
          else if(first_type -> isInt() && second_type -> isFloat()) {
               class op_unary_astnode* change_type = new class op_unary_astnode("TO_FLOAT", $1);
               types = new class types_info("float");
               change_type -> set_type_info(types);
               $$ = new class op_binary_astnode("LT_OP_FLOAT", change_type, $3);
          }
          else if(first_type -> isInt() && second_type -> isInt()) {
               $$ = new class op_binary_astnode("LT_OP_INT", $1, $3);
          }
          else if(first_type -> isFloat() && second_type -> isFloat()) {
               $$ = new class op_binary_astnode("LT_OP_FLOAT", $1, $3);
          }
          else if(first_type -> isCompatiblePointer(second_type)) {
               $$ = new class op_binary_astnode("LT_OP_INT", $1, $3);
          }
          else no_error = false;
          if(no_error) {
               types = new class types_info("int");
               $$ -> set_type_info(types);
          }
          else {
               error(@$, "Invalid operands types for binary < , \"" + first_type -> full_type() + "\" and \"" + second_type -> full_type() + "\"");
          }
     }

     | relational_expression '>' additive_expression
     {
          first_type = $1 -> get_type_info();
          second_type = $3 -> get_type_info();
          bool no_error = true;
          if(first_type -> isFloat() && second_type -> isInt()) {
               class op_unary_astnode* change_type = new class op_unary_astnode("TO_FLOAT", $3);
               types = new class types_info("float");
               change_type -> set_type_info(types);
               $$ = new class op_binary_astnode("GT_OP_FLOAT", $1, change_type);
          }
          else if(first_type -> isInt() && second_type -> isFloat()) {
               class op_unary_astnode* change_type = new class op_unary_astnode("TO_FLOAT", $1);
               types = new class types_info("float");
               change_type -> set_type_info(types);
               $$ = new class op_binary_astnode("GT_OP_FLOAT", change_type, $3);
          }
          else if(first_type -> isInt() && second_type -> isInt()) {
               $$ = new class op_binary_astnode("GT_OP_INT", $1, $3);
          }
          else if(first_type -> isFloat() && second_type -> isFloat()) {
               $$ = new class op_binary_astnode("GT_OP_FLOAT", $1, $3);
          }
          else if(first_type -> isCompatiblePointer(second_type)) {
               $$ = new class op_binary_astnode("GT_OP_INT", $1, $3);
          }
          else no_error = false;
          if(no_error) {
               types = new class types_info("int");
               $$ -> set_type_info(types);
          }
          else {
               error(@$, "Invalid operands types for binary > , \"" + first_type -> full_type() + "\" and \"" + second_type -> full_type() + "\"");
          }
     }

     | relational_expression LE_OP additive_expression
     {
          first_type = $1 -> get_type_info();
          second_type = $3 -> get_type_info();
          bool no_error = true;
          if(first_type -> isFloat() && second_type -> isInt()) {
               class op_unary_astnode* change_type = new class op_unary_astnode("TO_FLOAT", $3);
               types = new class types_info("float");
               change_type -> set_type_info(types);
               $$ = new class op_binary_astnode("LE_OP_FLOAT", $1, change_type);
          }
          else if(first_type -> isInt() && second_type -> isFloat()) {
               class op_unary_astnode* change_type = new class op_unary_astnode("TO_FLOAT", $1);
               types = new class types_info("float");
               change_type -> set_type_info(types);
               $$ = new class op_binary_astnode("LE_OP_FLOAT", change_type, $3);
          }
          else if(first_type -> isInt() && second_type -> isInt()) {
               $$ = new class op_binary_astnode("LE_OP_INT", $1, $3);
          }
          else if(first_type -> isFloat() && second_type -> isFloat()) {
               $$ = new class op_binary_astnode("LE_OP_FLOAT", $1, $3);
          }
          else if(first_type -> isCompatiblePointer(second_type)) {
               $$ = new class op_binary_astnode("LE_OP_INT", $1, $3);
          }
          else no_error = false;
          if(no_error) {
               types = new class types_info("int");
               $$ -> set_type_info(types);
          }
          else {
               error(@$, "Invalid operands types for binary <= , \"" + first_type -> full_type() + "\" and \"" + second_type -> full_type() + "\"");
          }
     }

     | relational_expression GE_OP additive_expression
     {
          first_type = $1 -> get_type_info();
          second_type = $3 -> get_type_info();
          bool no_error = true;
          if(first_type -> isFloat() && second_type -> isInt()) {
               class op_unary_astnode* change_type = new class op_unary_astnode("TO_FLOAT", $3);
               types = new class types_info("float");
               change_type -> set_type_info(types);
               $$ = new class op_binary_astnode("GE_OP_FLOAT", $1, change_type);
          }
          else if(first_type -> isInt() && second_type -> isFloat()) {
               class op_unary_astnode* change_type = new class op_unary_astnode("TO_FLOAT", $1);
               types = new class types_info("float");
               change_type -> set_type_info(types);
               $$ = new class op_binary_astnode("GE_OP_FLOAT", change_type, $3);
          }
          else if(first_type -> isInt() && second_type -> isInt()) {
               $$ = new class op_binary_astnode("GE_OP_INT", $1, $3);
          }
          else if(first_type -> isFloat() && second_type -> isFloat()) {
               $$ = new class op_binary_astnode("GE_OP_FLOAT", $1, $3);
          }
          else if(first_type -> isCompatiblePointer(second_type)) {
               $$ = new class op_binary_astnode("GE_OP_INT", $1, $3);
          }
          else no_error = false;
          if(no_error) {
               types = new class types_info("int");
               $$ -> set_type_info(types);
          }
          else {
               error(@$, "Invalid operands types for binary >= , \"" + first_type -> full_type() + "\" and \"" + second_type -> full_type() + "\"");
          }
     }
     ;

additive_expression:
     multiplicative_expression
     {
          $$ = $1;
     }

     | additive_expression '+' multiplicative_expression
     {
          first_type = $1 -> get_type_info();
          second_type = $3 -> get_type_info();
          bool no_error = true;
          if(first_type -> isFloat() && second_type -> isInt()) {
               class op_unary_astnode* change_type = new class op_unary_astnode("TO_FLOAT", $3);
               types = new class types_info("float");
               change_type -> set_type_info(types);
               $$ = new class op_binary_astnode("PLUS_FLOAT", $1, change_type);
               types = new class types_info("float");
          }
          else if(first_type -> isInt() && second_type -> isFloat()) {
               class op_unary_astnode* change_type = new class op_unary_astnode("TO_FLOAT", $1);
               types = new class types_info("float");
               change_type -> set_type_info(types);
               $$ = new class op_binary_astnode("PLUS_FLOAT", change_type, $3);
               types = new class types_info("float");
          }
          else if(first_type -> isInt() && second_type -> isInt()) {
               $$ = new class op_binary_astnode("PLUS_INT", $1, $3);
               types = new class types_info("int");
          }
          else if(first_type -> isFloat() && second_type -> isFloat()) {
               $$ = new class op_binary_astnode("PLUS_FLOAT", $1, $3);
               types = new class types_info("float");
          }
          else if(!(first_type -> isBaseType()) && second_type -> isInt()) {
               $$ = new class op_binary_astnode("PLUS_INT", $1, $3);
               types = new class types_info("int");
               *types = *first_type;
               types -> lval = false;
               types -> isZero = false;
          }
          else if(first_type -> isInt() && !(second_type -> isBaseType())) {
               $$ = new class op_binary_astnode("PLUS_INT", $1, $3);
               types = new class types_info("int");
               *types = *second_type;
               types -> lval = false;
               types -> isZero = false;
          }
          else no_error = false;
          if(no_error) {
               $$ -> set_type_info(types);
          }
          else {
               error(@$, "Invalid operands types for binary + , \"" + first_type -> full_type() + "\" and \"" + second_type -> full_type() + "\"");
          }
     }

     | additive_expression '-' multiplicative_expression
     {
          first_type = $1 -> get_type_info();
          second_type = $3 -> get_type_info();
          bool no_error = true;
          if(first_type -> isFloat() && second_type -> isInt()) {
               class op_unary_astnode* change_type = new class op_unary_astnode("TO_FLOAT", $3);
               types = new class types_info("float");
               change_type -> set_type_info(types);
               $$ = new class op_binary_astnode("MINUS_FLOAT", $1, change_type);
               types = new class types_info("float");
          }
          else if(first_type -> isInt() && second_type -> isFloat()) {
               class op_unary_astnode* change_type = new class op_unary_astnode("TO_FLOAT", $1);
               types = new class types_info("float");
               change_type -> set_type_info(types);
               $$ = new class op_binary_astnode("MINUS_FLOAT", change_type, $3);
               types = new class types_info("float");
          }
          else if(first_type -> isInt() && second_type -> isInt()) {
               $$ = new class op_binary_astnode("MINUS_INT", $1, $3);
               types = new class types_info("int");
          }
          else if(first_type -> isFloat() && second_type -> isFloat()) {
               $$ = new class op_binary_astnode("MINUS_FLOAT", $1, $3);
               types = new class types_info("float");
          }
          else if(!(first_type -> isBaseType()) && second_type -> isInt()) {
               $$ = new class op_binary_astnode("MINUS_INT", $1, $3);
               types = new class types_info("int");
               *types = *first_type;
               types -> lval = false;
               types -> isZero = false;
          }
          else if(first_type -> isInt() && !(second_type -> isBaseType())) {
               $$ = new class op_binary_astnode("MINUS_INT", $1, $3);
               types = new class types_info("int");
               *types = *second_type;
               types -> lval = false;
               types -> isZero = false;
          }
          else if(first_type -> isCompatiblePointer(second_type)) {
               $$ = new class op_binary_astnode("MINUS_INT", $1, $3);
               types = new class types_info("int");
          }
          else no_error = false;
          if(no_error) {
               $$ -> set_type_info(types);
          }
          else {
               error(@$, "Invalid operands types for binary - , \"" + first_type -> full_type() + "\" and \"" + second_type -> full_type() + "\"");
          }
     }
     ;

unary_expression:
     postfix_expression
     {
          $$ = $1;
     }

     | unary_operator unary_expression
     {
          first_type = $2 -> get_type_info();
          if($1 == "UMINUS") {
               if(first_type -> isInt()) {
                    $$ = new class op_unary_astnode($1, $2); 
                    types = new class types_info("int");
                    $$ -> set_type_info(types);                   
               }
               else if(first_type -> isFloat()) {
                    $$ = new class op_unary_astnode($1, $2); 
                    types = new class types_info("float");
                    $$ -> set_type_info(types); 
               }
               else {
                    error(@$, "Operand of unary - should be an int or float");
               }
          }
          else if($1 == "NOT") {
               if(first_type -> isInt() || first_type -> isFloat() || !(first_type -> isBaseType())) {
                    $$ = new class op_unary_astnode($1, $2);
                    types = new class types_info("int");
                    $$ -> set_type_info(types); 
               }
               else {
                    error(@$, "Operand of NOT should be an int or float or pointer");
               }
          }
          else if($1 == "ADDRESS") {
               if(first_type -> lval) {
                    $$ = new class op_unary_astnode($1, $2);
                    types = new class types_info("int");
                    *types = *first_type;
                    types -> arr_pointers ++;
                    types -> tot_pointers ++;
                    types -> lval = false;
                    types -> isZero = false;
                    $$ -> set_type_info(types);
               }
               else {
                    error(@$, "Operand of & should have lvalue");
               }
          }
          else if($1 == "DEREF") {
               if(first_type -> isCompletePointer()) {
                    $$ = new class op_unary_astnode($1, $2);
                    types = new class types_info("int");
                    *types = *first_type;
                    if(types -> arr_size > 0) {
                         types -> arr_const.pop();
                         types -> arr_size --;
                    }
                    else {
                         types -> num_pointers --;
                         types -> tot_pointers --;
                    }
                    types -> lval = true;
                    types -> isZero = false;
                    $$ -> set_type_info(types);
               }
               else {
                    error(@$, "Invalid operand type \"" + first_type -> full_type() + "\" of unary *");
               }
          }
     }    
     ;

multiplicative_expression:
     unary_expression
     {    
          $$ = $1;
     }

     | multiplicative_expression '*' unary_expression
     {
          first_type = $1 -> get_type_info();
          second_type = $3 -> get_type_info();
          bool no_error = true;
          if(first_type -> isFloat() && second_type -> isInt()) {
               class op_unary_astnode* change_type = new class op_unary_astnode("TO_FLOAT", $3);
               types = new class types_info("float");
               change_type -> set_type_info(types);
               $$ = new class op_binary_astnode("MULT_FLOAT", $1, change_type);
               types = new class types_info("float");
          }
          else if(first_type -> isInt() && second_type -> isFloat()) {
               class op_unary_astnode* change_type = new class op_unary_astnode("TO_FLOAT", $1);
               types = new class types_info("float");
               change_type -> set_type_info(types);
               $$ = new class op_binary_astnode("MULT_FLOAT", change_type, $3);
               types = new class types_info("float");
          }
          else if(first_type -> isInt() && second_type -> isInt()) {
               $$ = new class op_binary_astnode("MULT_INT", $1, $3);
               types = new class types_info("int");
          }
          else if(first_type -> isFloat() && second_type -> isFloat()) {
               $$ = new class op_binary_astnode("MULT_FLOAT", $1, $3);
               types = new class types_info("float");
          }
          else no_error = false;
          if(no_error) {
               $$ -> set_type_info(types);
          }
          else {
               error(@$, "Invalid operands types for binary * , \"" + first_type -> full_type() + "\" and \"" + second_type -> full_type() + "\"");
          }
     }

     | multiplicative_expression '/' unary_expression
     {
          first_type = $1 -> get_type_info();
          second_type = $3 -> get_type_info();
          bool no_error = true;
          if(first_type -> isFloat() && second_type -> isInt()) {
               class op_unary_astnode* change_type = new class op_unary_astnode("TO_FLOAT", $3);
               types = new class types_info("float");
               change_type -> set_type_info(types);
               $$ = new class op_binary_astnode("DIV_FLOAT", $1, change_type);
               types = new class types_info("float");
          }
          else if(first_type -> isInt() && second_type -> isFloat()) {
               class op_unary_astnode* change_type = new class op_unary_astnode("TO_FLOAT", $1);
               types = new class types_info("float");
               change_type -> set_type_info(types);
               $$ = new class op_binary_astnode("DIV_FLOAT", change_type, $3);
               types = new class types_info("float");
          }
          else if(first_type -> isInt() && second_type -> isInt()) {
               $$ = new class op_binary_astnode("DIV_INT", $1, $3);
               types = new class types_info("int");
          }
          else if(first_type -> isFloat() && second_type -> isFloat()) {
               $$ = new class op_binary_astnode("DIV_FLOAT", $1, $3);
               types = new class types_info("float");
          }
          else no_error = false;
          if(no_error) {
               $$ -> set_type_info(types);
          }
          else {
               error(@$, "Invalid operands types for binary / , \"" + first_type -> full_type() + "\" and \"" + second_type -> full_type() + "\"");
          }
     }
     ;

postfix_expression:
     primary_expression
     {
          $$ = $1;
     }
                    
     | postfix_expression '[' expression ']'
     {
          first_type = $1 -> get_type_info();
          second_type = $3 -> get_type_info();
          if(!(first_type -> isCompletePointer())) {
               error(@$, "Subscripted value is neither array nor pointer");
          }
          else if(!(second_type -> isInt())) {
               error(@$, "Array subscript is not an integer");
          }
          else {
               $$ = new class arrayref_astnode($1, $3);
               types = new class types_info("int");
               *types = *first_type;
               if(types -> arr_size > 0) {
                    types -> arr_const.pop();
                    types -> arr_size --;
               }
               else {
                    types -> num_pointers --;
                    types -> tot_pointers --;
               }
               types -> lval = true;
               types -> isZero = false;
               $$ -> set_type_info(types);
          }
     }
            
     | IDENTIFIER '(' ')'
     {
          temp_st = gst -> get_elems();
          if(temp_st.find($1) == temp_st.end() && predefined.find($1) == predefined.end() && func_name != $1) {
               error(@$, "Procedure \"" + $1 + "\" not declared");
          }
          else if(temp_st.find($1) != temp_st.end() && temp_st[$1] -> get_type() != "fun") {
               error(@$, "Called object \"" + $1 + "\" is not a procedure");
          }
          if(temp_st.find($1) != temp_st.end()) {
               temp_param_list = param_type_map[$1];
               types = new class types_info(temp_st[$1] -> get_returntype());
          }
          else if(func_name == $1) {
               temp_param_list = param_types;
               types = new class types_info(return_type);
          }
          else {
               types = new class types_info(predefined[$1]);
          }
          if(predefined.find($1) == predefined.end()) {
               if(temp_param_list.size() > 0) {
                    error(@$, "\"" + $1 + "\" called with too few arguments");
               }
          }
          class identifier_astnode* iden = new class identifier_astnode($1);
          vector<class exp_astnode*> vec;
          vec.push_back(iden);
          $$ = new class funcall_astnode(vec);
          $$ -> set_type_info(types);
     }
                      
     | IDENTIFIER '(' expression_list ')'
     {
          temp_st = gst -> get_elems();
          if(temp_st.find($1) == temp_st.end() && predefined.find($1) == predefined.end() && func_name != $1) {
               error(@$, "Procedure \"" + $1 + "\" not declared");
          }
          else if(temp_st.find($1) != temp_st.end() && temp_st[$1] -> get_type() != "fun") {
               error(@$, "Called object \"" + $1 + "\" is not a procedure");
          }
          if(temp_st.find($1) != temp_st.end()) {
               temp_param_list = param_type_map[$1];
          }
          else if(func_name == $1) {
               temp_param_list = param_types;
          }
          class identifier_astnode* iden = new class identifier_astnode($1);
          vector<class exp_astnode*> vec;
          if(temp_st.find($1) != temp_st.end() || func_name == $1) {
               if($3.size() < temp_param_list.size()) {
                    error(@$, "\"" + $1 + "\" called with too few arguments");
               }
               else if($3.size() > temp_param_list.size()) {
                    error(@$, "\"" + $1 + "\" called with too many arguments");
               }
               vec.push_back(iden);
               for(class exp_astnode* exp: $3) {
                    first_type = temp_param_list.front();
                    second_type = exp -> get_type_info();
                    bool assignable = first_type -> isCompatiblePointer(second_type) || first_type -> sameBaseType(second_type);
                    assignable |= !(first_type -> isBaseType()) && second_type -> isVoidPointer();
                    assignable |= first_type -> isVoidPointer() && !(second_type -> isBaseType());
                    assignable |= !(first_type -> isBaseType()) && second_type -> isZero;
                    temp_param_list.pop();
                    if(assignable) {
                         if((first_type -> isInt()) && (second_type -> isFloat())) {
                              class op_unary_astnode* change_type = new class op_unary_astnode("TO_INT", exp);
                              types = new class types_info("int");
                              change_type -> set_type_info(types);
                              vec.push_back(change_type);
                         }
                         else if((first_type -> isFloat()) && (second_type -> isInt())) {
                              class op_unary_astnode* change_type = new class op_unary_astnode("TO_FLOAT", exp);
                              types = new class types_info("float");
                              change_type -> set_type_info(types);
                              vec.push_back(change_type);
                         }
                         else vec.push_back(exp);
                    }
                    else {
                         error(@$, "Expected \"" + first_type -> full_type() + "\" but argument is of type \"" + second_type -> full_type() + "\"");
                    }
               }
               if(temp_st.find($1) != temp_st.end()) {
                    types = new class types_info(temp_st[$1] -> get_returntype());
               }
               else if(func_name == $1) {
                    types = new class types_info(return_type);
               }
          }
          else {
               vec.push_back(iden);
               for(class exp_astnode* exp: $3) {
                    vec.push_back(exp);
               }
               types = new class types_info(predefined[$1]);
          }
          $$ = new class funcall_astnode(vec);
          $$ -> set_type_info(types);
     }
                      
     | postfix_expression '.' IDENTIFIER
     {
          first_type = $1 -> get_type_info();
          if(first_type -> isStruct()) {
               map<std::string, class Symbols*> struct_symtab = symtab_map[first_type -> basetype] -> get_elems();
               if(struct_symtab.find($3) != struct_symtab.end()) {
                    class identifier_astnode* iden = new class identifier_astnode($3);
                    $$ = new class member_astnode($1, iden);
                    types = new class types_info("int");
                    *types = *(struct_symtab[$3] -> get_type_info());
                    types -> lval = true;
                    $$ -> set_type_info(types);
               }
               else {
                    error(@$, "Struct \"" + first_type -> basetype + "\" has no member named \"" + $3 + "\"");
               }
          }
          else {
               error(@$, "Left operand of \".\"  is not a structure");
          }
     }
                      
     | postfix_expression PTR_OP IDENTIFIER
     {
          first_type = $1 -> get_type_info();
          if(first_type -> isStructPointer()) {
               map<std::string, class Symbols*> struct_symtab = symtab_map[first_type -> basetype] -> get_elems();
               if(struct_symtab.find($3) != struct_symtab.end()) {
                    class identifier_astnode* iden = new class identifier_astnode($3);
                    $$ = new class arrow_astnode($1, iden);
                    types = new class types_info("int");
                    *types = *(struct_symtab[$3] -> get_type_info());
                    types -> lval = true;
                    $$ -> set_type_info(types);
               }
               else {
                    error(@$, "\"" + first_type -> basetype + "\" has no member named \"" + $3 + "\"");
               }
          }
          else {
               error(@$, "Left operand of \"->\"  is not a pointer to a structure");
          }
     }
                      
     | postfix_expression INC_OP
     {
          first_type = $1 -> get_type_info();
          if(first_type -> lval) {
               if(first_type -> isInt() || first_type -> isFloat() || first_type -> isBasePointer()) {
                    $$ = new class op_unary_astnode("PP", $1);
                    types = new class types_info("int");
                    *types = *first_type;
                    types -> lval = false;
                    types -> isZero = false;
                    $$ -> set_type_info(types);
               }
               else {
                    error(@$, "Operand of \"++\" should be a int, float or pointer");
               }
          }
          else {
               error(@$, "Operand of \"++\" should have lvalue");
          }
     }
     ;

primary_expression: 
     IDENTIFIER
     {
          temp_st = lst -> get_elems();
          if(temp_st.find($1) != temp_st.end()) {
               $$ = new class identifier_astnode($1);
               types = new class types_info("int");
               *types = *(temp_st[$1] -> get_type_info());
               types -> lval = true;
               $$ -> set_type_info(types);
          }
          else {
               error(@$, "Variable \"" + $1 + "\" not declared");
          }
     }
                      
     | INT_CONSTANT
     {
          $$ = new class intconst_astnode(stoi($1));
          types = new class types_info("int");
          if(stoi($1) == 0) types -> isZero = true;
          $$ -> set_type_info(types);
     }
     
     | FLOAT_CONSTANT
     {
          $$ = new class floatconst_astnode(stof($1));
          types = new class types_info("float");
          $$ -> set_type_info(types);
     }

     | STRING_LITERAL
     {
          $$ = new class string_astnode($1);
          types = new class types_info("string");
          $$ -> set_type_info(types);
     }
                      
     | '(' expression ')'
     {
          $$ = $2;
     }
     ;

expression_list:
     expression
     {
          vector<class exp_astnode*> vec;
          vec.push_back($1);
          $$ = vec;
     }
     
     | expression_list ',' expression
     {
          $$ = $1;
          $$.push_back($3);
     }
     ;

unary_operator: 
     '-'
     {
          $$ = "UMINUS";
     }
     
     | '!'
     {
          $$ = "NOT";
     }
     
     | '&'
     {
          $$ = "ADDRESS";
     }
     
     | '*'
     {
          $$ = "DEREF";
     }
     ;

selection_statement: 
     IF '(' expression ')' statement ELSE statement
     {
          $$ = new class if_astnode($3, $5, $7);
     }
     ;

iteration_statement:
     WHILE '(' expression ')' statement
     {
          $$ = new class while_astnode($3, $5);
     }
                       
     | FOR '(' assignment_expression ';' expression ';' assignment_expression ')' statement
     {
          $$ = new class for_astnode($3, $5, $7, $9);
     }
     ;

declaration_list:   
     declaration
     {
          
     }
     
     | declaration_list declaration
     {
          
     }
     ;

declaration:
     type_specifier declarator_list ';'
     {
          
     }
     ;

declarator_list:
     declarator
     {
          if(base_type == "void" && num_pointers == 0) {
               error(@$, "Cannot declare variable of type \"void\"");
          }
          else {
               if(num_pointers > 0) var_size = 4;
               else if(base_type == "int" || base_type == "float") var_size = 4;
               else {
                    temp_st = gst -> get_elems();
                    if(temp_st.find(base_type) == temp_st.end()) {
                         error(@$, "\"" + base_type + "\" is not defined");
                    }
                    var_size = temp_st[base_type] -> get_size();
               }
               var_size *= $1;
               if(func_name != "") {
                    prev_var_offset -= var_size;
               }
               types = new class types_info(base_type, num_pointers, 0, arr_const);
               derived_type = types -> full_type(true);
               class Symbols* varSym = new class Symbols(var_name, "var", "local", var_size, prev_var_offset, derived_type, types);
               num_pointers = 0;
               arr_const = queue<int>();
               if(func_name == "") {
                    prev_var_offset -= var_size;
               }
               lst -> addSymbol(varSym);
          }
     }

     | declarator_list ',' declarator
     {
          if(base_type == "void" && num_pointers == 0) {
               error(@$, "Cannot declare variable of type \"void\"");
          }
          else {
               if(num_pointers > 0) var_size = 4;
               else if(base_type == "int" || base_type == "float") var_size = 4;
               else {
                    temp_st = gst -> get_elems();
                    if(temp_st.find(base_type) == temp_st.end()) {
                         error(@$, "\"" + base_type + "\" is not defined");
                    }
                    var_size = temp_st[base_type] -> get_size();
               }
               var_size *= $3;
               if(func_name != "") {
                    prev_var_offset -= var_size;
               }
               types = new class types_info(base_type, num_pointers, 0, arr_const);
               derived_type = types -> full_type(true);
               class Symbols* varSym = new class Symbols(var_name, "var", "local", var_size, prev_var_offset, derived_type, types);
               num_pointers = 0;
               arr_const = queue<int>();
               if(func_name == "") {
                    prev_var_offset -= var_size;
               }
               lst -> addSymbol(varSym);
          }
     }
     ;

%%
void IPL::Parser::error( const location_type &l, const std::string &err_message )
{
   std::cout << "Error at line " << line_no << ": " << err_message << "\n";
   exit(1);
}


