//
// Created by 谢威宇 on 2020/4/29.
//

#ifndef ERVINCC_VARIABLES_H
#define ERVINCC_VARIABLES_H


#include <set>
#include <string>
#include <vector>
#include <map>
#include <set>
#include "exceptions.h"

using namespace std;

class type;

class rtype {
public:
    type *f;
    int pointer;
    set<string> qualifiers;
    vector<int> array_size;

    rtype();

    rtype(type *t, set<string> qua = set<string>({}), int pcnt = 0);

    rtype get_add();
    rtype get_tar();
    string debug();

    bool congruent(const rtype& b);
};


class type {
public:
    string name;
    string ttype;
    bool defined;

    explicit type(string t, const string &x);

    rtype get_sub(string name);

    vector<pair<rtype, string>> fields;

    string debug();

};

class types {
public:
    types();

    map<string, type *> m;

    type *new_type(string t, string x = "");

    type *get(string x);

    string debug();

};

class variable;

typedef variable *vp;

class func;

typedef func *fp;

class variable {
public:
    rtype r;

    string name;
    int scope_level;

    variable(rtype r, const string &name, int scopeLevel);

    string debug();

};


class variables {
public:
    map<string, vector<variable *>> m;

    variable *get(string x);

    vp new_var(rtype r, const string &name, int scopeLevel);

    string debug();

    bool comparabel(rtype t1, rtype t2);
};

class func {
public:
    rtype r;
    string name;
    bool defined;
    bool returned;

    func(const rtype &r, const string &name);

    vector<pair<rtype, string>> parameters;

    set<string> labels;

    void new_label(string name);

    string debug();
};

class functions {
public:
    map<string, func *> m;

    func *get(string x);

    fp new_func(rtype r, const string &name);

    string debug();
};


#endif //ERVINCC_VARIABLES_H
