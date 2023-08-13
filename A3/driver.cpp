
#include "scanner.hh"
#include "parser.tab.hh"
#include <fstream>
#include <vector>
#include <map>
using namespace std;

extern map<string, vector<string> > gen_code;
extern map<string, vector<string> > rodata;

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

for (pair<string, vector<string> > v: gen_code)
{
	vector<string> read_only = rodata[v.first];
	if(read_only.size() > 0) {
		cout << "\t.section\t.rodata\n";
	}
	for(string p: read_only) {
		cout << p << endl;
	}
	cout << "\t.text\n";
	for(string s: v.second) {
		cout << s << endl;
	}
}

	fclose(stdout);
}