#pragma once


#include <QObject>
#include <QString>

class QTextEdit;
class QMainWindow;

class FileManager : public QObject
{
    Q_OBJECT

public:
    explicit FileManager(QTextEdit* editor, QMainWindow* parentWindow);
    ~FileManager() override = default;

    bool newFile();

    bool openFile(const QString& fileName = QString());

    bool save();

    bool saveAs();

    QString currentFileName() const { return m_currentFile; }

    QString displayFileName() const;

    bool isModified() const;

    void updateWindowTitle();

    void showStatusMessage(const QString& message, int timeout = 3000);

    bool loadFile(const QString& fileName);

    void setCurrentFile(const QString& fileName);

signals:
    void fileLoaded();

    void fileSaved();

    void modificationChanged(bool modified);

    void requestUpdateStats();

private:
    bool maybeSave();

    bool saveToFile(const QString& fileName);

private:
    QTextEdit* m_editor;          // 文本编辑器
    QMainWindow* m_parentWindow;  // 父窗口
    QString m_currentFile;        // 当前文件名

    static const QStringList SUPPORTED_FORMATS;  // 支持的文件格式
};

