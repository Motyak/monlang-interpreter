#include <monlang-interpreter/builtin/print.h>

#include <utils/variant-utils.h>
#include <utils/loop-utils.h>

#include <iostream>
#include <iomanip>
#include <vector>

static void print(const value_t&);
static void print(const prim_value_t&);
static void print(const type_value_t&);
static void print(const struct_value_t&);
static void print(const enum_value_t&);

value_t builtin::print(const std::vector<value_t>& varargs) {
    LOOP for (auto arg: varargs) {
        if (!__first_it) {
            std::cout << " ";
        }
        print(arg);
        ENDLOOP
    }
    std::cout << "\n";
    return nil_value_t();
}

static void print(const value_t& val) {
    std::visit(
        [](auto* val){
            if (val == nullptr){
                std::cout << "$nil";
            }
            else {
                print(*val);
            }
        }
        , val
    );
}

static void print(const prim_value_t& prim_val) {
    std::visit(overload{
        [](prim_value_t::Byte byte){std::cout << "Byte(" << byte << ")";},
        [](prim_value_t::Bool bool_){std::cout << (bool_ == true? "$true" : "$false");},
        [](prim_value_t::Int int_){std::cout << int_;},
        [](prim_value_t::Float float_){std::cout << std::setprecision(std::numeric_limits<double>::digits10) << float_;},
        [](const prim_value_t::Str& str){std::cout << str;},
        [](const prim_value_t::List& list){
            std::cout << "[";
            LOOP for (auto val: list) {
                if (!__first_it) {
                    std::cout << ", ";
                }
                print(val);
                ENDLOOP
            }
            std::cout << "]";
        },
        [](const prim_value_t::Map& map){
            std::cout << "[";
            LOOP for (auto [key, val]: map) {
                if (!__first_it) {
                    std::cout << ", ";
                }
                print(key);
                std::cout << ":";
                print(val);
                ENDLOOP
            }
            std::cout << "]";
        },
        [](const prim_value_t::Lambda&){std::cout << "<lambda>";},
    }, prim_val.variant);
}

static void print(const type_value_t& type_val) {
    std::cout << "(" << type_val.type;
    print(type_val.value);
    std::cout << ")";
}

static void print(const struct_value_t& struct_val) {
    std::cout << struct_val.type << "(";
    LOOP for (auto [_field_name, field_value]: struct_val.fields) {
        if (!__first_it) {
            std::cout << ", ";
        }
        print(field_value);
        ENDLOOP
    }
    std::cout << ")";
}

static void print(const enum_value_t& enum_val) {
    std::cout << enum_val.type << "(";
    print(enum_val.enumerate_value);
    std::cout << ")";
}
