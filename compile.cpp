//
// Created by 谢威宇 on 2020/4/28.
//

#include <iostream>
#include <string>
#include <cstdlib>
#include <fstream>
#include <regex>

using namespace std;


int main(int argc, char *argv[]) {
    string in_dir;
    bool astpic = false;
    for (int i = 0; i < argc; i++) {
        if (string(argv[i]) == "-p") {
            astpic = true;
        } else {
            in_dir = argv[i];
        }
    }
    string name;
    for (int i = (int) in_dir.length() - 1; i >= 0; i--) {
        if (in_dir[i] == '.') {
            name = in_dir.substr(0, i);
            break;
        }
    }
    string out_dir = name + ".pp";
    cout << in_dir << endl;
    cout << out_dir << endl;
    ifstream in(in_dir);
    ofstream out(out_dir);
    char t[10000] = {};
    in.read(t, 10000);
    string x(t);
    regex r(R"((//[^\n]*)|(/\*.*\*/))");
    out << regex_replace(x, r, "") << endl;

    cout << "Calling Scanner" << endl;
    if (system(("./scanner " + out_dir).data()) != 0) {
        return 1;
    }
    cout << "Calling Parser" << endl;
    if (system(("./parser " + name + ".tokens" + (astpic ? " -p" : "")).data()) != 0) {
        return 1;
    }
    cout << "Calling Semantic Checker" << endl;
    if (system(("./semantic " + name + ".json ").data()) != 0) {
        return 1;
    }
    cout << "Calling Code Generator" << endl;
    if (system(("./codegen " + name + ".ll ").data()) != 0) {
        return 1;
    }
    return 0;
}