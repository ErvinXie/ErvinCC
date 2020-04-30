//
// Created by 谢威宇 on 2020/4/28.
//

#include "semantic.h"
#include <fstream>
#include <set>
#include <utility>


int main(int argc, char *argv[]) {
    string json_dir;
    for (int i = 0; i < argc; i++) {
        json_dir = argv[i];
    }
    cout << json_dir << endl;
    auto p = from_string(json_dir, true);
    semantic s;
    try {
        s.semantic_check(p);
//        s.debug();
    }
    catch (exception &e) {
        cout << e.what() << endl;
        return 1;
    }
    cout << "Semantic OK" << endl;
    return 0;
}

set<string> type_specifiers({"void", "char", "short", "int",
                             "long", "float", "double", "signed",
                             "unsigned", "struct-or-union-specifier", "enum-specifier"});
set<string> type_qualifiers({"const", "restrict", "volatile", "extern", "static", "register"});


semantic::semantic() {
    iteration_cnt = 0;
    token_cnt = 0;
    scope_level = 0;
}


void semantic::semantic_check(cnp now) {
    cout << now->type << " " << now->token << endl;
    if (now->type == "translation-unit") {

    } else if (now->type == "external-declaration") {

    } else if (now->type == "function-definition") {
        level_up();
        auto p = get_type_qualifiers(now->sons[0]);
        auto specifier = p.first;
        auto qualifier = p.second;
        auto rt = get_rtype_of_declarator(specifier, qualifier, now->sons[1]);
        auto name = get_declarator_name(now->sons[1]);

        auto newFunc = func_table.new_func(rt, name);
        nowfunc = newFunc;
        if (newFunc->defined) {
            cout << "Around: " << now->content() << endl;
            throw duplicate_definition();
        } else if (newFunc->parameters.empty()) {
            push_parameters(now, newFunc);
        }
        semantic_check(now->sons[2]);
        if (nowfunc->returned == false) {
            cout << "Around: " << now->content() << endl;
            throw func_not_returned();
        }
        level_down();
        nowfunc = nullptr;
        return;

    } else if (now->type == "declaration-list") {

    } else if (now->type == "declaration") {
        auto p = get_type_qualifiers(now->sons[0]);
        auto specifier = p.first;
        auto qualifier = p.second;
        auto idlist = vector({now->sons[1]});
        if (now->sonnames[1] == ";") {
            return;
        }
        if (now->sons[1]->type == "init-declarator-list")
            idlist = now->sons[1]->sons;
        for (auto id:idlist) {
            if (id->type == ",")
                continue;
            auto declarator = id;
            if (id->type == "init-declarator") {
                declarator = id->sons[0];
            }
            auto rt = get_rtype_of_declarator(specifier, qualifier, declarator);
            auto name = get_declarator_name(declarator);
            auto new_var = var_table.new_var(rt, name, scope_level);
            if (id->type == "init-declarator") {
                auto r = get_rtype_of_expression(id->sons[2]);
                if (r.congruent(new_var->r) == false) {
                    cout << "Around: " << id->content() << endl;
                    throw initializer_not_match();
                }
            }

        }
        return;
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
        nowfunc->new_label(now->sons[0]->token);
    } else if (now->type == "identifier") {

    } else if (now->type == "statement") {

    } else if (now->type == "compound-statement") {
        level_up();
        for (auto x:now->sons)
            semantic_check(x);
        level_down();
        return;

    } else if (now->type == "block-item-list") {

    } else if (now->type == "block-item") {

    } else if (now->type == "expression-statement") {
        if (now->sonnames[0] == "expression") {
            get_rtype_of_expression(now->sons[0]);
            return;
        }
    } else if (now->type == "selection-statement") {

    } else if (now->type == "iteration-statement") {
        iteration_cnt++;
        for (auto x:now->sons)
            semantic_check(x);
        iteration_cnt--;
        return;
    } else if (now->type == "for-statement") {
        level_up();
        for (auto x:now->sons)
            semantic_check(x);
        level_down();
        return;
    } else if (now->type == "jump-statement") {
        if (now->sonnames[0] == "goto") {
            if (nowfunc->labels.count(now->sons[1]->token) == 0) {
                cout << "Around: " << now->content() << endl;
                throw label_not_exist();
            }
        } else if (now->sonnames[0] == "continue" || now->sonnames[0] == "break") {
            if (iteration_cnt == 0) {
                cout << "Around: " << now->content() << endl;
                throw break_or_continue_not_in_loop();
            }
        } else if (now->sonnames[0] == "return") {
            if (now->sonnames[1] == ";") {
                if (nowfunc->r.f->name != "void") {
                    cout << "Around: " << now->content() << endl;
                    throw return_type_not_match();
                }
            } else {
                auto r = get_rtype_of_expression(now->sons[1]);
                if (r.congruent(nowfunc->r)) {
                    nowfunc->returned = true;
                } else {
                    cout << "Around: " << now->content() << endl;
                    throw return_type_not_match();
                }
            }
        }

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
            cout << "Around: " << now->content() << endl;
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
                    auto sdlist = vector({d->sons[1]});
                    if (d->sons[1]->type == "struct-declarator-list") {
                        sdlist = d->sons[1]->sons;
                    }
                    for (auto struct_declarator:sdlist) {
                        if (struct_declarator->type != ",") {
                            auto r = get_rtype_of_declarator(specifiers, qualifiers, struct_declarator);
                            auto name = get_declarator_name(struct_declarator);
                            newType->fields.emplace_back(r, name);
                            auto new_var = var_table.new_var(r, name, scope_level);
                            if (new_var == nullptr) {
                                cout << "Around: " << struct_declarator->content() << endl;
                                throw duplicate_definition();
                            }
                            if (struct_declarator->type == "struct-declarator") {
                                auto rt = get_rtype_of_expression(struct_declarator->sons[2]);
                                if (new_var->r.congruent(rt) == false) {
                                    cout << "Around: " << struct_declarator->content() << endl;
                                    throw initializer_not_match();
                                }
                            }
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
            cout << "Around: " << now->content() << endl;
            throw duplicate_type();
        } else {
            newType->defined = true;
            for (auto e:emulist) {
                if (e->type == ",")
                    continue;
                newType->fields.emplace_back(type_table.get("unsigned"), e->token);
                if (var_table.new_var(type_table.get("unsigned"), e->token, scope_level) == nullptr) {
                    cout << "Around: " << now->content() << endl;
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
        if (type_qualifiers.count(now->token)) {
            return now->token;
        }
    }
}

rtype semantic::get_rtype_of_declarator(type *ft, set<string> quas, cnp declarator) {
    if (declarator->type == "struct-declarator")
        return get_rtype_of_declarator(ft, std::move(quas), declarator->sons[0]);
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
                            cout << "Around: " << declarator->content() << endl;
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
        auto direct_dec = declarator;
        if (declarator->type == "declarator")
            direct_dec = declarator->sons.back();
        while (direct_dec->type == "direct-declarator") {
            direct_dec = direct_dec->sons[0];
        }
        return direct_dec->token;
    }
}

void semantic::debug() {
    cout << type_table.debug() << endl;
    cout << func_table.debug() << endl;
    cout << var_table.debug() << endl;
}

void semantic::level_up() {
    scope_level++;
}

void semantic::level_down() {
//    cout << var_table.debug() << endl;
    for (auto &x:var_table.m) {
        if (!x.second.empty() && x.second.back()->scope_level == scope_level) {
//            cout << x.first << endl;

            x.second.pop_back();

//            cout << x.first << endl;
        }
    }
//    cout << var_table.debug() << endl;
//    cout << "---" << endl;
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
            else {
                cout << "Around: " << declaration_specifiers->content() << endl;
                throw spec_cnt_error();
            }
        }
    }
    if (specifiers == nullptr) {
        cout << "Around: " << declaration_specifiers->content() << endl;
        throw spec_cnt_error();
    }
    return {specifiers, qualifiers};
}

void semantic::push_parameters(cnp func, fp f) {
    auto declarator = func->sons[1];
    if (declarator->type == "declarator") {
        declarator = declarator->sons.back();
    }

    if (declarator->sons.size() >= 3 && declarator->sonnames[2] == "parameter-list") {

        auto declist = vector({declarator->sons[2]});
        if (declarator->sons[2]->type == "parameter-list")
            declist = declarator->sons[2]->sons;
        for (auto d:declist) {
            if (d->type == ",")
                continue;
            auto p = get_type_qualifiers(d->sons[0]);
            auto spec = p.first;
            auto quali = p.second;
            auto rt = get_rtype_of_declarator(spec, quali, d->sons[1]);
            auto name = get_declarator_name(d->sons[1]);
            var_table.new_var(rt, name, scope_level);
            f->parameters.emplace_back(rt, name);
        }
    }

}

rtype semantic::get_rtype_of_expression(cnp now) {
    if (now->type == "primary-expression") {
        return get_rtype_of_expression(now->sons[1]);
    } else if (now->type == "identifier") {
        return var_table.get(now->token)->r;
    } else if (now->type == "string-literal") {
        return rtype(type_table.get("char"), {}, 1);
    } else if (now->type == "integer-constant") {
        return rtype(type_table.get("int"));
    } else if (now->type == "floating-constant") {
        return rtype(type_table.get("double"));
    } else if (now->type == "character-constant") {
        return rtype(type_table.get("char"));
    } else if (now->type == "postfix-expression") {
        if (now->sonnames[1] == ".") {
            auto r = get_rtype_of_expression(now->sons[0]);
            if (r.pointer != 0 || r.array_size.empty() == false) {
                cout << "Around: " << now->content() << endl;
                throw pointer_expected();
            } else {
                return r.f->get_sub(now->sons[2]->token);
            }
        } else if (now->sonnames[1] == "->") {
            auto r = get_rtype_of_expression(now->sons[0]);
            if (r.pointer == 0 && r.array_size.empty() == false) {
                cout << "Around: " << now->content() << endl;
                throw pointer_not_expected();
            } else {
                return r.f->get_sub(now->sons[2]->token);
            }
        } else if (now->sonnames[1] == "++" || now->sonnames[1] == "--") {
            return get_rtype_of_expression(now->sons[0]);
        }
    } else if (now->type == "array-access") {
        auto r = get_rtype_of_expression(now->sons[0]);
        r.array_size.pop_back();
        return r;
    } else if (now->type == "function-call") {

        auto name = now->sons[0]->token;
        auto f = func_table.get(name);

        if (now->sonnames[2] == "argument-expression-list") {
            auto arglist = vector<cnp>({now->sons[2]});
            if (now->sons[2]->type == "argument-expression-list")
                arglist = now->sons[2]->sons;

            if (arglist.size() != f->parameters.size()) {
                cout << "Around: " << now->content() << endl;
                throw func_args_not_match();
            }
            for (int i = 0; i < arglist.size(); i++) {
                auto a = arglist[i];
                if (a->type == ",")
                    continue;
                auto rt = get_rtype_of_expression(a);
                if (rt.congruent(f->parameters[i].first) == false) {
                    cout << "Around: " << now->content() << endl;
                    throw func_args_not_match();
                }
            }
        } else {
            if (f->parameters.empty() == false) {
                cout << "Around: " << now->content() << endl;
                throw func_args_not_match();
            }
        }
        return f->r;
    } else if (now->type == "argument-expression-list") {

    } else if (now->type == "unary-expression") {
        if (now->sonnames[0] == "++" || now->sonnames[0] == "--") {
            return get_rtype_of_expression(now->sons[1]);
        } else if (now->sonnames[0] == "unary-operator") {
            auto t = now->sons[0]->type;
            auto r = get_rtype_of_expression(now->sons[1]);
            if (t == "+" || t == "-" || t == "~") {
                return r;
            } else if (t == "*") {
                return r.get_tar();
            } else if (t == "&") {
                return r.get_add();
            } else if (t == "!") {
                return rtype(type_table.get("int"));
            }
        } else if (now->sonnames[0] == "sizeof") {
            return rtype(type_table.get("int"));
        }
    } else if (now->type == "unary-operator") {

    } else if (now->type == "cast-expression") {
        auto p = get_type_qualifiers(now->sons[1]);
        return rtype(p.first);

    } else if (now->type == "multiplicative-expression"
               || now->type == "additive-expression"
               || now->type == "shift-expression"
               || now->type == "AND-expression"
               || now->type == "exclusive-OR-expression"
               || now->type == "inclusive-OR-expression"
               || now->type == "assignment-expression") {
        auto l = get_rtype_of_expression(now->sons[0]);
        auto r = get_rtype_of_expression(now->sons[2]);
        if (l.congruent(r) == false) {
            cout << "Around: " << now->content() << endl;
            throw opnd_not_match();
        } else {
            return l;
        }

    } else if (now->type == "relational-expression"
               || now->type == "equality-expression"
               || now->type == "logical-AND-expression"
               || now->type == "logical-OR-expression") {
        return rtype(type_table.get("int"));
    } else if (now->type == "conditional-expression") {
        auto l = get_rtype_of_expression(now->sons[2]);
        auto r = get_rtype_of_expression(now->sons[4]);
        if (l.congruent(r) == false) {
            cout << "Around: " << now->content() << endl;
            throw opnd_not_match();
        } else {
            return l;
        }
    } else if (now->type == "expression") {
        get_rtype_of_expression(now->sons[2]);
        return get_rtype_of_expression(now->sons[0]);
    }
}


