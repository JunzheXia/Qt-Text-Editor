#include "FindReplaceController.h"
#include "KMPMatcher.h"

#include <QTextEdit>
#include <QMainWindow>
#include <QInputDialog>
#include <QMessageBox>
#include <QTextCursor>
#include <QPushButton>
#include <QLineEdit>
#include <QStatusBar>

FindReplaceController::FindReplaceController(QTextEdit* editor, QMainWindow* parentWindow)
    : QObject(parentWindow)
    , m_editor(editor)
    , m_parentWindow(parentWindow)
    , m_currentMatch(-1)
{
}

FindReplaceController::~FindReplaceController() = default;

void FindReplaceController::showStatus(const QString& message, int timeout)
{
    if (m_parentWindow && m_parentWindow->statusBar()) {
        m_parentWindow->statusBar()->showMessage(message, timeout);
    }
}

void FindReplaceController::updateMatches()
{
    if (!m_editor || m_lastPattern.isEmpty()) {
        m_matches.clear();
        m_currentMatch = -1;
        return;
    }

    QString text = m_editor->toPlainText();
    m_matches = KMPMatcher::search(text, m_lastPattern);
    m_currentMatch = m_matches.isEmpty() ? -1 : 0;
}

void FindReplaceController::highlightMatch(int matchIndex)
{
    if (!m_editor || matchIndex < 0 || matchIndex >= m_matches.size())
        return;

    int pos = m_matches[matchIndex];
    int patternLen = m_lastPattern.size();

    QTextCursor cursor = m_editor->textCursor();
    cursor.setPosition(pos);
    cursor.setPosition(pos + patternLen, QTextCursor::KeepAnchor);
    m_editor->setTextCursor(cursor);
    m_editor->setFocus();

    showStatus(tr("匹配 %1 / %2").arg(matchIndex + 1).arg(m_matches.size()));
}

void FindReplaceController::find()
{
    if (!m_editor)
        return;

    showStatus(tr("提示：查找后使用 F3 查找下一个，Shift+F3 查找上一个"), 4000);

    bool ok = false;
    QString patternStr = QInputDialog::getText(m_parentWindow,
        tr("查找"),
        tr("输入查找字符串："),
        QLineEdit::Normal, m_lastPattern, &ok);

    if (!ok || patternStr.isEmpty())
        return;

    m_lastPattern = patternStr;
    updateMatches();

    if (m_matches.isEmpty()) {
        QMessageBox::information(m_parentWindow, tr("查找"), tr("未找到匹配项"));
        return;
    }

    highlightMatch(m_currentMatch);
}

void FindReplaceController::findNext()
{
    if (m_matches.isEmpty()) {
        showStatus(tr("无匹配项，先执行查找"), 2000);
        return;
    }

    m_currentMatch = (m_currentMatch + 1) % m_matches.size();
    highlightMatch(m_currentMatch);
}

void FindReplaceController::findPrev()
{
    if (m_matches.isEmpty()) {
        showStatus(tr("无匹配项，先执行查找"), 2000);
        return;
    }

    m_currentMatch = (m_currentMatch - 1 + m_matches.size()) % m_matches.size();
    highlightMatch(m_currentMatch);
}

void FindReplaceController::replace()
{
    if (!m_editor)
        return;

    showStatus(tr("提示：F3 查找下一个，F4 替换下一个，Shift+F3 查找上一个，Shift+F4 替换上一个"), 5000);

    // 获取查找字符串
    bool ok = false;
    QString patternStr = QInputDialog::getText(m_parentWindow,
        tr("替换 - 查找"),
        tr("输入查找字符串："),
        QLineEdit::Normal, m_lastPattern, &ok);

    if (!ok || patternStr.isEmpty())
        return;

    // 获取替换字符串
    QString replaceStr = QInputDialog::getText(m_parentWindow,
        tr("替换 - 替换为"),
        tr("输入替换字符串："),
        QLineEdit::Normal, m_lastReplace, &ok);

    if (!ok)
        return;

    m_lastPattern = patternStr;
    m_lastReplace = replaceStr;
    updateMatches();

    if (m_matches.isEmpty()) {
        QMessageBox::information(m_parentWindow, tr("替换"), tr("未找到匹配项"));
        return;
    }

    // 选择替换方式
    QMessageBox msgBox(m_parentWindow);
    msgBox.setWindowTitle(tr("替换"));
    msgBox.setText(tr("找到 %1 个匹配，选择替换方式：").arg(m_matches.size()));

    QPushButton* replaceOneBtn = msgBox.addButton(tr("替换当前"), QMessageBox::AcceptRole);
    QPushButton* replaceAllBtn = msgBox.addButton(tr("替换全部"), QMessageBox::AcceptRole);
    msgBox.addButton(QMessageBox::Cancel);

    msgBox.exec();

    if (msgBox.clickedButton() == replaceOneBtn) {
        // 替换当前（第一个匹配）
        replaceAtIndex(0, m_lastReplace);
        if (!m_matches.isEmpty()) {
            highlightMatch(0);
        }
        showStatus(tr("已替换当前匹配"));
    }
    else if (msgBox.clickedButton() == replaceAllBtn) {
        // 替换全部（从后往前替换）
        for (int i = m_matches.size() - 1; i >= 0; --i) {
            replaceAtIndex(i, m_lastReplace);
        }
        showStatus(tr("已替换全部 %1 个匹配").arg(m_matches.size()));
    }
}

void FindReplaceController::replaceNext()
{
    if (m_lastPattern.isEmpty() || m_lastReplace.isEmpty()) {
        showStatus(tr("请先使用替换对话设置查找和替换文本"), 3000);
        return;
    }

    if (m_matches.isEmpty()) {
        showStatus(tr("无匹配项，先执行查找/替换"), 2000);
        return;
    }

    // 找到当前光标位置之后的第一个匹配
    QTextCursor cursor = m_editor->textCursor();
    int cursorPos = cursor.position();

    int nextIndex = -1;
    for (int i = 0; i < m_matches.size(); ++i) {
        if (m_matches[i] >= cursorPos) {
            nextIndex = i;
            break;
        }
    }

    // 如果没找到后面的，循环到第一个
    if (nextIndex == -1 && !m_matches.isEmpty()) {
        nextIndex = 0;
    }

    if (nextIndex != -1) {
        replaceAtIndex(nextIndex, m_lastReplace);
        m_currentMatch = nextIndex;

        // 高亮下一个可用的匹配
        if (!m_matches.isEmpty()) {
            highlightMatch(m_currentMatch);
        }
        showStatus(tr("已替换下一个匹配"));
    }
}

void FindReplaceController::replacePrev()
{
    if (m_lastPattern.isEmpty() || m_lastReplace.isEmpty()) {
        showStatus(tr("请先使用替换对话设置查找和替换文本"), 3000);
        return;
    }

    if (m_matches.isEmpty()) {
        showStatus(tr("无匹配项，先执行查找/替换"), 2000);
        return;
    }

    // 找到当前光标位置之前的最后一个匹配
    QTextCursor cursor = m_editor->textCursor();
    int cursorPos = cursor.position();

    int prevIndex = -1;
    for (int i = m_matches.size() - 1; i >= 0; --i) {
        if (m_matches[i] < cursorPos) {
            prevIndex = i;
            break;
        }
    }

    // 如果没找到前面的，循环到最后一个
    if (prevIndex == -1 && !m_matches.isEmpty()) {
        prevIndex = m_matches.size() - 1;
    }

    if (prevIndex != -1) {
        replaceAtIndex(prevIndex, m_lastReplace);
        m_currentMatch = prevIndex;

        // 高亮上一个可用的匹配
        if (!m_matches.isEmpty()) {
            highlightMatch(m_currentMatch);
        }
        showStatus(tr("已替换上一个匹配"));
    }
}

void FindReplaceController::deleteAllMatches()
{
    if (!m_editor)
        return;

    // 如果没有已知模式，询问用户输入
    if (m_lastPattern.isEmpty()) {
        bool ok = false;
        QString pattern = QInputDialog::getText(m_parentWindow,
            tr("删除匹配"),
            tr("输入要删除的字符串："),
            QLineEdit::Normal, QString(), &ok);

        if (!ok || pattern.isEmpty())
            return;

        m_lastPattern = pattern;
    }

    updateMatches();

    if (m_matches.isEmpty()) {
        showStatus(tr("未找到匹配项，未删除"), 2000);
        return;
    }

    // 从后向前删除
    for (int i = m_matches.size() - 1; i >= 0; --i) {
        int pos = m_matches[i];
        int len = m_lastPattern.size();

        QTextCursor cursor(m_editor->document());
        cursor.setPosition(pos);
        cursor.setPosition(pos + len, QTextCursor::KeepAnchor);
        cursor.removeSelectedText();
    }

    // 清理状态
    m_matches.clear();
    m_currentMatch = -1;
    emit requestUpdate();

    showStatus(tr("已删除全部 %1 个匹配").arg(m_lastPattern), 3000);
}

bool FindReplaceController::replaceAtIndex(int index, const QString& replaceStr)
{
    if (!m_editor || index < 0 || index >= m_matches.size())
        return false;

    int pos = m_matches[index];
    int patternLen = m_lastPattern.size();

    QTextCursor cursor(m_editor->document());
    cursor.setPosition(pos);
    cursor.setPosition(pos + patternLen, QTextCursor::KeepAnchor);
    cursor.insertText(replaceStr);

    // 替换后更新匹配列表
    updateMatches();
    emit requestUpdate();

    return true;
}