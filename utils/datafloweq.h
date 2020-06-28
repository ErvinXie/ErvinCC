//
// Created by 谢威宇 on 2020/6/27.
//

#ifndef ERVINCC_DATAFLOWEQ_H
#define ERVINCC_DATAFLOWEQ_H

#include <string>
#include <vector>
#include <set>
#include <fstream>
#include <map>
#include "utils.h"

using namespace std;

class var;

class codeline;

class codeblock;

class var {
public:
    string name;
    set<codeline *> def, use;

    var(const string &name);

    void debug();
};

class codeline {
public:
    int id;
    vector<var *> def;
    vector<var *> use;
    codeblock *b;

    void addvar(var *, bool isuse);

    void debug();

    codeline(int id);
};

class codeblock {
public:
    int id;
    vector<codeline *> lines;

    vector<codeblock *> from;
    vector<codeblock *> to;

    void debug();

    codeblock(int id);


};

class code {
public:
    string dir;
    string dirh;
    ifstream fin;

    code(const string &dir);

    map<string, var *> vars;

    map<int, codeblock *> blocks;

    map<int, codeline *> lines;

    var *getvar(string name);

    codeblock *getblock(int id);

    codeline *getline(int id);

    void readfromfile();

    void build_block();

    int lineid;

    codeline *build_line(codeblock *block, string content);

    map<codeblock *, set<codeline *>> gen, kill, in, out;

    map<var *, map<codeblock *, set<codeline *>> > defl, usel, inl, outl;

    void compute_in_out();

    void computeL();

    void show_form();

    void show_formL();



    void debug();


};


#endif //ERVINCC_DATAFLOWEQ_H
