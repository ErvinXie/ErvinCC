//
// Created by 谢威宇 on 2020/3/30.
//

#ifndef ERVINCC_GRAMMER_H
#define ERVINCC_GRAMMER_H


#include <string>
#include <vector>
#include <iostream>
#include <set>
#include <stack>
#include "utils.h"

using namespace std;

class grammer_node;

typedef grammer_node *nodep;

class grammer;

typedef grammer *grammerp;

// LR(0) 的项目
class item;

typedef item *itemp;

class closure;

typedef closure *closurep;


class grammer_node {
private:
    set<nodep> first;
    vector<set<nodep>> first_by_rule;
public:
    grammer_node(string name, class grammer *g);

    ~grammer_node();

    string name;
    vector<vector<nodep> > rules;
    grammer *grammer;

    void add(vector<string> x);

    void substitute(nodep v);

    void substitute_first(nodep v);

    string to_string();

    string to_text();


    set<nodep> get_first();


    vector<set<nodep>> get_first_by_rule();

    bool de_epsilon();//能否推导出epsilon


    //LR(0)
    map<pair<vector<nodep>, int>, itemp> itemps;

    itemp get_item(vector<nodep> rule, int pos);

    set<itemp> get_items(function<bool(itemp)> filter= nullptr);

};


class grammer {
public:
    grammer() = default;

    grammer(const grammer &g);

    ~grammer();

    vector<nodep> nodes;//nodes sequenced by the appearance order of left node.
    map<string, nodep> all_vertices;

    nodep next_node(string s);


    void read_rules();

    void print_rules(bool tex = false);

    vector<nodep> get_all_Vn();

    vector<nodep> get_all_Vt();

    set<nodep> all_vt;

    bool isvt(nodep x);

    vector<nodep> get_root();


    /*------LL(1)-------*/
    grammer *eliminate_left_recursion();


    map<nodep, set<nodep>> first;

    map<nodep, set<nodep>> get_first();

    void print_first();

    map<nodep, set<nodep>> follow;

    map<nodep, set<nodep>> get_follow();

    void print_follow();

    map<pair<nodep, nodep>, vector<vector<nodep>>> LL1form;

    map<pair<nodep, nodep>, vector<vector<nodep>>> get_LL1form();

    void print_LL1form();

    /*------LR(0)-------*/

    map<set<itemp>, closurep> closures;
    closurep get_closure(set<itemp> items);

    closurep head_closure= nullptr;
    void build_closures();


    void print_closures();

};


class item {
public:
    item(nodep left, vector<nodep> rule, int dot_position);

    nodep left;
    vector<nodep> rule;
    int dot_position;


    set<itemp> closure_items;

    set<itemp> get_closure_items();

    string to_string(string prefix = "");


};

class closure {
public:
    closure(set<itemp> s);
    int mycnt;
    bool expanded = false;
    set<itemp> items;
    map<nodep, closurep> go;

    void expand();
    string to_string();

private:
    static int cnt;
};



#endif //ERVINCC_GRAMMER_H
