#include <monlang-parser/parse.h>

LV2::Program parse(const std::string& text) {
    auto res = parse(Source{text});
    return asCorrectLV2(res);
}
