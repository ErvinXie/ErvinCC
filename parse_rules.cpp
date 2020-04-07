//
// Created by 谢威宇 on 2020/3/19.
//

#include <iostream>
#include <vector>
#include <string>
#include <cstdio>
#include <algorithm>
#include <map>
#include <set>
#include <bitset>

#include "grammer.h"

using namespace std;

grammer *x = new grammer();


int main() {
    freopen("../input.txt", "r", stdin);
    freopen("../output.txt", "w", stdout);
    x->read_rules();
    x->print_rules();

//    cout << "Eliminate Left Recursion" << endl;
//    x = x->eliminate_left_recursion();
//    x->print_rules();
//    x->print_first();
//    x->print_follow();
//    x->print_LL1form();
    x->build_closures();
    x->print_closures();

    return 0;
}