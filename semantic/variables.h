//
// Created by 谢威宇 on 2020/4/29.
//

#ifndef ERVINCC_VARIABLES_H
#define ERVINCC_VARIABLES_H

#include "types.h"
#include <set>

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
};

class func {
public:
    rtype r;
    string name;
    bool defined;
    func(const rtype &r, const string &name);

    vector<pair<rtype, string>> parameters;
};

class functions {
public:
    map<string, func *> m;

    func *get(string x);



    fp new_func(rtype r, const string &name);
};


#endif //ERVINCC_VARIABLES_H
