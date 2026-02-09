#include <monlang-interpreter/interpret.h>
#include <monlang-interpreter/InterpretError.h>
#include <monlang-interpreter/builtin.h>

#include <utils/variant-utils.h>
#include <utils/assert-utils.h>
#include <utils/defer-util.h>
#include <utils/vec-utils.h>
#include <utils/abs63.h>
#include <utils/min0.h>

#define unless(x) if(!(x))

using Bool = prim_value_t::Bool;
using Byte = prim_value_t::Byte;
using Int = prim_value_t::Int;
using Float = prim_value_t::Float;
using Str = prim_value_t::Str;
using List = prim_value_t::List;
using Map = prim_value_t::Map;
namespace LV2 {using Lambda = Lambda;}

value_t createPaths(const Lvalue& lvalue, Environment* env) {
    return std::visit(overload{
        [env](Symbol* symbol){return evaluateValue(*symbol, env);},
        [env](auto* lvalue){return createPaths(*lvalue, env);},
    }, lvalue.variant);
}

// see ::evaluateValue(FieldAccess) in src/interpret.cpp
value_t createPaths(const FieldAccess& fieldAccess, Environment* env) {
    auto object = createPaths(fieldAccess.object, env);
    //            ^~~~~~~~~~~
    ASSERT (std::holds_alternative<prim_value_t*>(object)); // TODO: tmp
    auto* objPrimValPtr = std::get<prim_value_t*>(object);

    if (objPrimValPtr == nullptr) {
        throw InterpretError("Accessing field on a $nil");
    }

    if (std::holds_alternative<Map>(objPrimValPtr->variant)) {
        const auto& map = std::get<Map>(objPrimValPtr->variant);
        auto key = new prim_value_t{(Str)fieldAccess.field.name};
        unless (map.contains(key)) {
            ::activeCallStack.push_back(const_cast<Symbol*>(&fieldAccess.field));
            throw InterpretError("Field not found `" + fieldAccess.field.name + "`");
        }
        return map.at(key);
    }

    else throw InterpretError("Accessing field on a non-struct");
}

// see ::evaluateValue(Subscript) in src/interpret.cpp
value_t createPaths(const Subscript& subscript, Environment* env) {
    extern value_t SENTINEL_NEW_MAP; // defined in src/interpret.cpp //
    auto arrVal = createPaths(subscript.array, env);
    //            ^~~~~~~~~~~
    // arrVal = deepcopy(arrVal); // TODO: no need ?

    if (arrVal == SENTINEL_NEW_MAP) { //
        arrVal = new prim_value_t{prim_value_t::Map()}; //
    } //

    ASSERT (std::holds_alternative<prim_value_t*>(arrVal)); // TODO: tmp
    auto* arrPrimValPtr = std::get<prim_value_t*>(arrVal);

    if (arrPrimValPtr == nullptr) {
        throw InterpretError("Subscripting a $nil");
    }

    return std::visit(overload{
        [&subscript, env](const Str& str) -> value_t {
            if (std::holds_alternative<Subscript::Key>(subscript.argument)) {
                throw InterpretError("Subscripting a Str with a key");
            }

            else if (std::holds_alternative<Subscript::Index>(subscript.argument)) {
                auto index = std::get<Subscript::Index>(subscript.argument);
                auto nthVal = evaluateValue(variant_cast(index.nth), env);
                ::activeCallStack.push_back(variant_cast(index.nth));
                defer {safe_pop_back(::activeCallStack);};
                auto intVal = builtin::prim_ctor::Int_(nthVal);

                unless (intVal != 0) throw InterpretError("Subscript index is zero");
                unless (abs63(intVal) <= abs63(str.size())) throw InterpretError("Subscript index is out of bounds");
                auto pos = intVal < 0? str.size() - abs63(intVal) : size_t(intVal) - 1;
                return new prim_value_t{Byte(str.at(pos))};
            }

            else if (std::holds_alternative<Subscript::Range>(subscript.argument)) {
                auto range = std::get<Subscript::Range>(subscript.argument);
                abs63(str.size()); // make sure it's storable in a Int

                /* from */
                ::activeCallStack.push_back(variant_cast(range.from));
                auto fromVal = evaluateValue(variant_cast(range.from), env);
                auto intFromVal = builtin::prim_ctor::Int_(fromVal);
                unless (intFromVal != 0) throw InterpretError("Subscript range 'from' is zero");
                safe_pop_back(::activeCallStack);

                /* to */
                ::activeCallStack.push_back(variant_cast(range.to));
                auto toVal = evaluateValue(variant_cast(range.to), env);
                auto intToVal = builtin::prim_ctor::Int_(toVal);
                unless (intToVal != 0 || range.exclusive) throw InterpretError("Subscript range 'to' is zero");
                safe_pop_back(::activeCallStack);

                /* 1) handle exclusive range, if present */
                if (range.exclusive) {
                    if (intFromVal == intToVal) {
                        return new prim_value_t{Str()}; // empty range
                    }
                    else {
                        Int fromPos = intFromVal < 0? str.size() - abs63(intFromVal) : intFromVal - 1;
                        Int toPos = intToVal == 0? (intFromVal > 0? -1 : LLONG_MAX) : intToVal < 0? str.size() - abs63(intToVal) : intToVal - 1;
                        if (fromPos < toPos || toPos == LLONG_MAX) {
                            intToVal -= 1;
                        }
                        else if (fromPos > toPos) {
                            intToVal += 1;
                        }
                    }
                }

                /* 2) transform to pos, with index starting at zero */
                Int fromPos = intFromVal <= 0? str.size() - abs63(intFromVal) : intFromVal - 1;
                Int toPos = intToVal < 0? str.size() - abs63(intToVal) : intToVal - 1;

                /* 3) check out of bounds */
                unless (0 <= fromPos && size_t(fromPos) < str.size()) {
                    ::activeCallStack.push_back(variant_cast(range.from));
                    throw InterpretError("Subscript range 'from' is out of bounds");
                }
                unless (0 <= toPos && size_t(toPos) < str.size()) {
                    ::activeCallStack.push_back(variant_cast(range.from));
                    throw InterpretError("Subscript range 'to' is out of bounds");
                }

                return new prim_value_t{str.substr(fromPos, min0(toPos - fromPos) + 1)};
            }

            else SHOULD_NOT_HAPPEN();
        },

        [&subscript, env](const List& list) -> value_t {
            if (std::holds_alternative<Subscript::Key>(subscript.argument)) {
                throw InterpretError("Subscripting a List with a key");
            }

            else if (std::holds_alternative<Subscript::Index>(subscript.argument)) {
                auto index = std::get<Subscript::Index>(subscript.argument);
                auto nthVal = evaluateValue(variant_cast(index.nth), env);
                ::activeCallStack.push_back(variant_cast(index.nth));
                defer {safe_pop_back(::activeCallStack);};
                auto intVal = builtin::prim_ctor::Int_(nthVal);

                unless (intVal != 0) throw InterpretError("Subscript index is zero");
                unless (abs63(intVal) <= abs63(list.size())) throw InterpretError("Subscript index is out of bounds");
                auto pos = intVal < 0? list.size() - abs63(intVal) : size_t(intVal) - 1;
                return list.at(pos);
            }

            else if (std::holds_alternative<Subscript::Range>(subscript.argument)) {
                auto range = std::get<Subscript::Range>(subscript.argument);
                abs63(list.size()); // make sure it's storable in a Int

                /* from */
                ::activeCallStack.push_back(variant_cast(range.from));
                auto fromVal = evaluateValue(variant_cast(range.from), env);
                auto intFromVal = builtin::prim_ctor::Int_(fromVal);
                unless (intFromVal != 0) throw InterpretError("Subscript range 'from' is zero");
                safe_pop_back(::activeCallStack);

                /* to */
                ::activeCallStack.push_back(variant_cast(range.to));
                auto toVal = evaluateValue(variant_cast(range.to), env);
                auto intToVal = builtin::prim_ctor::Int_(toVal);
                unless (intToVal != 0 || range.exclusive) throw InterpretError("Subscript range 'to' is zero");
                safe_pop_back(::activeCallStack);

                /* 1) handle exclusive range, if present */
                if (range.exclusive) {
                    if (intFromVal == intToVal) {
                        return new prim_value_t{List()}; // empty range
                    }
                    else {
                        Int fromPos = intFromVal < 0? list.size() - abs63(intFromVal) : intFromVal - 1;
                        Int toPos = intToVal == 0? (intFromVal > 0? -1 : LLONG_MAX) : intToVal < 0? list.size() - abs63(intToVal) : intToVal - 1;
                        if (fromPos < toPos || toPos == LLONG_MAX) {
                            intToVal -= 1;
                        }
                        else if (fromPos > toPos) {
                            intToVal += 1;
                        }
                    }
                }

                /* 2) transform to pos, with index starting at zero */
                Int fromPos = intFromVal <= 0? list.size() - abs63(intFromVal) : intFromVal - 1;
                Int toPos = intToVal < 0? list.size() - abs63(intToVal) : intToVal - 1;

                /* 3) check out of bounds */
                unless (0 <= fromPos && size_t(fromPos) < list.size()) {
                    ::activeCallStack.push_back(variant_cast(range.from));
                    throw InterpretError("Subscript range 'from' is out of bounds");
                }
                unless (0 <= toPos && size_t(toPos) < list.size()) {
                    ::activeCallStack.push_back(variant_cast(range.from));
                    throw InterpretError("Subscript range 'to' is out of bounds");
                }

                auto res = List(list.begin() + fromPos, list.begin() + std::max(fromPos, toPos) + 1);
                return new prim_value_t{res};
            }

            else SHOULD_NOT_HAPPEN();
        },

        [&subscript, env](Map& map) -> value_t {
        //                ^~~~ no const
            extern value_t SENTINEL_NEW_MAP; // defined in src/interpret.cpp //
            if (std::holds_alternative<Subscript::Index>(subscript.argument)) {
                throw InterpretError("Subscripting a Map with an index");
            }

            else if (std::holds_alternative<Subscript::Range>(subscript.argument)) {
                throw InterpretError("Subscripting a Map with an range");
            }

            else if (std::holds_alternative<Subscript::Key>(subscript.argument)) {
                auto key = std::get<Subscript::Key>(subscript.argument);
                auto keyVal = evaluateValue(key.expr, env);
                if (subscript.suffix == '?') {
                    return map.contains(keyVal)? BoolConst::TRUE : BoolConst::FALSE;
                }
                // ::activeCallStack.push_back(key.expr);
                // unless (map.contains(keyVal)) throw InterpretError("Subscript key not found");
                // safe_pop_back(::activeCallStack);
                if (!map.contains(keyVal)) { //
                    map[keyVal] = SENTINEL_NEW_MAP; //
                } //
                return map.at(keyVal);
            }

            else SHOULD_NOT_HAPPEN();
        },

        [](Bool) -> value_t {throw InterpretError("Cannot subscript a Bool");},
        [](Byte) -> value_t {throw InterpretError("Cannot subscript a Byte");},
        [](Int) -> value_t {throw InterpretError("Cannot subscript an Int");},
        [](Float) -> value_t {throw InterpretError("Cannot subscript a Float");},
        [](const prim_value_t::Lambda&) -> value_t {throw InterpretError("Cannot subscript a Lambda");},
    }, arrPrimValPtr->variant);
}
