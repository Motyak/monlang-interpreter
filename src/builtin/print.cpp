#include <monlang-interpreter/builtin/print.h>

/* impl only */

#include <monlang-interpreter/builtin/prim_ctors.h>
#include <monlang-interpreter/interpret.h>

#include <utils/variant-utils.h>
#include <utils/loop-utils.h>
#include <utils/assert-utils.h>
#include <utils/str-utils.h>

#include <iostream>
#include <iomanip>
#include <vector>
#include <limits>

static void print(const value_t&, std::ostream& = std::cout, bool shouldQuot = false);
static void print(const prim_value_t&, std::ostream&, bool shouldQuot);
static void print(const type_value_t&, std::ostream&);
static void print(const struct_value_t&, std::ostream&);
static void print(const enum_value_t&, std::ostream&);

extern uint64_t builtin_lambda_id; // defined in src/interpret.cpp

const value_t builtin::print __attribute__((init_priority(3000))) = new prim_value_t{prim_value_t::Lambda{
    builtin_lambda_id++,
    IntConst::ZERO,
    [](const std::vector<FlattenArg>& varargs) -> value_t {
        LOOP for (auto arg: varargs) {
            if (!__first_it) {
                std::cout << " ";
            }
            auto argValue = evaluateValue(arg.expr, arg.env);
            ::print(argValue);
            ENDLOOP
        }
        std::cout << std::endl;
        return nil_value_t();
    }
}};

static void print(const value_t& val, std::ostream& out, bool shouldQuot) {
    if (is_nil(val)) {
        out << "$nil";
        return;
    }
    std::visit(overload{
        [&out, shouldQuot](prim_value_t* val){
            print(*val, out, shouldQuot);
        },
        [&out](type_value_t* val){
            print(*val, out);
        },
        [&out](struct_value_t* val){
            print(*val, out);
        },
        [&out](enum_value_t* val){
            print(*val, out);
        },
        [](char*){SHOULD_NOT_HAPPEN();},
    }, val);
}

static std::string quote_str(const std::string& str) {
    return "\"" +
    escape_newlines(
        escape_double_quotes(
            escape_antislashes(str)
        )
    )
    + "\"";
}

static void print(const prim_value_t& primVal, std::ostream& out, bool shouldQuot) {
    std::visit(overload{
        [&out](prim_value_t::Bool bool_){out << (bool_ == true? "$true" : "$false");},
        [&out, shouldQuot](prim_value_t::Byte byte){
            shouldQuot? out << (int)byte : out << byte;
        },
        [&out](prim_value_t::Int int_){out << int_;},
        [&out](prim_value_t::Float float_){out << std::setprecision(std::numeric_limits<double>::digits10) << float_;},
        [&out, shouldQuot](const prim_value_t::Str& str){
            shouldQuot? out << quote_str(str) : out << str;
        },
        [&out](const prim_value_t::List& list){
            out << "[";
            LOOP for (const auto& val: list) {
                if (!__first_it) {
                    out << ", ";
                }
                print(val, out, /*shouldQuot*/true);
                ENDLOOP
            }
            out << "]";
        },
        [&out](const prim_value_t::Map& map){
            if (map.empty()) {
                out << "[:]";
                return;
            }
            out << "[";
            LOOP for (const auto& [key, val]: map) {
                if (!__first_it) {
                    out << ", ";
                }
                print(key, out, /*shouldQuot*/true);
                out << ":";
                print(val, out, /*shouldQuot*/true);
                ENDLOOP
            }
            out << "]";
        },
        [&out](const prim_value_t::Lambda&){out << "<lambda>";},
    }, primVal.variant);
}

static void print(const type_value_t& type_val, std::ostream& out) {
    out << type_val.type << "(";
    print(type_val.value, out);
    out << ")";
}

static void print(const struct_value_t& struct_val, std::ostream& out) {
    out << struct_val.type << "(";
    LOOP for (auto [_field_name, field_value]: struct_val.fields) {
        if (!__first_it) {
            out << ", ";
        }
        print(field_value, out);
        ENDLOOP
    }
    out << ")";
}

static void print(const enum_value_t& enum_val, std::ostream& out) {
    out << enum_val.type << "(";
    print(enum_val.enumerate_value, out);
    out << ")";
}

void builtin::print_(const std::vector<value_t>& varargs, std::ostream& out) {
    LOOP for (auto arg: varargs) {
        if (!__first_it) {
            out << " ";
        }
        ::print(arg, out);
        ENDLOOP
    }
    out << std::endl;
};

void builtin::putstr_(const std::vector<value_t>& varargs, std::ostream& out) {
    LOOP for (auto arg: varargs) {
        if (!__first_it) {
            out << " ";
        }
        ::print(arg, out);
        ENDLOOP
    }
    // out << std::endl;
};
