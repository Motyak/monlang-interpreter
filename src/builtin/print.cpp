#include <monlang-interpreter/builtin/print.h>

/* impl only */

#include <monlang-interpreter/builtin/prim_ctors.h>
#include <monlang-interpreter/interpret.h>
#include <monlang-interpreter/builtin/operators.h>

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
static void print(const type_value_t&, std::ostream&, bool shouldQuot);
static void print(const struct_value_t&, std::ostream&);
static void print(const enum_value_t&, std::ostream&, bool shouldQuot);

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
        [&out, shouldQuot](type_value_t* val){
            print(*val, out, shouldQuot);
        },
        [&out, shouldQuot](struct_value_t* val){
            print(*val, out);
        },
        [&out, shouldQuot](enum_value_t* val){
            print(*val, out, shouldQuot);
        },
        [](char*){SHOULD_NOT_HAPPEN();},
        [](FieldLvalue*){SHOULD_NOT_HAPPEN();},
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

static void print(const type_value_t& type_val, std::ostream& out, bool shouldQuote) {
    if (builtin::op::is_(type_val.typeTag, "Str")) {
        auto underlyingVal = rec_unwrap_typeval(type_val.underlyingVal);
        print(underlyingVal, out, shouldQuote);
    }
    else if (builtin::op::is_(type_val.typeTag, "List")) {
        auto underlyingVal = rec_unwrap_typeval(type_val.underlyingVal);
        ASSERT (std::holds_alternative<prim_value_t*>(underlyingVal));
        auto primVal = *std::get<prim_value_t*>(underlyingVal);
        ASSERT (std::holds_alternative<prim_value_t::List>(primVal.variant));
        auto list = std::get<prim_value_t::List>(primVal.variant);
        out << type_val.typeTag << "(";
        bool first_it = true;
        for (const auto& val: list) {
            if (!first_it) {
                out << ", ";
            }
            print(val, out, /*shouldQuot*/true);
            first_it = false;
        }
        out << ")";
    }
    else if (builtin::op::is_(type_val.typeTag, "Map")) {
        auto underlyingVal = rec_unwrap_typeval(type_val.underlyingVal);
        ASSERT (std::holds_alternative<prim_value_t*>(underlyingVal));
        auto primVal = *std::get<prim_value_t*>(underlyingVal);
        ASSERT (std::holds_alternative<prim_value_t::Map>(primVal.variant));
        auto map = std::get<prim_value_t::Map>(primVal.variant);
        out << type_val.typeTag << "(";
        bool first_it = true;
        for (const auto& [key, val]: map) {
            if (!first_it) {
                out << ", ";
            }
            out << "[";
            print(key, out, /*shouldQuot*/true);
            out << ", ";
            print(val, out, /*shouldQuot*/true);
            out << "]";
            first_it = false;
        }
        out << ")";
    }
    else {
        // if unwrapped typeval is a struct, then we directly
        // ..print the struct content while keeping the type tag
        auto unwrapped = rec_unwrap_typeval(type_val.underlyingVal);
        if (std::holds_alternative<struct_value_t*>(unwrapped)) {
            auto struct_val = *std::get<struct_value_t*>(unwrapped);
            out << type_val.typeTag << "(";
            LOOP for (auto [_type, _name, field_value]: struct_val.fields) {
                if (!__first_it) {
                    out << ", ";
                }
                print(field_value, out, /*shouldQuot*/true);
                ENDLOOP
            }
            out << ")";
        }
        else {
            /* ignore unwrapped */
            out << type_val.typeTag << "(";
            print(type_val.underlyingVal, out, /*shouldQuot*/true);
            out << ")";
        }
    }
}

static void print(const struct_value_t& struct_val, std::ostream& out) {
    out << struct_val.type << "(";
    LOOP for (auto [_type, _name, field_value]: struct_val.fields) {
        if (!__first_it) {
            out << ", ";
        }
        print(field_value, out, /*shouldQuot*/true);
        ENDLOOP
    }
    out << ")";
}

static void print(const enum_value_t& enum_val, std::ostream& out, bool shouldQuote) {
    if (builtin::op::is_(enum_val.type, "Str")) {
        auto unwrapped = rec_unwrap_typeval(enum_val.enumerate);
        print(unwrapped, out, shouldQuote);
    }
    else {
        out << enum_val.type << "::" << enum_val.enumerator;
    }
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
