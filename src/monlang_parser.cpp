#include <monlang-parser/parse.h>
#include <monlang-interpreter/ParseError.h>

LV2::Program parse(const std::string& text, Tokens* tokens) {
    auto res = parse(Source{text});
    if (tokens != nullptr && res._tokensLV2.size() > 0) {
        *tokens = res._tokensLV2;
    }
    try {
        return asCorrectLV2(res);
    }
    catch (const std::bad_variant_access&) {
        throw ParseError{};
    }
}
