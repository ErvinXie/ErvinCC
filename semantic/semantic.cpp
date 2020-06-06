//
// Created by 谢威宇 on 2020/4/28.
//

#include "semantic.h"
#include <fstream>
#include <set>
#include <utility>


int main(int argc, char *argv[]) {
    string json_dir;
    string ir_dir;
    for (int i = 0; i < argc; i++) {
        json_dir = argv[i];
    }
//    json_dir = "/Users/ervinxie/ClionProjects/ErvinCC/test-files/var-not-defined.json";
    for (int i = (int) json_dir.length() - 1; i >= 0; i--) {
        if (json_dir[i] == '.') {
            ir_dir = json_dir.substr(0, i) + ".ll";
            break;
        }
    }


    cout << json_dir << endl;
    auto p = from_string(json_dir, true);
    semantic s(ir_dir);
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

semantic::semantic(string outdir) {
    irout = ofstream(outdir);
    iteration_cnt = 0;
    token_cnt = 0;
    scope_level = 0;
    global_temp_variable = 0;

    auto fp = func_table.new_func(rtype(type_table.get("void")), "Mars_PrintStr");
    fp->parameters.push_back(*var_table.new_var(rtype(type_table.get("char"), {}, 1), "", scope_level));

    fp = func_table.new_func(rtype(type_table.get("void")), "Mars_PrintInt");
    fp->parameters.push_back(*var_table.new_var(rtype(type_table.get("int")), "", scope_level));

    fp = func_table.new_func(rtype(type_table.get("int")), "Mars_GetInt");

}

set<string> type_specifiers({"void", "char", "short", "int",
                             "long", "float", "double", "signed",
                             "unsigned", "struct-or-union-specifier", "enum-specifier"});
set<string> type_qualifiers({"const", "restrict", "volatile", "extern", "static", "register"});


void semantic::semantic_check(cnp now) {
//    cout << now->type << " " << now->token << endl;
    if (now->type == "translation-unit") {

    } else if (now->type == "external-declaration") {

    } else if (now->type == "function-definition") {
        irout << endl << "; function-definition" << endl;
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
        string parameter_list = "(";
        for (int i = 0; i < nowfunc->parameters.size(); i++) {
            parameter_list += nowfunc->parameters[i].r.llvm_type();
            if (i == nowfunc->parameters.size() - 1)
                parameter_list += ")";
            else
                parameter_list += ",";
        }
        if (parameter_list.size() == 1)
            parameter_list.push_back(')');

        irout << "define " << rt.llvm_type() << " @" << name << parameter_list << "{" << endl;
        for (int i = 0; i < nowfunc->parameters.size(); i++) {
            auto par = nowfunc->parameters[i];
            auto name = get_temp_idx();
            irout << "    %" << par.level_name() << " = alloca " << par.r.llvm_type() << endl;
            irout << "    %" << par.level_name() << " store %" << name << endl;
        }
        semantic_check(now->sons[2]);
        irout << "}" << endl;
        if (nowfunc->returned == false) {
            cout << "Around: " << now->content() << endl;
            throw func_not_returned();
        }
        newFunc->defined = true;
        level_down();
        nowfunc = nullptr;
        irout << "; function-definition-end" << endl << endl;
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

            auto direct_declarator = declarator;
            if (direct_declarator->type == "declarator")
                direct_declarator = declarator->sons[1];
            if (direct_declarator->sons.size() >= 3 && direct_declarator->sonnames[1] == "(") {
                auto newFunc = func_table.new_func(rt, name);
                push_parameters(now, newFunc);
                irout << "declare " << rt.llvm_type() << " @" << name << "(";
                for (int i = 0; i < newFunc->parameters.size(); i++) {
                    irout << newFunc->parameters[i].r.llvm_type();
                    if (i == newFunc->parameters.size() - 1)
                        irout << ")" << endl;
                    else
                        irout << ",";
                }
                if (newFunc->parameters.empty())
                    irout << ")" << endl;

            } else {
                auto new_var = var_table.new_var(rt, name, scope_level);
                if (nowfunc == nullptr)
                    irout << "@";
                else
                    irout << "    %";
                irout << new_var->level_name() << " = " << "alloca " << rt.llvm_type() << endl;

                if (id->type == "init-declarator") {
                    auto v = get_variable_of_expression(id->sons[2]);
                    if (v.r.congruent(new_var->r) == false) {
                        cout << "Around: " << id->content() << endl;
                        throw initializer_not_match();
                    } else {
                        irout << "    %" << new_var->level_name() << " store %" << v.level_name() << endl;
                    }
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
        auto name = now->sons[0]->token;
        nowfunc->new_label(name);
        irout << "" << name << ":" << endl;
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
            auto v = get_variable_of_expression(now->sons[0]);
            return;
        }
    } else if (now->type == "selection-statement") {
        irout << endl << "; if-statement" << endl;
        auto condition = get_variable_of_expression(now->sons[2]);
        if (now->sons.size() <= 5) {
            auto if_start = to_string(get_temp_idx());
            auto if_end = to_string(get_temp_idx());
            irout << "    br i1 %" << condition.level_name() << ", label %" << if_start
                  << ", lable %" << if_end << endl;
            irout << "" << if_start << ":" << endl;
            semantic_check(now->sons[4]);
            irout << "" << if_end << ":" << endl;
        } else {
            auto if_start = to_string(get_temp_idx());
            auto else_start = to_string(get_temp_idx());
            auto if_end = to_string(get_temp_idx());
            irout << "    br " << condition.r.llvm_type() << " %" << condition.level_name() << ", label %" << if_start
                  << ", lable %" << else_start << endl;
            irout << "" << if_start << ":" << endl;
            semantic_check(now->sons[4]);
            irout << "    br label %" << if_end << endl;
            irout << "" << else_start << ":" << endl;
            semantic_check(now->sons[6]);
            irout << "" << if_end << ":" << endl;
        }
        irout << "; if-statement-end" << endl << endl;
        return;
    } else if (now->type == "iteration-statement") {
        irout << endl << "; iteration-statement" << endl;
        iteration_cnt++;
        if (now->sonnames[0] == "for") {
            level_up();
            auto for_initial = now->sons[1]->sons[1];
            auto for_restriction = now->sons[1]->sons[2];
            auto for_increase = now->sons[1]->sons[3];
            irout << "; for-start" << endl;
            irout << "; " << for_initial->type << endl;
            semantic_check(for_initial);
            auto for_check = to_string(get_temp_idx());
            auto for_start = to_string(get_temp_idx());
            auto for_end = to_string(get_temp_idx());
            iteration_start.push_back(for_check);
            iteration_end.push_back(for_end);
            irout << "; for-condition" << endl;
            irout << "" << for_check << ":" << endl;
            if (for_restriction->type != ";") {
                for_restriction = for_restriction->sons[0];
                auto condition = get_variable_of_expression(for_restriction);
                irout << "    br " << condition.r.llvm_type() << " %" << condition.level_name()
                      << ", label %" << for_start
                      << ", lable %" << for_end << endl;
            }
            irout << "" << for_start << ":" << endl;
            semantic_check(now->sons[2]);
            irout << "; for-increase" << endl;
            get_variable_of_expression(for_increase);
            irout << "    br label %" << for_check << endl;
            irout << "" << for_end << ":" << endl;
            iteration_end.pop_back();
            iteration_start.pop_back();
            level_down();
        } else if (now->sonnames[0] == "while") {
            auto while_restriction = now->sons[2];

            auto while_check = to_string(get_temp_idx());
            auto while_start = to_string(get_temp_idx());
            auto while_end = to_string(get_temp_idx());
            iteration_start.push_back(while_check);
            iteration_end.push_back(while_end);
            irout << "" << while_check << ":" << endl;
            auto condition = get_variable_of_expression(while_restriction);
            irout << "    br " << condition.r.llvm_type() << " %" << condition.level_name()
                  << ", label %" << while_start
                  << ", lable %" << while_end << endl;

            irout << "" << while_start << ":" << endl;
            semantic_check(now->sons[4]);
            irout << "    br label %" << while_check << endl;
            irout << "" << while_end << ":" << endl;
            iteration_end.pop_back();
            iteration_start.pop_back();

        } else if (now->sonnames[0] == "do") {
            auto while_restriction = now->sons[4];
            auto while_start = to_string(get_temp_idx());
            auto while_end = to_string(get_temp_idx());
            iteration_start.push_back(while_start);
            iteration_end.push_back(while_end);
            irout << "" << while_start << ":" << endl;
            semantic_check(now->sons[1]);

            auto condition = get_variable_of_expression(while_restriction);
            irout << "    br " << condition.r.llvm_type() << " %" << condition.level_name()
                  << ", label %" << while_start
                  << ", lable %" << while_end << endl;

            irout << "" << while_end << ":" << endl;
            iteration_end.pop_back();
            iteration_start.pop_back();
        }
        iteration_cnt--;
        irout << "; iteration-statement-end" << endl << endl;
        return;
    } else if (now->type == "jump-statement") {
        if (now->sonnames[0] == "goto") {
            if (nowfunc->labels.count(now->sons[1]->token) == 0) {
                cout << "Around: " << now->content() << endl;
                throw label_not_exist();
            }
            auto label_name = now->sons[1]->token;
            irout << "    br label " << label_name << endl;
        } else if (now->sonnames[0] == "continue" || now->sonnames[0] == "break") {
            if (iteration_cnt == 0) {
                cout << "Around: " << now->content() << endl;
                throw break_or_continue_not_in_loop();
            }
            if (now->sonnames[0] == "continue") {
                irout << "    br label %" << iteration_start.back() << endl;
            } else if (now->sonnames[0] == "break") {
                irout << "    br label %" << iteration_end.back() << endl;
            }
        } else if (now->sonnames[0] == "return") {
            if (now->sonnames[1] == ";") {
                if (nowfunc->r.f->name != "void") {
                    cout << "Around: " << now->content() << endl;
                    throw return_type_not_match();
                }
                irout << "    ret void" << endl;
            } else {
                auto r = get_variable_of_expression(now->sons[1]);
                if (r.r.congruent(nowfunc->r)) {
                    nowfunc->returned = true;
                } else {
                    cout << "Around: " << now->content() << endl;
                    throw return_type_not_match();
                }
                irout << "    ret " << r.r.llvm_type() << " %" << r.level_name() << endl;
            }
        }
        return;
    }
//    irout << "; " << now->type << endl;
    for (int i = 0; i < now->sons.size(); i++) {
        if (i < now->sonnames.size()) {
            if (now->sonnames[i] == "expression") {
                get_variable_of_expression(now->sons[i]);
            }
        } else {
            semantic_check(now->sons[i]);
        }
    }

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
            if (declist.empty() == false) {
                newType->defined = true;
            }
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
                                auto rt = get_variable_of_expression(struct_declarator->sons[2]).r;
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

        irout << "@" << newType->name << " = " << "type " << newType->llvm_type_define() << endl;

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
            if (declarator->type == "direct-declarator")
                direct_dec = declarator;

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
            auto nv = var_table.new_var(rt, name, scope_level);
            f->parameters.push_back(*nv);
        }
    }

}

variable semantic::get_variable_of_expression(cnp now) {
    if (now->type == "primary-expression") {
        return get_variable_of_expression(now->sons[1]);
    } else if (now->type == "identifier") {
        return *var_table.get(now->token);
    } else if (now->type == "string-literal") {
        auto name = to_string(get_temp_idx());
        irout << "    %" << name << " = alloca ";
        rtype str_type(type_table.get("char"), {}, 0);
        str_type.array_size.push_back(now->token.length() + 1);
        irout << str_type.llvm_type() << " "
              << "\"" << now->token
              << "\\0" << "\"" << endl;
        return *var_table.new_var(str_type, name, scope_level);
    } else if (now->type == "integer-constant") {
        auto name = to_string(get_temp_idx());
        irout << "    %" << name << " = alloca " << rtype(type_table.get("int")).llvm_type() << endl;
        irout << "    %" << name << " store " << now->token << endl;
        return *var_table.new_var(rtype(type_table.get("int"), {}, 0),
                                  name,
                                  scope_level);
    } else if (now->type == "floating-constant") {
        auto name = to_string(get_temp_idx());
        irout << "    %" << name << " = alloca " << rtype(type_table.get("double")).llvm_type() << endl;
        irout << "    %" << name << " store " << now->token << endl;
        return *var_table.new_var(rtype(type_table.get("double"), {}, 0),
                                  name,
                                  scope_level);
    } else if (now->type == "character-constant") {
        auto name = to_string(get_temp_idx());
        irout << "    %" << name << " = alloca " << rtype(type_table.get("char")).llvm_type() << endl;
        irout << "    %" << name << " store " << (int) now->token[0] << endl;
        return *var_table.new_var(rtype(type_table.get("char"), {}, 0),
                                  name,
                                  scope_level);
    } else if (now->type == "postfix-expression") {
        if (now->sonnames[1] == ".") {
            auto father_v = get_variable_of_expression(now->sons[0]);
            auto r = father_v.r;
            if (r.pointer != 0 || r.array_size.empty() == false) {
                cout << "Around: " << now->content() << endl;
                throw pointer_expected();
            } else {
                auto name = father_v.level_name() + "." + now->sons[2]->token;
                return *var_table.new_var(r.f->get_sub(now->sons[2]->token),
                                          name,
                                          scope_level);
            }
        } else if (now->sonnames[1] == "->") {
            auto father_v = get_variable_of_expression(now->sons[0]);
            auto r = father_v.r;
            if (r.pointer == 0 && r.array_size.empty() == false) {
                cout << "Around: " << now->content() << endl;
                throw pointer_expected();
            } else {
                auto name = father_v.level_name() + "->" + now->sons[2]->token;
                return *var_table.new_var(r.f->get_sub(now->sons[2]->token),
                                          name,
                                          scope_level);
            }

        } else if (now->sonnames[1] == "++" || now->sonnames[1] == "--") {
            auto v = get_variable_of_expression(now->sons[0]);
//            irout << "    %" << v.level_name() << " = alloca " << v.r.llvm_type() << endl;
            irout << "    %" << v.level_name() << " store ";
            if (now->sonnames[1] == "++")
                irout << "+ ";
            else
                irout << "- ";
            irout << v.r.llvm_type() << " " << v.r.llvm_type() << " %" << v.level_name() << ", i32 " << 1 << endl;
            return v;
        }
    } else if (now->type == "array-access") {
        auto nex = now;
        vector<variable> va;
        while (nex->type == "array-access") {
            auto i = get_variable_of_expression(nex->sons[2]);
            va.push_back(i);
            nex = nex->sons[0];
        }
        reverse(va.begin(), va.end());
        auto v = get_variable_of_expression(nex);
        auto r = v.r;
        for (int i = 0; i < va.size(); i++)
            r = r.get_tar();
        auto name = to_string(get_temp_idx());
        irout << "    %" << name << " = alloca " << r.llvm_type() << endl;
        irout << "    %" << name << " store %" << v.level_name();
        for (auto i:va)
            irout << "(%" << i.level_name() << ")";
        irout << endl;
        return *var_table.new_var(r, name, 0);
    } else if (now->type == "function-call") {

        auto name = now->sons[0]->token;
        auto f = func_table.get(name);
        auto temp_name = to_string(get_temp_idx());

        string parameter_list = "(";
        if (now->sonnames[2] == "argument-expression-list") {
            auto arglistt = vector<cnp>({now->sons[2]});
            if (now->sons[2]->type == "argument-expression-list")
                arglistt = now->sons[2]->sons;

            decltype(arglistt) arglist;
            for (auto a:arglistt)
                if (a->type != ",")
                    arglist.push_back(a);

            if (arglist.size() != f->parameters.size()) {
                cout << "Around: " << now->content() << endl;
                cout << "Size:" << arglist.size() << endl;
                cout << f->debug() << endl;

                throw func_args_not_match();
            }
            for (int i = 0; i < arglist.size(); i++) {
                auto a = arglist[i];
                if (a->type == ",")
                    continue;
                auto v = get_variable_of_expression(a);
                if (v.r.congruent(f->parameters[i].r) == false) {
                    cout << "Around: " << now->content() << endl;
                    cout << "Type" << endl;
                    throw func_args_not_match();
                }
                parameter_list += " " + v.r.llvm_type() + " %" + v.level_name() + ",";
            }
            parameter_list.back() = ' ';
            parameter_list.push_back(')');
        } else {
            if (f->parameters.empty() == false) {
                cout << "Around: " << now->content() << endl;
                throw func_args_not_match();
            }
            parameter_list.push_back(' ');
            parameter_list.push_back(')');
        }
        if (f->r.llvm_type() == "void") {
            irout << "    call " << f->r.llvm_type() << " @" << f->name;
        } else {
            irout << "    %" << temp_name << " = alloca " << f->r.llvm_type() << endl;
            irout << "    %" << temp_name << " store call " << f->r.llvm_type() << " @" << f->name;
        }
        irout << parameter_list << endl;
        return *var_table.new_var(f->r, temp_name, scope_level);
    } else if (now->type == "argument-expression-list") {

    } else if (now->type == "unary-expression") {
        if (now->sonnames[0] == "++" || now->sonnames[0] == "--") {
            auto v = get_variable_of_expression(now->sons[1]);
//            irout << "    %" << v.level_name() << " = alloca " << v.r.llvm_type() << endl;
            irout << "    %" << v.level_name() << " store ";
            if (now->sonnames[0] == "++")
                irout << "+ ";
            else
                irout << "- ";
            irout << v.r.llvm_type() << " " << v.r.llvm_type() << " %" << v.level_name() << ", i32 " << 1 << endl;
            return v;
        } else if (now->sonnames[0] == "unary-operator") {
            auto op = now->sons[0]->type;
            auto v = get_variable_of_expression(now->sons[1]);
            if (op == "+" || op == "-" || op == "~") {
                auto name = to_string(get_temp_idx());
                auto r = v.r;
                irout << "    %" << name << " = alloca " << r.llvm_type() << endl;
                irout << "    %" << name << " store " << op << " " << r.llvm_type() << ", "
                      << v.r.llvm_type() << " %" << v.level_name() << endl;
                return *var_table.new_var(r, name, scope_level);
            } else if (op == "*") {
                //de
                auto name = to_string(get_temp_idx());
                auto r = v.r.get_tar();
                irout << "    %" << name << " = load " << r.llvm_type() << ", "
                      << v.r.llvm_type() << " %" << v.level_name() << endl;
                return *var_table.new_var(r, name, scope_level);

            } else if (op == "&") {
                //de
                auto name = to_string(get_temp_idx());
                auto r = v.r.get_add();
                irout << "    %" << name << " = get_addr " << r.llvm_type() << ", "
                      << v.r.llvm_type() << " %" << v.level_name() << endl;
                return *var_table.new_var(r, name, scope_level);
            } else if (op == "!") {
                auto name = to_string(get_temp_idx());
                auto r = type_table.get("int");
                irout << "    %" << name << " = alloca " << v.r.llvm_type() << endl;
                irout << "    %" << name << " store ! " << v.r.llvm_type() << " %" << v.level_name() << ", 0"
                      << endl;
                return *var_table.new_var(r, name, scope_level);
            }
        } else if (now->sonnames[0] == "sizeof") {
            //de
            if (now->sonnames[1] == "(") {
                auto name = to_string(get_temp_idx());
                auto r = type_table.get("int");
                irout << "% " << name << " = sizeof some type " << endl;
                return *var_table.new_var(r, name, scope_level);
            } else {
                auto v = get_variable_of_expression(now->sons[1]);
                auto name = to_string(get_temp_idx());
                auto r = type_table.get("int");
                irout << "% " << name << " = sizeof" << v.r.llvm_type() << " %" << v.level_name() << endl;
                return *var_table.new_var(r, name, scope_level);
            }
        }
    } else if (now->type == "unary-operator") {

    } else if (now->type == "cast-expression") {
        //de
        auto v = get_variable_of_expression(now->sons[3]);
        auto p = get_type_qualifiers(now->sons[1]);
        auto name = to_string(get_temp_idx());
        auto r = rtype(p.first);
        irout << "% " << name << " = cast " << r.llvm_type() << ", "
              << v.r.llvm_type() << " %" << v.level_name() << endl;
        return *var_table.new_var(r, name, scope_level);

    } else if (now->type == "multiplicative-expression"
               || now->type == "additive-expression"
               || now->type == "shift-expression"
               || now->type == "AND-expression"
               || now->type == "exclusive-OR-expression"
               || now->type == "inclusive-OR-expression"
               || now->type == "assignment-expression") {
        auto l = get_variable_of_expression(now->sons[0]);
        auto r = get_variable_of_expression(now->sons[2]);
        if (l.r.congruent(r.r) == false) {
            cout << "Around: " << now->content() << endl;
            throw opnd_not_match();
        } else {
            auto name = to_string(get_temp_idx());
            auto rt = l.r;
            irout << "    %" << name << " = alloca " << l.r.llvm_type() << endl;
            irout << "    %" << name << " store " << now->sons[1]->type << " " << l.r.llvm_type() << ", "
                  << l.r.llvm_type() << " %" << l.level_name() << ", "
                  << r.r.llvm_type() << " %" << r.level_name() << endl;
            return *var_table.new_var(rt, name, scope_level);
        }

    } else if (now->type == "relational-expression"
               || now->type == "equality-expression"
               || now->type == "logical-AND-expression"
               || now->type == "logical-OR-expression") {
        auto l = get_variable_of_expression(now->sons[0]);
        auto r = get_variable_of_expression(now->sons[2]);

        auto name = to_string(get_temp_idx());
        auto rt = type_table.get("int");
        irout << "    %" << name << " = alloca " << rtype(type_table.get("int")).llvm_type() << endl;
        irout << "    %" << name << " store " << now->sons[1]->type << " " << rt->llvm_type() << ", "
              << l.r.llvm_type() << " %" << l.level_name() << ", "
              << r.r.llvm_type() << " %" << r.level_name() << endl;
        return *var_table.new_var(rt, name, scope_level);

    } else if (now->type == "conditional-expression") {
        auto condition = get_variable_of_expression(now->sons[0]);
        auto l = get_variable_of_expression(now->sons[2]);
        auto r = get_variable_of_expression(now->sons[4]);
        if (l.r.congruent(r.r) == false) {
            cout << "Around: " << now->content() << endl;
            throw opnd_not_match();
        } else {
            auto left = to_string(get_temp_idx());
            auto right = to_string(get_temp_idx());
            auto end = to_string(get_temp_idx());
            irout << "    br " << condition.r.llvm_type() << " " << condition.level_name() << ", "
                  << " label %" << left << " , label %" << right << endl;

            auto name = to_string(get_temp_idx());
            auto rt = l.r;
            irout << "    %" << name << " = alloca " << l.r.llvm_type() << endl;
            irout << "" << left << ":" << endl;
            irout << "    %" << name << " store %" << l.level_name() << endl;
            irout << "    br label %" << end << endl;
            irout << "" << right << ":" << endl;
            irout << "    %" << name << " store %" << r.level_name() << endl;
            irout << "" << end << ":" << endl;
            return *var_table.new_var(rt, name, scope_level);
        }
    } else if (now->type == "expression") {
        get_variable_of_expression(now->sons[2]);
        return get_variable_of_expression(now->sons[0]);
    }
}

int semantic::get_temp_idx() {
    if (nowfunc == nullptr) {
        return global_temp_variable++;
    } else {
        return nowfunc->temp_varible++;
    }
}




