#include <monlang-interpreter/builtin/print.h>

/* impl only */

#include <monlang-interpreter/builtin/prim_ctors.h>
#include <monlang-interpreter/interpret.h>

#include <utils/variant-utils.h>
#include <utils/loop-utils.h>

#include <iostream>
#include <iomanip>
#include <vector>

static void print(const value_t&, std::ostream& = std::cout);
static void print(const prim_value_t&, std::ostream& = std::cout);
static void print(const type_value_t&, std::ostream& = std::cout);
static void print(const struct_value_t&, std::ostream& = std::cout);
static void print(const enum_value_t&, std::ostream& = std::cout);

const value_t builtin::print __attribute__((init_priority(3000))) = new prim_value_t{prim_value_t::Lambda{
    new prim_value_t{prim_value_t::Int(0)},
    [](const std::vector<FlattenArg>& varargs) -> value_t {
        LOOP for (auto arg: varargs) {
            if (!__first_it) {
                std::cout << " ";
            }
            auto argValue = evaluateValue(arg.expr, arg.env);
            ::print(argValue);
            ENDLOOP
        }
        std::cout << "\n";
        return nil_value_t();
    }
}};

static void print(const value_t& val, std::ostream& out) {
    std::visit(
        [&out](auto* val){
            if (val == nullptr){
                out << "$nil";
            }
            else {
                print(*val, out);
            }
        }
        , val
    );
}

static void print(const prim_value_t& primVal, std::ostream& out) {
    std::visit(overload{
        [&out](prim_value_t::Byte byte){out << (unsigned)byte;},
        [&out](prim_value_t::Bool bool_){out << (bool_ == true? "$true" : "$false");},
        [&out](prim_value_t::Int int_){out << int_;},
        [&out](prim_value_t::Float float_){out << std::setprecision(std::numeric_limits<double>::digits10) << float_;},
        [&out](const prim_value_t::Str& str){out << str;},
        [&out](const prim_value_t::List& list){
            out << "[";
            LOOP for (auto val: list) {
                if (!__first_it) {
                    out << ", ";
                }
                print(val);
                ENDLOOP
            }
            out << "]";
        },
        [&out](const prim_value_t::Map& map){
            out << "[";
            LOOP for (auto [key, val]: map) {
                if (!__first_it) {
                    out << ", ";
                }
                print(key);
                out << ":";
                print(val);
                ENDLOOP
            }
            out << "]";
        },
        [&out](const prim_value_t::Lambda&){out << "<lambda>";},
    }, primVal.variant);
}

static void print(const type_value_t& type_val, std::ostream& out) {
    out << type_val.type << "(";
    print(type_val.value);
    out << ")";
}

static void print(const struct_value_t& struct_val, std::ostream& out) {
    out << struct_val.type << "(";
    LOOP for (auto [_field_name, field_value]: struct_val.fields) {
        if (!__first_it) {
            out << ", ";
        }
        print(field_value);
        ENDLOOP
    }
    out << ")";
}

static void print(const enum_value_t& enum_val, std::ostream& out) {
    out << enum_val.type << "(";
    print(enum_val.enumerate_value);
    out << ")";
}

value_t builtin::print_(const std::vector<value_t>& varargs, std::ostream& out) {
    LOOP for (auto arg: varargs) {
        if (!__first_it) {
            out << " ";
        }
        ::print(arg, out);
        ENDLOOP
    }
    out << "\n";
    return nil_value_t();
};
