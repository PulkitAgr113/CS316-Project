
#include "scanner.hh"
#include "parser.tab.hh"
#include <fstream>
#include <map>
#include <set>
using namespace std;

extern map<string, class ST*> symtab_map;
extern class ST *gst;
set<string> gstfun;
set<string> gststruct; 
string filename;
extern map<string, class abstract_astnode*> ast_map;
int main(int argc, char **argv)
{
	using namespace std;
	fstream in_file, out_file;
	

	in_file.open(argv[1], ios::in);

	IPL::Scanner scanner(in_file);

	IPL::Parser parser(scanner);

#ifdef YYDEBUG
	parser.set_debug_level(1);
#endif
parser.parse();
// create gstfun and gststruct

for (pair<string, class Symbols*> s: gst -> get_elems())
{
	string type = (s.second) -> get_type();
	if(type == "fun") gstfun.insert(s.first);
	else if(type == "struct") gststruct.insert(s.first);
}

// start the JSON printing

cout << "{\"globalST\": " << endl;
gst -> printst();
cout << "," << endl;

cout << "  \"structs\": [" << endl;
int num_structs = gststruct.size();
int ind = 1;

for (string s: gststruct)

{   cout << "{" << endl;
	cout << "\"name\": " << "\"" << s << "\"," << endl;
	cout << "\"localST\": " << endl;
	symtab_map[s] -> printst();
	cout << "}" << endl;
	if(ind != num_structs) cout << ",\n";
	ind ++;
}
cout << "]," << endl;
cout << "  \"functions\": [" << endl;
int num_funs = gstfun.size();
ind = 1;

for (string s: gstfun)

{
	cout << "{" << endl;
	cout << "\"name\": " << "\"" << s << "\"," << endl;
	cout << "\"localST\": " << endl;
	symtab_map[s] -> printst();
	cout << "," << endl;
	cout << "\"ast\": " << endl;
	ast_map[s] -> print();
	cout << "}" << endl;
	if(ind != num_funs) cout << ",\n";
	ind ++;
}
cout << "]" << endl;
cout << "}" << endl;

	fclose(stdout);
}