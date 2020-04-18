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

#include "grammar.h"

using namespace std;

grammar *x = new grammar();


int main() {
    freopen("../testfiles/rules.txt", "r", stdin);
//    freopen("../testfiles/rules_nodes.txt", "w", stdout);
//    freopen("../testfiles/slr1.txt", "w", stdout);
    freopen("../testfiles/lr1.txt", "w", stdout);
    x->read_rules();
//    x->print_rules();

//    LL(1)
//    x = x->eliminate_left_recursion();
//    x->print_rules();
//    x->print_first();
//    x->print_follow();
//    x->print_LL1form();

//     SLR(1)
    x->print_lr0_closures(false);
    x->print_follow();
    x->print_SLR1form();

//    LR(1)
    x->print_lr1_closures(false);
    x->print_LR1form();
    return 0;
}