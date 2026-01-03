#include "KMPMatcher.h"

QVector<int> KMPMatcher::search(const QString& text, const QString& pattern)
{
    QVector<int> matches;
    const int n = text.size();
    const int m = pattern.size();

    if (m == 0 || n == 0 || m > n) {
        return matches;
    }

    // 构建 KMP 数组
    QString t = "@" + pattern;  // 哨兵，索引从1开始
    QVector<int> kmp(m + 1, 0);

    for (int i = 2, j = 0; i <= m; ++i) {
        while (j && t[i] != t[j + 1]) {
            j = kmp[j];
        }
        j += (t[i] == t[j + 1]);
        kmp[i] = j;
    }

    // KMP 匹配
    QString s = "@" + text;
    for (int i = 1, j = 0; i <= n; ++i) {
        while (j && s[i] != t[j + 1]) {
            j = kmp[j];
        }
        if (s[i] == t[j + 1]) {
            ++j;
        }
        if (j == m) {
            matches.append(i - m);  // 0-bs
            j = kmp[j];
        }
    }

    return matches;
}

int KMPMatcher::findNext(const QString& text, const QString& pattern, int startPos)
{
    if (startPos < 0) startPos = 0;
    if (startPos >= text.size()) return -1;

    QString subText = text.mid(startPos);
    QVector<int> matches = search(subText, pattern);

    if (!matches.isEmpty()) {
        return startPos + matches.first();
    }
    return -1;
}

int KMPMatcher::findPrev(const QString& text, const QString& pattern, int startPos)
{
    if (startPos < 0 || startPos > text.size()) {
        startPos = text.size();
    }

    QString subText = text.left(startPos);
    QVector<int> matches = search(subText, pattern);

    if (!matches.isEmpty()) {
        return matches.last();
    }
    return -1;
}