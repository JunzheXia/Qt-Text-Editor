#include "StringProcessor.h"
#include <QChar>

StringProcessor::Result StringProcessor::process(const QString& text) const
{
    Result r;

    for (const QChar& ch : text) {
        // 跳过换行符和回车符
        if (ch == '\n' || ch == '\r') continue;

        ++r.total;

        if (isChineseChar(ch)) {
            ++r.chinese;
        }
        else if (ch.isLetter()) {
            ++r.letters;
        }
        else if (ch.isDigit()) {
            ++r.digits;
        }
        else if (!ch.isSpace()) {
            ++r.symbols;
        }
    }

    return r;
}

bool StringProcessor::isChineseChar(QChar ch) const
{
    uint uc = ch.unicode();
    return (uc >= 0x4E00 && uc <= 0x9FFF) ||    // 基本CJK
        (uc >= 0x3400 && uc <= 0x4DBF) ||    // 扩展A
        (uc >= 0x20000 && uc <= 0x2A6DF) ||  // 扩展B
        (uc >= 0x2A700 && uc <= 0x2B73F) ||  // 扩展C
        (uc >= 0x2B740 && uc <= 0x2B81F) ||  // 扩展D
        (uc >= 0x2B820 && uc <= 0x2CEAF);    // 扩展E
}