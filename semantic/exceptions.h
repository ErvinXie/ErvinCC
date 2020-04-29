//
// Created by 谢威宇 on 2020/4/29.
//

#ifndef ERVINCC_EXCEPTIONS_H
#define ERVINCC_EXCEPTIONS_H

#include <exception>

using namespace std;


class var_not_defined : public exception {
    const char *what() const noexcept override {
        return "Variables not defined";
    }
};


class duplicate_type : public exception {
    const char *what() const noexcept override {
        return "duplicate type definition";
    }
};

class duplicate_definition : public exception {
    const char *what() const noexcept override {
        return "duplicate definition";
    }
};

class break_not_in_loop : public exception {
    const char *what() const noexcept override {
        return "Break appears outside a loop or switch";
    }
};

class func_args_not_match : public exception {
    const char *what() const noexcept override {
        return "Function args not match in numbers or types";
    }
};

class opnd_not_match : public exception {
    const char *what() const noexcept override {
        return "Types of variables not match";
    }
};

class array_access_out_of_bound : public exception {
    const char *what() const noexcept override {
        return "Array access out of bound";
    }
};

class goto_target_not_exist : public exception {
    const char *what() const noexcept override {
        return "Goto target not exist";
    }
};

class func_not_returned : public exception {
    const char *what() const noexcept override {
        return "Function not returned";
    }
};

class func_with_array : public exception {
    const char *what() const noexcept override {
        return "Function appear with array";
    }
};

class spec_cnt_error : public exception {
    const char *what() const noexcept override {
        return "Specifiers must be one";
    }
};

class illegal_index : public exception {
public:
    const char *what() const noexcept override {
        return "Array index must be an integer";
    }
};

#endif //ERVINCC_EXCEPTIONS_H
