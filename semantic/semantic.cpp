//
// Created by 谢威宇 on 2020/4/28.
//

#include "semantic.h"
#include <fstream>
#include <set>
#include <utility>


int main() {
    cnp x = new cst_node("test");
    auto p = from_string("/Users/ervinxie/ClionProjects/ErvinCC/test-files/test-file1.json", true);
//    ofstream out("/Users/ervinxie/ClionProjects/ErvinCC/test-files/test-file2.json");
//    out << p->to_string(true);
    semantic s;
    try {
        s.semantic_check(p);
        s.debug();
    } catch (exception &e) {
        cout << e.what() << endl;
    }

    return 0;
}

set<string> type_specifiers({"void", "char", "short", "int",
                             "long", "float", "double", "signed",
                             "unsigned", "struct-or-union-specifier", "enum-specifier"});
set<string> type_qualifiers({"const", "restrict", "volatile", "extern", "static", "register"});


semantic::semantic() {
    token_cnt = 0;
    scope_level = 0;
}


void semantic::semantic_check(cnp now) {

//    cout << now->type << endl;
    if (now->type == "translation-unit") {

    } else if (now->type == "external-declaration") {

    } else if (now->type == "function-definition") {
        level_up();
        auto p = get_type_qualifiers(now->sons[0]);
        auto specifier = p.first;
        auto qualifier = p.second;
        auto rt = get_rtype(specifier, qualifier, now->sons[1]);
        auto name = get_declarator_name(now->sons[1]);
        auto newFunc = func_table.new_func(rt, name);
        if (newFunc->defined) {
            throw duplicate_definition();
        } else if (newFunc->parameters.empty()) {
            push_parameters(now, newFunc);
        }

        //todo: body
        level_down();
        return;git
    } else if (now->type == "declaration-list") {

    } else if (now->type == "declaration") {

    } else if (now->type == "declaration-specifiers") {

    } else if (now->type == "storage-class-specifier") {

    } else if (now->type == "type-specifier") {

    } else if (now->type == "type-qualifier") {

    } else if (now->type == "init-declarator-list") {

    } else if (now->type == "init-declarator") {

    } else if (now->type == "struct-or-union-specifier") {
        get_type_from_specifier(now);
        return;
    } else if (now->type == "struct-or-union") {

    } else if (now->type == "struct-declaration-list") {

    } else if (now->type == "struct-declaration") {


    } else if (now->type == "struct-declarator-list") {

    } else if (now->type == "struct-declarator") {

    } else if (now->type == "declarator") {

    } else if (now->type == "enum-specifier") {
        get_type_from_specifier(now);
        return;
    } else if (now->type == "enumerator-list") {

    } else if (now->type == "enumerator") {

    } else if (now->type == "enumeration-constant") {

    } else if (now->type == "declarator") {

    } else if (now->type == "direct-declarator") {

    } else if (now->type == "pointer") {

    } else if (now->type == "type-qualifier-list") {

    } else if (now->type == "parameter-type-list") {

    } else if (now->type == "parameter-list") {

    } else if (now->type == "parameter-declaration") {

    } else if (now->type == "identifier-list") {

    } else if (now->type == "type-name") {

    } else if (now->type == "initializer") {

    } else if (now->type == "initializer-list") {

    } else if (now->type == "statement") {

    } else if (now->type == "labeled-statement") {

    } else if (now->type == "identifier") {

    } else if (now->type == "statement") {

    } else if (now->type == "compound-statement") {

    } else if (now->type == "block-item-list") {

    } else if (now->type == "block-item") {

    } else if (now->type == "expression-statement") {

    } else if (now->type == "selection-statement") {

    } else if (now->type == "iteration-statement") {

    } else if (now->type == "jump-statement") {

    } else if (now->type == "primary-expression") {

    } else if (now->type == "constant") {

    } else if (now->type == "postfix-expression") {

    } else if (now->type == "array-access") {

    } else if (now->type == "function-call") {

    } else if (now->type == "argument-expression-list") {

    } else if (now->type == "unary-expression") {

    } else if (now->type == "unary-operator") {

    } else if (now->type == "cast-expression") {

    } else if (now->type == "multiplicative-expression") {

    } else if (now->type == "additive-expression") {

    } else if (now->type == "shift-expression") {

    } else if (now->type == "relational-expression") {

    } else if (now->type == "equality-expression") {

    } else if (now->type == "AND-expression") {

    } else if (now->type == "exclusive-OR-expression") {

    } else if (now->type == "inclusive-OR-expression") {

    } else if (now->type == "logical-AND-expression") {

    } else if (now->type == "logical-OR-expression") {

    } else if (now->type == "conditional-expression") {

    } else if (now->type == "assignment-expression") {

    } else if (now->type == "assignment-operator") {

    } else if (now->type == "expression") {

    } else if (now->type == "constant-expression") {

    }
    for (auto x:now->sons)
        semantic_check(x);
}


type *semantic::get_type_from_specifier(cnp now) {

    if (now->type == "type-specifier") {
        return get_type_from_specifier(now->sons[0]);
    } else if (type_specifiers.count(now->token)) {
        return type_table.get(now->token);
    } else if (now->type == "struct-or-union-specifier") {
        level_up();
        string struct_or_union = now->sons[0]->token;
        string name;
        vector<cnp> declist;
        for (int i = 0; i < now->sonnames.size(); i++) {
            if (now->sonnames[i] == "identifier") {
                name = now->sons[i]->token;
            }
            if (now->sonnames[i] == "struct-declaration-list") {
                if (now->sons[i]->type == "struct-declaration-list")
                    declist = now->sons[i]->sons;
                else
                    declist.push_back(now->sons[i]);
            }
        }

        auto newType = type_table.new_type(struct_or_union, name);
        if (newType->defined == true && !declist.empty()) {
            throw duplicate_type();
        } else {
            newType->defined = true;
            for (auto d:declist) {
                set<string> qualifiers;
                type *specifiers = nullptr;
                auto p = get_type_qualifiers(d->sons[0]);
                qualifiers = p.second;
                specifiers = p.first;

                if (d->sonnames[1] == "struct-declarator-list") {
                    if (d->sons[1]->type == "struct-declarator-list") {
                        for (auto struct_declarator:d->sons[1]->sons) {
                            if (struct_declarator->type != ",") {
                                auto r = get_rtype(specifiers, qualifiers, struct_declarator);
                                auto name = get_declarator_name(struct_declarator);
                                newType->fields.emplace_back(r, name);
                            }
                        }
                    } else {
                        auto r = get_rtype(specifiers, qualifiers, d->sons[1]);
                        auto name = get_declarator_name(d->sons[1]);
                        newType->fields.emplace_back(r, name);

                        if (var_table.new_var(r, name, scope_level) == nullptr) {
                            throw duplicate_definition();
                        }
                    }
                } else {
                    newType->fields.emplace_back(rtype(specifiers, qualifiers, 0), "");
                }
            }
        }
        level_down();
        return newType;
    } else if (now->type == "enum-specifier") {
        level_up();
        string name;
        vector<cnp> emulist;
        for (int i = 0; i < now->sonnames.size(); i++) {
            if (now->sonnames[i] == "identifier") {
                name = now->sons[i]->token;
            }
            if (now->sonnames[i] == "enumerator-list") {
                if (now->sons[i]->type == "enumerator-list")
                    emulist = now->sons[i]->sons;
                else
                    emulist.push_back(now->sons[i]);
            }
        }
        auto newType = type_table.new_type("enum", name);
        if (newType->defined == true && !emulist.empty()) {
            throw duplicate_type();
        } else {
            newType->defined = true;
            for (auto e:emulist) {
                if (e->type == ",")
                    continue;
                newType->fields.emplace_back(type_table.get("unsigned"), e->token);
                if (var_table.new_var(type_table.get("unsigned"), e->token, scope_level) == nullptr) {
                    throw duplicate_definition();
                }
            }
        }
        level_down();
        return newType;
    }
}

string semantic::get_qualifier(cnp now) {
    if (now->type == "type-qualifier") {
        return get_qualifier(now->sons[0]);
    } else {
        if (set<string>({"const", "restrict", "volatile"}).count(now->token)) {
            return now->token;
        }
    }
}

rtype semantic::get_rtype(type *ft, set<string> quas, cnp declarator) {
    if (declarator->type == "struct-declarator")
        return get_rtype(ft, std::move(quas), declarator->sons[0]);
    else {
        int pcnt = 0;

        if (!declarator->sons.empty()) {
            if (declarator->sons[0]->type == "pointer") {
                pcnt = declarator->sons[0]->sons.size();
            }
            if (declarator->sons[0]->type == "*") {
                pcnt = 1;
            }
        }

        rtype re(ft, quas, pcnt);
        if (!declarator->sons.empty()) {
            auto direct_dec = declarator->sons.back();
            while (direct_dec->type == "direct-declarator") {
                if (direct_dec->sons.size() >= 3 && direct_dec->sons[1]->type == "[") {
                    if (direct_dec->sons[2]->type == "]") {
                        re.array_size.push_back(-1);
                    } else {
                        if (direct_dec->sons[2]->type == "integer-constant") {
                            re.array_size.push_back(atoi(direct_dec->sons[2]->token.data()));
                        } else {
                            throw illegal_index();
                        }
                    }
                }
                direct_dec = direct_dec->sons[0];
            }
        }
        reverse(re.array_size.begin(), re.array_size.end());
        return re;
    }
}

string semantic::get_declarator_name(cnp declarator) {

    if (declarator->type == "struct-declarator")
        return get_declarator_name(declarator->sons[0]);
    else if (declarator->type == "identifier") {
        return declarator->token;
    } else {
        auto direct_dec = declarator->sons.back();
        while (direct_dec->type == "direct-declarator") {
            direct_dec = direct_dec->sons[0];
        }
        return direct_dec->token;
    }
}

void semantic::debug() {
    cout << type_table.debug() << endl;
}

void semantic::level_up() {
    scope_level++;
}

void semantic::level_down() {
    for (auto x:var_table.m) {
        if (!x.second.empty() && x.second.back()->scope_level == scope_level) {
            x.second.pop_back();
        }
    }
    scope_level--;
}

pair<type *, set<string> > semantic::get_type_qualifiers(cnp declaration_specifiers) {
    auto sqs = vector({declaration_specifiers});
    if (declaration_specifiers->type == "declaration-specifiers") {
        sqs = declaration_specifiers->sons;
    }
    type *specifiers = nullptr;
    set<string> qualifiers;
    for (auto sq:sqs) {
        if (type_qualifiers.count(sq->type)) {
            qualifiers.insert(get_qualifier(sq));
        } else if (type_specifiers.count(sq->type)) {
            if (specifiers == nullptr)
                specifiers = get_type_from_specifier(sq);
            else
                throw spec_cnt_error();
        }
    }
    if (specifiers == nullptr) {
        throw spec_cnt_error();
    }
    return {specifiers, qualifiers};
}

void semantic::push_parameters(cnp func, fp f) {
    auto declarator = func->sons[1];
    while (declarator->type == "direct-declarator") {
        if (declarator->sons.size() >= 3 && declarator->sonnames[2] == "parameter-list") {
            auto declist = vector({declarator->sons[2]});
            if (declarator->sons[2]->type == "parameter-list")
                declist = declarator->sons[2]->sons;
            for (auto d:declist) {
                auto p = get_type_qualifiers(d->sons[0]);
                auto spec = p.first;
                auto quali = p.second;
                auto rt = get_rtype(spec, quali, d->sons[1]);
                auto name = get_declarator_name(d->sons[1]);
                var_table.new_var(rt, name, scope_level);
                f->parameters.emplace_back(rt, name);
            }
        }
        declarator = declarator->sons[0];
    }
}


