//
// Created by 谢威宇 on 2020/3/30.
//

#ifndef ERVINCC_GRAMMAR_H
#define ERVINCC_GRAMMAR_H


#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <set>
#include <stack>
#include "../../utils/utils.h"

using namespace std;

class grammar_node;

typedef grammar_node *nodep;

class grammar;

typedef grammar *grammerp;

// LR(0) 的项目
class lr0_item;

typedef lr0_item *lr0_itemp;

class lr0_closure;

typedef lr0_closure *lr0_closurep;

//LR(1)项目
class lr1_item;

typedef lr1_item *lr1_itemp;

class lr1_closure;

typedef lr1_closure *lr1_closurep;


class grammar_node {
private:
    set<nodep> first;
    vector<set<nodep>> first_by_rule;
public:
    grammar_node(string name, class grammar *g);

    ~grammar_node();

    string name;
    vector<vector<nodep> > rules;
    grammar *grammar;

    void add_rule(vector<string> x);

    void substitute(nodep v);

    void substitute_first(nodep v);

    string to_string();

    static string rule_to_string(vector<nodep> rule);

    string to_text();


    set<nodep> get_first(set<nodep> fathers = set<nodep>({}));

    set<nodep> get_follow();

    vector<set<nodep>> get_first_by_rule(set<nodep> fathers= set<nodep>({}));

    bool de_epsilon(set<nodep> fathers = set<nodep>({}));//能否推导出epsilon
    bool is_vt();


    //LR(0)
    map<pair<vector<nodep>, int>, lr0_itemp> all_lr0_items;

    lr0_itemp get_lr0_item(vector<nodep> rule, int pos);

    set<lr0_itemp> get_lr0_items(function<bool(lr0_itemp)> filter = nullptr);

    //LR(1)
    map<pair<vector<nodep>, pair<int, nodep> >, lr1_itemp> all_lr1_items;

    lr1_itemp get_lr1_item(vector<nodep> rule, int pos, nodep search_char);


};


class grammar {
public:
    grammar() = default;

    grammar(const grammar &g);

    ~grammar();

    vector<nodep> nodes;//nodes sequenced by the appearance order of left node.
    map<string, nodep> all_nodes;

    nodep get_node(string s);


    void read_rules(istream &in);

    void print_rules(ostream &out);

    vector<nodep> get_all_Vn();

    vector<nodep> get_all_Vt();

    set<nodep> all_vt;

    bool isvt(nodep x);


    vector<nodep> get_root();


    /*------LL(1)-------*/
    grammar *eliminate_left_recursion();

    void elimiante_common_left();

    map<nodep, set<nodep>> first;

    map<nodep, set<nodep>> get_first();

    void print_first(ostream &out);

    map<nodep, set<nodep>> follow;

    map<nodep, set<nodep>> get_follow();

    void print_follow(ostream &out);

    map<pair<nodep, nodep>, vector<vector<nodep>>> LL1form;

    map<pair<nodep, nodep>, vector<vector<nodep>>> get_LL1form();

    void print_LL1form(ostream &out);

    /*------LR(0)-------*/

    map<set<lr0_itemp>, lr0_closurep> lr0_closures;

    lr0_closurep get_lr0_closure(set<lr0_itemp> items);

    lr0_closurep head_lr0_closure = nullptr;

    int lr0_closure_cnt = 0;

    void build_lr0_closures();

    void print_lr0_closures(ostream &out, ostream &dout);

    map<pair<lr0_closurep, nodep>, vector<string>> SLR1form;

    map<pair<lr0_closurep, nodep>, vector<string>> get_SLR1form();

    void print_SLR1form(ostream &out);

    /*------LR(1)-------*/

    map<set<lr1_itemp>, lr1_closurep> lr1_closures;

    lr1_closurep get_lr1_closure(set<lr1_itemp> items);

    lr1_closurep head_lr1_closure = nullptr;

    int lr1_closure_cnt = 0;

    void build_lr1_closures();

    void print_lr1_closures(ostream &out, ostream &dout);

    map<pair<lr1_closurep, nodep>, vector<string>> LR1form;

    map<pair<lr1_closurep, nodep>, vector<string>> get_LR1form();

    void print_LR1form(ostream &out);

};


class lr0_item {
    //LR(0) lr0_item
public:
    lr0_item(nodep left, vector<nodep> rule, int dot_position);

    nodep left;
    vector<nodep> rule;
    int dot_position;


    set<lr0_itemp> closure_items;

    set<lr0_itemp> expand_closure_items();

    string to_string(string prefix = "");

    enum kind {
        RDC,//reduce
        ACC,//accept
        SFT,//shift
        TRD,//to be reduced
    };

    kind get_kind();

    std::string get_kind_str();
};


class lr0_closure {
public:
    explicit lr0_closure(set<lr0_itemp> s);

    grammerp g;
    int lr0_closure_cnt;
    bool expanded = false;
    set<lr0_itemp> itemps;
    map<nodep, lr0_closurep> go;

    void expand();

    string to_string();
};

class lr1_item : public lr0_item {
public:
    lr1_item(nodep left1, vector<nodep> rule1, int dotPosition, nodep search);

    nodep search_char;

    set<lr1_itemp> closure_items;

    set<lr1_itemp> expand_closure_items();

    string to_string(string prefix = "");
};

class lr1_closure {
public:
    explicit lr1_closure(set<lr1_itemp> s);

    grammerp g;
    int lr1_closure_cnt;
    bool expanded = false;
    set<lr1_itemp> itemps;
    map<nodep, lr1_closurep> go;

    void expand();

    string to_string();

};


#endif //ERVINCC_GRAMMAR_H
