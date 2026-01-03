#pragma once

#include <QVector>
#include <QString>

class KMPMatcher
{
public:
    KMPMatcher() = default;
    ~KMPMatcher() = default;

    static QVector<int> search(const QString& text, const QString& pattern);

    // 查找下一个匹配位置
    static int findNext(const QString& text, const QString& pattern, int startPos = 0);

    // 查找上一个匹配位置
    static int findPrev(const QString& text, const QString& pattern, int startPos = -1);
};