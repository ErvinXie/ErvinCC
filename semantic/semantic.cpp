//
// Created by 谢威宇 on 2020/4/28.
//

#include "semantic.h"
#import <fstream>

int main() {
    cnp x = new cst_node("test");
    auto p = from_string("/Users/ervinxie/ClionProjects/ErvinCC/test-files/test-file1.json", true);
    ofstream out("/Users/ervinxie/ClionProjects/ErvinCC/test-files/test-file2.json");
    out << p->to_string(true);

    return 0;
}
