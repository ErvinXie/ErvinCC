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

    vector<cnp> all_vt();

};



cnp from_string(string str, bool file);

#endif //ERVINCC_CST_NODE_H
