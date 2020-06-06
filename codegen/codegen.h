//
// Created by 谢威宇 on 2020/6/5.
//

#ifndef ERVINCC_CODEGEN_H
#define ERVINCC_CODEGEN_H

#include  <sstream>
#include <fstream>
#include <vector>
#include <string>
#include <map>

using namespace std;


class var_type {
public:
    var_type() = default;
    var_type(string);

    int count;
    int size;
    var_type *subtype;

    string get_unit_descripter();
    int get_unit_size();

    int all_count();
};

class reg;

class codegen;

class var {
public:
    string name;
    var_type t;

    var(const string &name, codegen *c);

    codegen *c;

    reg *ad = nullptr;
    reg *va = nullptr;

    string add(ostringstream &body);

    string val(ostringstream &body);

    void unbind();

};

class reg {
public:
    string name;
    var *v = nullptr;
};


class codegen {
public:
    vector<string> ir;
    int now;
    int fcnt;
    map<string, var *> toass;
    codegen(const vector<string> &ir, string outdir);

    ofstream out;
    ostringstream global;
    ostringstream function;

    reg r[32], f[32];
    reg *a[4];

    string push(string r);
    string pop(string r);
    string idler();
    reg* idle();


    void gen();

    void genfunc();

    void callfunc(ostringstream& body);

    void genline();


};


#endif //ERVINCC_CODEGEN_H
