//
// Created by 谢威宇 on 2020/4/29.
//

#include "variables.h"

variable::variable(rtype r, const string &name, int scopeLevel) : r(r), name(name), scope_level(scopeLevel) {}

variable *variables::get(string x) {
    return m[x].back();
}

vp variables::new_var(rtype r, const string &name, int scopeLevel) {
    if (m.count(name)) {
        if (!m[name].empty() && m[name].back()->scope_level == scopeLevel) {
            throw duplicate_definition();
        }
    }
    vp re = new variable(r, name, scopeLevel);
    m[name].push_back(re);
    return re;
}

func *functions::get(string x) {
    return m[x];
}

fp functions::new_func(rtype r, const string &name) {
    if (m.count(name)) {
        return m[name];
    } else {
        m[name] = new func(r, name);
        return m[name];
    }
}

func::func(const rtype &r, const string &name) : r(r), name(name) { defined = false; }
