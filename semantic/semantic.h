//
// Created by 谢威宇 on 2020/4/28.
//

#ifndef ERVINCC_SEMANTIC_H
#define ERVINCC_SEMANTIC_H

#include "../parser/parser/cst_node.h"
#include "variables.h"

#include "exceptions.h"
#include <iostream>

class semantic {
public:
    semantic();

    types type_table;
    variables var_table;
    functions func_table;
    cnp head;

    void semantic_check(cnp now);

    void debug();

    string get_qualifier(cnp now);

    pair<type *, set<string> > get_type_qualifiers(cnp declaration_specifiers);

    type *get_type_from_specifier(cnp now);

    rtype get_rtype_of_declarator(type *ft, set<string> quas, cnp declarator);

    string get_declarator_name(cnp declarator);

    void push_parameters(cnp func, fp f);

    rtype get_rtype_of_expression(cnp now);

    void level_up();

    void level_down();

    fp nowfunc = nullptr;
    int iteration_cnt;
    int token_cnt;
    int scope_level;
};


#endif //ERVINCC_SEMANTIC_H
