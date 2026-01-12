#include <monlang-interpreter/PathResolution.h>
#include <monlang-interpreter/interpret.h>
#include <monlang-interpreter/InterpretError.h>
#include <monlang-interpreter/types.h>
#include <monlang-interpreter/builtin.h>
#include <monlang-interpreter/deepcopy.h>

#include <utils/assert-utils.h>
#include <utils/defer-util.h>
#include <utils/vec-utils.h>

#define unless(x) if(!(x))

using Bool = prim_value_t::Bool;
using Byte = prim_value_t::Byte;
using Int = prim_value_t::Int;
using Float = prim_value_t::Float;
using Str = prim_value_t::Str;
using List = prim_value_t::List;
using Map = prim_value_t::Map;

/*
    from now on,

    we use this->evaluate.. to eval basepaths (Lvalue left part)
    with respect with envAtResolution parameter

    and we use ::evaluate.. to eval path values (e.g.: index, range, key)
    with respect with this->pathValuesEnv
*/

value_t PathResolution::value(Environment* envAtResolution) {
    return this->evaluateValue(this->path, envAtResolution);
}

value_t* PathResolution::lvalue(Environment* envAtResolution) {
    return this->evaluateLvalue(this->path, envAtResolution);
}

PathResolution::PathResolution(const Lvalue& path, Environment* pathValuesEnv)
        : path(path), pathValuesEnv(pathValuesEnv){}

//==============================================================
// evaluateValue
//==============================================================

value_t PathResolution::evaluateValue(const Lvalue& lvalue, Environment* envAtResolution) {
    ::activeCallStack.push_back(lvalue);
    defer {safe_pop_back(::activeCallStack);};
    return std::visit(
        [this, envAtResolution](auto* lvaluePtr){
            return this->evaluateValue(*lvaluePtr, envAtResolution);
        }
        , lvalue.variant
    );
}

// see ::evaluateValue(Subscript) in src/interpret.cpp
value_t PathResolution::evaluateValue(const Subscript& subscript, Environment* envAtResolution) {
    defer {this->nthSubscript += 1;};
    auto arrVal = this->evaluateValue(subscript.array, envAtResolution);
    // arrVal = deepcopy(arrVal); // TODO: no need ?
    ASSERT (std::holds_alternative<prim_value_t*>(arrVal)); // TODO: tmp
    auto* arrPrimValPtr = std::get<prim_value_t*>(arrVal);
    if (arrPrimValPtr == nullptr) {
        throw InterpretError("Subscripting a $nil");
    }

    return std::visit(overload{
        [&subscript, this](const Str& str) -> value_t {
            if (std::holds_alternative<Subscript::Key>(subscript.argument)) {
                throw InterpretError("Subscripting a Str with a key");
            }

            else if (std::holds_alternative<Subscript::Index>(subscript.argument)) {
                auto index = std::get<Subscript::Index>(subscript.argument);
                size_t pos;
                ::activeCallStack.push_back(variant_cast(index.nth));
                defer {safe_pop_back(::activeCallStack);};
                if (this->pathValues.size() >= size_t(this->nthSubscript + 1)) {
                    pos = std::any_cast<size_t>(pathValues.at(this->nthSubscript));
                    unless (pos < str.size()) throw InterpretError("Subscript index is out of bounds");
                }
                else {
                    auto nthVal = ::evaluateValue(variant_cast(index.nth), this->pathValuesEnv);
                    //                                                     ^~~~~~~~~~~~~~~~~~~
                    auto intVal = builtin::prim_ctor::Int_(nthVal);
                    unless (intVal != 0) throw InterpretError("Subscript index is zero");
                    unless (abs(intVal) <= str.size()) throw InterpretError("Subscript index is out of bounds");
                    pos = intVal < 0? str.size() - abs(intVal) : size_t(intVal) - 1;

                    this->pathValues.push_back(pos);
                }
                return new prim_value_t{Byte(str.at(pos))};
            }

            else if (std::holds_alternative<Subscript::Range>(subscript.argument)) {
                auto range = std::get<Subscript::Range>(subscript.argument);
                Int fromPos;
                Int toPos;
                if (this->pathValues.size() >= size_t(this->nthSubscript + 1)) {
                    auto pair = std::any_cast<std::pair<Int, Int>>(pathValues.at(this->nthSubscript));

                    fromPos = pair.first;
                    ::activeCallStack.push_back(variant_cast(range.from));
                    unless (fromPos < Int(str.size())) throw InterpretError("Subscript range 'from' is out of bounds");
                    safe_pop_back(::activeCallStack);

                    toPos = pair.second;
                    ::activeCallStack.push_back(variant_cast(range.to));
                    unless (toPos < Int(str.size())) throw InterpretError("Subscript range 'to' is out of bounds");
                    safe_pop_back(::activeCallStack);
                }
                else {
                    /* from */
                    ::activeCallStack.push_back(variant_cast(range.from));
                    auto fromVal = ::evaluateValue(variant_cast(range.from), this->pathValuesEnv);
                    //                                                       ^~~~~~~~~~~~~~~~~~~
                    auto intFromVal = builtin::prim_ctor::Int_(fromVal);
                    unless (intFromVal != 0) throw InterpretError("Subscript range 'from' is zero");
                    fromPos = intFromVal < 0? Int(str.size()) - abs(intFromVal) : intFromVal - 1;
                    unless (0 <= fromPos && fromPos < Int(str.size())) throw InterpretError("Subscript range 'from' is out of bounds");
                    safe_pop_back(::activeCallStack); // variant_cast(range.from)

                    /* to */
                    ::activeCallStack.push_back(variant_cast(range.to));
                    auto toVal = ::evaluateValue(variant_cast(range.to), this->pathValuesEnv);
                    //                                                   ^~~~~~~~~~~~~~~~~~~
                    auto intToVal = builtin::prim_ctor::Int_(toVal);
                    unless (intToVal != 0) throw InterpretError("Subscript range 'to' is zero");
                    toPos = intToVal < 0? Int(str.size()) - abs(intToVal) : intToVal - 1;
                    unless (toPos >= 0) throw InterpretError("Subscript range 'to' is out of bounds");
                    if (range.exclusive) {
                        toPos -= fromPos <= toPos? 1 : -1;
                    }
                    else if (toPos < fromPos) {
                        toPos = fromPos;
                    }
                    unless (toPos < Int(str.size())) throw InterpretError("Subscript range 'to' is out of bounds");
                    safe_pop_back(::activeCallStack); // variant_cast(range.to)

                    this->pathValues.push_back(std::make_pair(fromPos, toPos));
                }
                return new prim_value_t{str.substr(fromPos, toPos - fromPos + 1)};
            }

            else SHOULD_NOT_HAPPEN();
        },

        [&subscript, this](const List& list) -> value_t {
            if (std::holds_alternative<Subscript::Key>(subscript.argument)) {
                throw InterpretError("Subscripting a List with a key");
            }

            else if (std::holds_alternative<Subscript::Index>(subscript.argument)) {
                auto index = std::get<Subscript::Index>(subscript.argument);
                ::activeCallStack.push_back(variant_cast(index.nth));
                defer {safe_pop_back(::activeCallStack);};
                size_t pos;
                if (this->pathValues.size() >= size_t(this->nthSubscript + 1)) {
                    pos = std::any_cast<size_t>(pathValues.at(this->nthSubscript));
                    unless (pos < list.size()) throw InterpretError("Subscript index is out of bounds");
                }
                else {
                    auto nthVal = ::evaluateValue(variant_cast(index.nth), this->pathValuesEnv);
                    //                                                     ^~~~~~~~~~~~~~~~~~~
                    auto intVal = builtin::prim_ctor::Int_(nthVal);
                    unless (intVal != 0) throw InterpretError("Subscript index is zero");
                    unless (abs(intVal) <= list.size()) throw InterpretError("Subscript index is out of bounds");
                    pos = intVal < 0? list.size() - abs(intVal) : size_t(intVal) - 1;

                    this->pathValues.push_back(pos);
                }
                return list.at(pos);
            }

            else if (std::holds_alternative<Subscript::Range>(subscript.argument)) {
                auto range = std::get<Subscript::Range>(subscript.argument);
                Int fromPos;
                Int toPos;
                if (this->pathValues.size() >= size_t(this->nthSubscript + 1)) {
                    auto pair = std::any_cast<std::pair<Int, Int>>(pathValues.at(this->nthSubscript));

                    fromPos = pair.first;
                    ::activeCallStack.push_back(variant_cast(range.from));
                    unless (fromPos < Int(list.size())) throw InterpretError("Subscript range 'from' is out of bounds");
                    safe_pop_back(::activeCallStack);

                    toPos = pair.second;
                    ::activeCallStack.push_back(variant_cast(range.to));
                    unless (toPos < Int(list.size())) throw InterpretError("Subscript range 'to' is out of bounds");
                    safe_pop_back(::activeCallStack);
                }
                else {
                    /* from */
                    ::activeCallStack.push_back(variant_cast(range.from));
                    auto fromVal = ::evaluateValue(variant_cast(range.from), this->pathValuesEnv);
                    //                                                       ^~~~~~~~~~~~~~~~~~~
                    auto intFromVal = builtin::prim_ctor::Int_(fromVal);
                    unless (intFromVal != 0) throw InterpretError("Subscript range 'from' is zero");
                    fromPos = intFromVal < 0? Int(list.size()) - abs(intFromVal) : intFromVal - 1;
                    unless (0 <= fromPos && fromPos < Int(list.size())) throw InterpretError("Subscript range 'from' is out of bounds");
                    safe_pop_back(::activeCallStack); // variant_cast(range.from)

                    /* to */
                    ::activeCallStack.push_back(variant_cast(range.to));
                    auto toVal = ::evaluateValue(variant_cast(range.to), this->pathValuesEnv);
                    //                                                   ^~~~~~~~~~~~~~~~~~~
                    auto intToVal = builtin::prim_ctor::Int_(toVal);
                    unless (intToVal != 0) throw InterpretError("Subscript range 'to' is zero");
                    toPos = intToVal < 0? Int(list.size()) - abs(intToVal) : intToVal - 1;
                    unless (toPos >= 0) throw InterpretError("Subscript range 'to' is out of bounds");
                    if (range.exclusive) {
                        toPos -= fromPos <= toPos? 1 : -1;
                    }
                    else if (toPos < fromPos) {
                        toPos = fromPos;
                    }
                    unless (toPos < Int(list.size())) throw InterpretError("Subscript range 'to' is out of bounds");
                    safe_pop_back(::activeCallStack); // variant_cast(range.to)

                    this->pathValues.push_back(std::make_pair(fromPos, toPos));
                }

                auto res = List(list.begin() + fromPos, list.begin() + + toPos + 1);
                return new prim_value_t{res};
            }

            else SHOULD_NOT_HAPPEN();
        },

        [&subscript, this](const Map& map) -> value_t {
            if (std::holds_alternative<Subscript::Index>(subscript.argument)) {
                throw InterpretError("Subscripting a Map with an index");
            }

            else if (std::holds_alternative<Subscript::Range>(subscript.argument)) {
                throw InterpretError("Subscripting a Map with an range");
            }

            else if (std::holds_alternative<Subscript::Key>(subscript.argument)) {
                auto key = std::get<Subscript::Key>(subscript.argument);
                value_t keyVal;
                if (this->pathValues.size() >= size_t(this->nthSubscript + 1)) {
                    keyVal = std::any_cast<value_t>(pathValues.at(this->nthSubscript));
                }
                else {
                    keyVal = ::evaluateValue(key.expr, this->pathValuesEnv);
                    //                                 ^~~~~~~~~~~~~~~~~~~

                    this->pathValues.push_back(keyVal);
                }
                if (subscript.suffix == '?') {
                    return map.contains(keyVal)? BoolConst::TRUE : BoolConst::FALSE;
                }
                ::activeCallStack.push_back(key.expr);
                unless (map.contains(keyVal)) throw InterpretError("Subscript key not found");
                safe_pop_back(::activeCallStack);
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

// see ::evaluateValue(FieldAccess) in src/interpret.cpp
value_t PathResolution::evaluateValue(const FieldAccess& fieldAccess, Environment* envAtResolution) {
    auto object = this->evaluateValue(fieldAccess.object, envAtResolution);
    //            ^~~~~~                                  ^~~~~~~~~~~~~~~

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

value_t PathResolution::evaluateValue(const Symbol& symbol, Environment* envAtResolution) {
    this->nthSubscript = 0;
    return ::evaluateValue(symbol, envAtResolution);
}

//==============================================================
// evaluateLvalue
//==============================================================

value_t* PathResolution::evaluateLvalue(const Lvalue& lvalue, Environment* envAtResolution, bool subscripted) {
    ::activeCallStack.push_back(lvalue);
    defer {safe_pop_back(::activeCallStack);};
    return std::visit(overload{
        [this, envAtResolution, subscripted](Symbol* symbol){
            return this->evaluateLvalue(*symbol, envAtResolution, subscripted);
        },
        [this, envAtResolution](auto* lvalue){
            return this->evaluateLvalue(*lvalue, envAtResolution);
        },
    }, lvalue.variant);
}

// see ::evaluateLvalue(Subscript) in src/interpret.cpp
value_t* PathResolution::evaluateLvalue(const Subscript& subscript, Environment* envAtResolution) {
    extern value_t SENTINEL_NEW_MAP; // defined in src/interpret.cpp
    defer {this->nthSubscript += 1;};

    if (subscript.suffix == '?') {
        throw InterpretError("lvaluing a subscript[]?");
    }
    auto* lvalue = this->evaluateLvalue(subscript.array, envAtResolution, /*subscripted*/true);
    //             ^~~~~~                                ^~~~~~~~~~~~~~~
    ASSERT (lvalue != nullptr);
    ASSERT (std::holds_alternative<prim_value_t*>(*lvalue)); // TODO: tmp
    auto& lvaluePrimValPtr = std::get<prim_value_t*>(*lvalue);

    if (*lvalue == SENTINEL_NEW_MAP) {
        lvaluePrimValPtr = new prim_value_t{prim_value_t::Map()};
    }

    if (lvaluePrimValPtr == nullptr) {
        throw InterpretError("lvaluing a $nil subscript array");
    }

    if (std::holds_alternative<Subscript::Range>(subscript.argument)) {
        throw InterpretError("lvaluing a subscript range");
    }

    return std::visit(overload{
        [&subscript, this](Str& str) -> value_t* {
            if (std::holds_alternative<Subscript::Key>(subscript.argument)) {
                throw InterpretError("Subscripting a Str with a key");
            }
            else if (std::holds_alternative<Subscript::Index>(subscript.argument)) {
                auto index = std::get<Subscript::Index>(subscript.argument);
                ::activeCallStack.push_back(variant_cast(index.nth));
                defer {safe_pop_back(::activeCallStack);};
                size_t pos;
                if (this->pathValues.size() >= size_t(this->nthSubscript + 1)) {
                    pos = std::any_cast<size_t>(pathValues.at(this->nthSubscript));
                    unless (pos < str.size()) throw InterpretError("Subscript index is out of bounds");
                }
                else {
                    auto index = std::get<Subscript::Index>(subscript.argument);
                    auto nthVal = ::evaluateValue(variant_cast(index.nth), this->pathValuesEnv);
                    //                                                     ^~~~~~~~~~~~~~~~~~~
                    auto intVal = builtin::prim_ctor::Int_(nthVal);
                    unless (intVal != 0) throw InterpretError("Subscript index is zero");
                    unless (abs(intVal) <= str.size()) throw InterpretError("Subscript index is out of bounds");
                    pos = intVal < 0? str.size() - abs(intVal) : size_t(intVal) - 1;

                    this->pathValues.push_back(pos);
                }
                return new value_t{&str[pos]};
            }
            else SHOULD_NOT_HAPPEN();
        },
        [&subscript, this](List& list) -> value_t* {
            if (std::holds_alternative<Subscript::Key>(subscript.argument)) {
                throw InterpretError("Subscripting a List with a key");
            }
            else if (std::holds_alternative<Subscript::Index>(subscript.argument)) {
                auto index = std::get<Subscript::Index>(subscript.argument);
                ::activeCallStack.push_back(variant_cast(index.nth));
                defer {safe_pop_back(::activeCallStack);};
                size_t pos;
                if (this->pathValues.size() >= size_t(this->nthSubscript + 1)) {
                    pos = std::any_cast<size_t>(pathValues.at(this->nthSubscript));
                    unless (pos < list.size()) throw InterpretError("Subscript index is out of bounds");
                }
                else {
                    auto index = std::get<Subscript::Index>(subscript.argument);
                    auto nthVal = ::evaluateValue(variant_cast(index.nth), this->pathValuesEnv);
                    //                                                     ^~~~~~~~~~~~~~~~~~~
                    auto intVal = builtin::prim_ctor::Int_(nthVal);
                    unless (intVal != 0) throw InterpretError("Subscript index is zero");
                    unless (abs(intVal) <= list.size()) throw InterpretError("Subscript index is out of bounds");
                    pos = intVal < 0? list.size() - abs(intVal) : size_t(intVal) - 1;

                    this->pathValues.push_back(pos);
                }
                return &list[pos];
            }
            else SHOULD_NOT_HAPPEN();
        },

        [&subscript, this](Map& map) -> value_t* {
            if (std::holds_alternative<Subscript::Index>(subscript.argument)) {
                throw InterpretError("Subscripting a Map with an index");
            }
            else if (std::holds_alternative<Subscript::Key>(subscript.argument)) {
                auto key = std::get<Subscript::Key>(subscript.argument);
                value_t keyVal;
                if (this->pathValues.size() >= size_t(this->nthSubscript + 1)) {
                    keyVal = std::any_cast<value_t>(pathValues.at(this->nthSubscript));
                }
                else {
                    keyVal = ::evaluateValue(key.expr, this->pathValuesEnv);
                    //                                 ^~~~~~~~~~~~~~~~~~~

                    this->pathValues.push_back(keyVal);
                }
                if (!map.contains(keyVal)) {
                    if (subscript.suffix == '!') {
                        ::activeCallStack.push_back(key.expr);
                        throw InterpretError("Subscript key not found");
                    }
                    map[keyVal] = SENTINEL_NEW_MAP;
                }
                return &map[keyVal];
            }
            else SHOULD_NOT_HAPPEN();
        },

        [](Bool&) -> value_t* {throw InterpretError("Cannot subscript a Bool");},
        [](Byte&) -> value_t* {throw InterpretError("Cannot subscript a Byte");},
        [](Int&) -> value_t* {throw InterpretError("Cannot subscript an Int");},
        [](Float&) -> value_t* {throw InterpretError("Cannot subscript a Float");},
        [](prim_value_t::Lambda&) -> value_t* {throw InterpretError("Cannot subscript a Lambda");},
    }, lvaluePrimValPtr->variant);
}

// see ::evaluateLvalue(FieldAccess) in src/interpret.cpp
value_t* PathResolution::evaluateLvalue(const FieldAccess& fieldAccess, Environment* envAtResolution) {
    auto* lvalue = this->evaluateLvalue(fieldAccess.object, envAtResolution, /*subscripted*/true);
    //             ^~~~~~                                   ^~~~~~~~~~~~~~~
    ASSERT (lvalue != nullptr);

    ASSERT (std::holds_alternative<prim_value_t*>(*lvalue)); // TODO: tmp
    auto* lvaluePrimValPtr = std::get<prim_value_t*>(*lvalue);

    if (lvaluePrimValPtr == nullptr) {
        throw InterpretError("Accessing field on a $nil");
    }

    if (std::holds_alternative<Map>(lvaluePrimValPtr->variant)) {
        auto& map = std::get<Map>(lvaluePrimValPtr->variant);
        auto key = new prim_value_t{(Str)fieldAccess.field.name};
        unless (map.contains(key)) {
            ::activeCallStack.push_back(const_cast<Symbol*>(&fieldAccess.field));
            throw InterpretError("Field not found `" + fieldAccess.field.name + "`");
        }
        return &map[key];
    }

    else throw InterpretError("Accessing field on a non-struct");
}

value_t* PathResolution::evaluateLvalue(const Symbol& symbol, Environment* envAtResolution, bool subscripted) {
    this->nthSubscript = 0;
    return ::evaluateLvalue(symbol, envAtResolution, subscripted);
}
