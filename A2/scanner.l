%{
// This part goes directly into the generated parser

#include "scanner.hh"
#include "parser.tab.hh"

// YY_DECL is bound to the type of the scanner. The default is 
// #define YY_DECL int Scanner::yylex()
// which is generated anyway.

//If you want the type of the generated scanner to be different, do this

#undef  YY_DECL
#define YY_DECL int IPL::Scanner::yylex( IPL::Parser::semantic_type * const lval, IPL::Parser::location_type *location )

//We want these to be the first action after a token is recognized. 

#define YY_USER_ACTION loc->step(); loc->columns(yyleng);

// token_type is a short hand for IPL::Parser::token

using token_type = IPL::Parser::token;
extern int line_no;

%}


/* %option debug */
%option nodefault
%option yyclass="Scanner"
%option noyywrap
%option c++

LETTER      [a-zA-Z]
DIGIT       [0-9]
IDEN        ("_"|{LETTER})("_"|{LETTER}|{DIGIT})*
INT         ([1-9]{DIGIT}*)|("0"[bB][01]*)|("0"[0-7]*)|("0"[xX][0-9a-fA-F]*)
EXP         ([eE][+-]{DIGIT}+)|([eE]{DIGIT}+)
FLOAT       ({DIGIT}+"."{DIGIT}*{EXP}?)|({DIGIT}*"."{DIGIT}+{EXP}?)
STRING      \"([^"\\]|"\\n"|"\\t"|"\\\\"|"\\\"")*\"

%%

%{ /** Code executed at the beginning of yylex **/

      yylval = lval;
      loc = location;

%}

[ \t]                                       //skip
[\n]                                        {loc -> lines(); line_no ++;}
"/*"([^*]|\*+[^*/])*\*+"/"                  //skip
"struct"                                    {yylval->emplace< std::string >( YYText() );
                                                 return token_type::STRUCT;}
"if"                                        {yylval->emplace< std::string >( YYText() );
                                                 return token_type::IF;}
"else"                                      {yylval->emplace< std::string >( YYText() );
                                                 return token_type::ELSE;}
"void"                                      {yylval->emplace< std::string >( YYText() );
                                                 return token_type::VOID;}
"int"                                       {yylval->emplace< std::string >( YYText() );
                                                 return token_type::INT;}
"float"                                     {yylval->emplace< std::string >( YYText() );
                                                 return token_type::FLOAT;}
"while"                                     {yylval->emplace< std::string >( YYText() );
                                                 return token_type::WHILE;}
"for"                                       {yylval->emplace< std::string >( YYText() );
                                                 return token_type::FOR;}
"return"                                    {yylval->emplace< std::string >( YYText() );
                                                 return token_type::RETURN;}
"||"                                        {yylval->emplace< std::string >( YYText() );
                                                 return token_type::OR_OP;}
"&&"                                        {yylval->emplace< std::string >( YYText() );
                                                 return token_type::AND_OP;}
"=="                                        {yylval->emplace< std::string >( YYText() );
                                                 return token_type::EQ_OP;}
"!="                                        {yylval->emplace< std::string >( YYText() );
                                                 return token_type::NE_OP;}
"<="                                        {yylval->emplace< std::string >( YYText() );
                                                 return token_type::LE_OP;}
">="                                        {yylval->emplace< std::string >( YYText() );
                                                 return token_type::GE_OP;}
"++"                                        {yylval->emplace< std::string >( YYText() );
                                                 return token_type::INC_OP;}
"->"                                        {yylval->emplace< std::string >( YYText() );
                                                 return token_type::PTR_OP;}
{IDEN}                                      {yylval->emplace< std::string >( YYText() );
                                                 return token_type::IDENTIFIER;}
{INT}                                       {yylval->emplace< std::string >( YYText() );
                                                 return token_type::INT_CONSTANT;}
{FLOAT}                                     {yylval->emplace< std::string >( YYText() );
                                                 return token_type::FLOAT_CONSTANT;}
{STRING}                                    {yylval->emplace< std::string >( YYText() );
                                                 return token_type::STRING_LITERAL;}
"!"                                         return '!';
"&"                                         return '&';
"."                                         return '.';       
"+"                                         return '+';
"-"                                         return '-';
"*"                                         return '*';
"/"                                         return '/';
"("                                         return '(';
")"                                         return ')';
","                                         return ',';
"["                                         return '[';
"]"                                         return ']';
"<"                                         return '<';
">"                                         return '>';
"="                                         return '=';
"{"                                         return '{';
"}"                                         return '}';
";"                                         return ';';
<<EOF>>                                     yyterminate();
.				                {yylval->emplace< std::string >( YYText() );
                                                 return token_type::OTHERS;}

%%

