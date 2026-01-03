#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_QtWidgetsApplication.h"
#include <QString>

class QTextEdit;
class QLabel;
class StringProcessor;
class FileManager;           
class FindReplaceController;
class FontTextMenu;
class QAction;
class QShortcut;

class QtWidgetsApplication : public QMainWindow
{
    Q_OBJECT

public:
    QtWidgetsApplication(QWidget* parent = nullptr);
    ~QtWidgetsApplication();

private slots:
    void on_NewFile_triggered();
    void on_OpenFile_triggered();
    void on_SaveFile_triggered();

    void on_Find_triggered();
    void on_Replace_triggered();
    void on_Delete_triggered();

    void updateStats();

private:
    // 初始化函数
    void initEditor();
    void initActions();
    void initStats();
    void initControllers();
    void initShortcuts();
    void initFontMenu();

    // 辅助函数
    void showTemporaryHint(const QString& hint, int timeout);

private:
    Ui::QtWidgetsApplicationClass ui;
    QTextEdit* m_editor;

    // 控制器
    FileManager* m_fileManager;          
    StringProcessor* m_processor;
    FindReplaceController* m_findController;
    FontTextMenu* m_fontController;

    // 界面组件
    QLabel* m_statsLabel;

    // 动作（从UI获取）
    QAction* m_findAction;
    QAction* m_replaceAction;
    QAction* m_deleteAction;
    QAction* m_fontAction;
    QAction* m_textSizeAction;

    // 快捷键
    QShortcut* m_shortcutFindNext;
    QShortcut* m_shortcutFindPrev;
    QShortcut* m_shortcutReplaceNext;
    QShortcut* m_shortcutReplacePrev;
};