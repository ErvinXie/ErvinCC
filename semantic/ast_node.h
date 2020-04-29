//
// Created by 谢威宇 on 2020/4/29.
//

#ifndef ERVINCC_AST_NODE_H
#define ERVINCC_AST_NODE_H

#include <vector>
#include <string>

using namespace std;

class ast_node;

typedef ast_node *anp;

class ast_node {
public:
    string name;

};

class translation_unit : public ast_node {
public:
    vector<anp> list;
};

class function_definition : public ast_node {
public:

};


#endif //ERVINCC_AST_NODE_H
