#pragma once

#include <QString>

class StringProcessor
{
public:
    struct Result {
        int total = 0;
        int chinese = 0;
        int letters = 0;
        int digits = 0;
        int symbols = 0;
    };

    Result process(const QString& text) const;

private:
    bool isChineseChar(QChar ch) const;
};