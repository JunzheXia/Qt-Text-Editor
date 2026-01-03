#include "fonttextmenu.h"

#include <QAction>
#include <QMenu>
#include <QFontDialog>
#include <QInputDialog>
#include <QTextEdit>
#include <QTextCursor>
#include <QTextCharFormat>
#include <QMainWindow>
#include <QStatusBar>
#include <QMessageBox>
#include <QApplication>
#include <QRegularExpressionValidator>

FontTextMenu::FontTextMenu(QTextEdit* editor, QMainWindow* parentWindow,
    QAction* fontAction, QAction* textSizeAction,
    QMenu* targetMenu)
    : QObject(parentWindow)
    , m_editor(editor)
    , m_parentWindow(parentWindow)
    , m_fontAction(fontAction)
    , m_textSizeAction(textSizeAction)
{
    if (!m_parentWindow || !m_editor) return;

    // 如果没有传入动作，就创建新的
    if (!m_fontAction) {
        m_fontAction = new QAction(tr("&Font..."), this);
        m_fontAction->setStatusTip(tr("Set font (applies to selected text or entire document)"));
    }

    if (!m_textSizeAction) {
        m_textSizeAction = new QAction(tr("&Size..."), this);
        m_textSizeAction->setStatusTip(tr("Set font size (applies to selected text or entire document)"));
    }

    // 连接信号
    connect(m_fontAction, &QAction::triggered, this, &FontTextMenu::onFontTriggered);
    connect(m_textSizeAction, &QAction::triggered, this, &FontTextMenu::onTextSizeTriggered);

    // 添加到菜单
    if (targetMenu) {
        // 检查菜单中是否已存在这些动作
        bool hasFontAction = false;
        bool hasTextSizeAction = false;

        for (QAction* action : targetMenu->actions()) {
            if (action == m_fontAction) hasFontAction = true;
            if (action == m_textSizeAction) hasTextSizeAction = true;
        }

        // 只添加不存在的动作
        if (!hasFontAction) targetMenu->addAction(m_fontAction);
        if (!hasTextSizeAction) targetMenu->addAction(m_textSizeAction);
    }
}

void FontTextMenu::onFontTriggered()
{
    // 获取当前选中文本的字体作为默认值
    QFont initialFont = currentSelectionFont();
    if (!initialFont.family().isEmpty()) {
        // 如果有选中文本，使用选中文本的字体
    }
    else {
        // 否则使用编辑器的默认字体
        initialFont = m_editor->font();
    }

    // 打开字体对话框
    bool ok = false;
    QFont font = QFontDialog::getFont(&ok, initialFont, m_parentWindow,
        tr("Select Font"));

    if (!ok) {
        // 用户取消了对话框
        if (m_parentWindow) {
            m_parentWindow->statusBar()->showMessage(tr("Font selection cancelled"), 1500);
        }
        return;
    }

    // 应用于选中文本；若无选区则应用于全文
    bool hasSelection = m_editor->textCursor().hasSelection();
    setFont(font, hasSelection);

    // 显示状态信息
    if (m_parentWindow) {
        QString message;
        if (hasSelection) {
            message = tr("Font set for selection: %1, %2pt").arg(font.family()).arg(font.pointSize());
        }
        else {
            message = tr("Font set for entire document: %1, %2pt").arg(font.family()).arg(font.pointSize());
        }
        m_parentWindow->statusBar()->showMessage(message, 3000);
    }

    // 发送信号
    emit fontChanged(font);
}

void FontTextMenu::onTextSizeTriggered()
{
    // 获取当前字号
    qreal currentSize = currentSelectionFontSize();
    if (currentSize <= 0) {
        currentSize = m_editor->fontPointSize() > 0 ? m_editor->fontPointSize() : 12.0;
    }

    // 创建自定义输入对话框
    bool ok = false;
    QString sizeText = QInputDialog::getText(m_parentWindow,
        tr("Set Font Size"),
        tr("Font size (pt):"),
        QLineEdit::Normal,
        QString::number(currentSize, 'f', 1),
        &ok);

    if (!ok || sizeText.isEmpty()) {
        if (m_parentWindow) {
            m_parentWindow->statusBar()->showMessage(tr("Font size change cancelled"), 1500);
        }
        return;
    }

    // 验证输入
    bool conversionOk = false;
    qreal size = sizeText.toDouble(&conversionOk);

    if (!conversionOk || size < 6.0 || size > 200.0) {
        QMessageBox::warning(m_parentWindow,
            tr("Invalid Input"),
            tr("Please enter a valid font size between 6.0 and 200.0 pt."));
        return;
    }

    // 应用于选中文本；若无选区则应用于全文
    bool hasSelection = m_editor->textCursor().hasSelection();
    setFontPointSize(size, hasSelection);

    // 显示状态信息
    if (m_parentWindow) {
        QString message;
        if (hasSelection) {
            message = tr("Font size set for selection: %1 pt").arg(size, 0, 'f', 1);
        }
        else {
            message = tr("Font size set for entire document: %1 pt").arg(size, 0, 'f', 1);
        }
        m_parentWindow->statusBar()->showMessage(message, 3000);
    }

    // 发送信号
    emit fontSizeChanged(size);
}

void FontTextMenu::setFont(const QFont& font, bool selectionOnly)
{
    if (!m_editor) return;

    QTextCursor cursor = m_editor->textCursor();

    if (selectionOnly && cursor.hasSelection()) {
        // 仅应用于选中文本
        QTextCharFormat fmt;
        fmt.setFont(font);
        cursor.mergeCharFormat(fmt);
        m_editor->setTextCursor(cursor);
    }
    else {
        // 应用于整个文档
        QTextCursor docCursor(m_editor->document());
        docCursor.select(QTextCursor::Document);
        QTextCharFormat fmt;
        fmt.setFont(font);
        docCursor.mergeCharFormat(fmt);

        // 同时设置编辑器的默认字体（用于后续输入）
        m_editor->setFont(font);
    }
}

void FontTextMenu::setFontPointSize(qreal pointSize, bool selectionOnly)
{
    if (!m_editor) return;

    QTextCursor cursor = m_editor->textCursor();

    if (selectionOnly && cursor.hasSelection()) {
        // 仅应用于选中文本
        QTextCharFormat fmt;
        fmt.setFontPointSize(pointSize);
        cursor.mergeCharFormat(fmt);
        m_editor->setTextCursor(cursor);
    }
    else {
        // 应用于整个文档
        QTextCursor docCursor(m_editor->document());
        docCursor.select(QTextCursor::Document);
        QTextCharFormat fmt;
        fmt.setFontPointSize(pointSize);
        docCursor.mergeCharFormat(fmt);

        // 同时设置编辑器的默认字体大小
        QFont font = m_editor->font();
        font.setPointSizeF(pointSize);
        m_editor->setFont(font);
    }
}

QFont FontTextMenu::currentSelectionFont() const
{
    if (!m_editor) return QFont();

    QTextCursor cursor = m_editor->textCursor();
    if (cursor.hasSelection()) {
        // 获取选中文本的字体格式
        QTextCharFormat format = cursor.charFormat();
        return format.font();
    }

    // 如果没有选中文本，返回编辑器的当前字体
    return m_editor->currentFont();
}

qreal FontTextMenu::currentSelectionFontSize() const
{
    if (!m_editor) return -1;

    QTextCursor cursor = m_editor->textCursor();
    if (cursor.hasSelection()) {
        // 获取选中文本的字体格式
        QTextCharFormat format = cursor.charFormat();
        return format.fontPointSize();
    }

    // 如果没有选中文本，返回编辑器的当前字体大小
    return m_editor->fontPointSize();
}