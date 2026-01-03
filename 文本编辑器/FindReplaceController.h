#pragma once

#include <QObject>
#include <QString>

class QTextEdit;
class QMainWindow;

class FindReplaceController : public QObject
{
    Q_OBJECT
public:
    explicit FindReplaceController(QTextEdit* editor, QMainWindow* parentWindow = nullptr);
    ~FindReplaceController() override;

public slots:
    void find();            // 弹出查找对话并定位第一个匹配
    void replace();         // 弹出替换对话，可替换当前或全部
    void findNext();        // 查找下一个（F3）
    void findPrev();        // 查找上一个 (Shift+F3)
    void replaceNext();     // 替换下一个（F4）
    void replacePrev();     // 替换上一个 (Shift+F4)
    void deleteAllMatches(); // 直接删除所有匹配

signals:
    void requestUpdate();

private:
    // 高亮匹配项
    void highlightMatch(int matchIndex);
    // 在指定位置替换
    bool replaceAtIndex(int index, const QString& replaceStr);
    // 更新匹配结果
    void updateMatches();
    // 显示状态消息
    void showStatus(const QString& message, int timeout = 2000);

private:
    QTextEdit* m_editor;
    QMainWindow* m_parentWindow;

    QString m_lastPattern;   // 最后一次查找的字符串
    QString m_lastReplace;   // 最后一次替换的字符串
    QVector<int> m_matches;  // 匹配位置列表（在文本中的位置，0-based）
    int m_currentMatch;      // 当前选中的匹配索引
};