#ifndef MONLANG_PARSER_H
#define MONLANG_PARSER_H

#include <monlang-LV2/ast/Program.h>
#include <monlang-parser/Tokens.h>

#include <utils/assert-utils.h>

#include <string>

LV2::Program parse(const std::string& text, Tokens* = nullptr);

// basically check if LV1 module is used, if so => ERR
#ifdef AST_COMMON_H
#error "doesn't make sense to both include `monlang_parser` wrapper AND LV1"
#endif

template <typename T>
size_t token_id(const T& entity) {
    return entity._tokenId;
}

template <>
size_t token_id(const DoWhileStatement&) {
    SHOULD_NOT_HAPPEN();
}

template <typename T>
size_t token_id(T* entity) {
    return entity->_tokenId;
}

template <>
size_t token_id(DoWhileStatement*) {
    SHOULD_NOT_HAPPEN(); // TODO: tmp
}

template <typename... Targs>
size_t token_id(const std::variant<Targs...>& entityVariant) {
    return std::visit(
        [](const auto& entity){return token_id(entity);},
        entityVariant
    );
}

#endif // MONLANG_PARSER_H
