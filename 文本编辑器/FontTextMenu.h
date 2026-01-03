#pragma once

#include <QObject>
#include <QTextEdit>
#include <QMainWindow>

class QAction;
class QMenu;

class FontTextMenu : public QObject
{
    Q_OBJECT

public:
    explicit FontTextMenu(QTextEdit* editor, QMainWindow* parentWindow,
        QAction* fontAction = nullptr,
        QAction* textSizeAction = nullptr,
        QMenu* targetMenu = nullptr);

    ~FontTextMenu() override = default;

    QAction* fontAction() const { return m_fontAction; }

    QAction* textSizeAction() const { return m_textSizeAction; }

    void setFont(const QFont& font, bool selectionOnly = true);

    void setFontPointSize(qreal pointSize, bool selectionOnly = true);

    QFont currentSelectionFont() const;

    qreal currentSelectionFontSize() const;

signals:
    void fontChanged(const QFont& font);

    void fontSizeChanged(qreal size);

private slots:
    void onFontTriggered();

    void onTextSizeTriggered();

private:
    QTextEdit* m_editor;
    QMainWindow* m_parentWindow;
    QAction* m_fontAction;    // 使用传入的，而不是新建
    QAction* m_textSizeAction; // 使用传入的，而不是新建
};

