//
// Created by 谢威宇 on 2020/4/28.
//

#include <iostream>
#include <string>
#include <cstdlib>

using namespace std;


int main(int argc, char *argv[]) {
    string in_dir;
    string out_dir;
    for (int i = 0; i < argc; i++) {
        if (string(argv[i]) == "-o") {
            out_dir = argv[i + 1];
            i++;
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


    cout << in_dir << endl;
    cout << "Calling Scanner" << endl;
    if (system(("./scanner " + in_dir).data()) != 0) {
        return 1;
    }
    cout << "Calling Parser" << endl;
    if (system(("./parser " + name + ".tokens").data()) != 0) {
        return 1;
    }
    return 0;
}