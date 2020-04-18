//
// Created by 谢威宇 on 2020/4/14.
//

#ifndef ERVINCC_CST_NODE_H
#define ERVINCC_CST_NODE_H

#include <string>
#include <map>
#include <vector>

using namespace std;


class cst_node;

typedef cst_node *cnp;

const int INT = 0;
const int CHA = 1;
const int FLT = 2;
const int STR = 3;
const int NOD = 4;
const int NOA = 5;

class cst_value {
public:

    int type;

    int i;
    char c;
    double f;
    string s;
    cnp node;
    vector<cnp> nodes;

    cst_value(int i);

    cst_value(char c);

    cst_value(double f);

    cst_value(string s);

    cst_value(cst_node *node);

    cst_value(vector<cnp> nodes);

    string to_string();
};


class cst_node {
public:
    explicit cst_node(string type, string token = "");
    static int cst_node_cnt;
    int cnt;

    string type;
    string token;
    vector<cnp> sons;
    vector<string> sonnames;


    void to_ast();

    string to_dot(string outdir);

    void eliminate();

    void expand_list();

    bool is_vt();

    bool is_list();

    string to_string(bool recursive = false);
};


#endif //ERVINCC_CST_NODE_H
