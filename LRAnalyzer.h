//
// Created by 谢威宇 on 2020/4/13.
//

#ifndef ERVINCC_LRANALYZER_H
#define ERVINCC_LRANALYZER_H

#include <string>
#include <map>
#include <vector>
#include <functional>
#include "cst_node.h"

using namespace std;

class LRAnalyzer {
public:
    vector<int> state_st;
    vector<cnp> node_st;
    map<pair<int, string>, function<int(cnp)> > action;
    map<pair<int, string>, int> go_to;

public:
    void read_lr1_form(string rule_dir);

    int parse(vector<string> type, vector<string> token);

    void debug();

};


#endif //ERVINCC_LRANALYZER_H
