#include "FileManager.h"

#include <QTextEdit>
#include <QMainWindow>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QFileInfo>
#include <QMessageBox>
#include <QStatusBar>

const QStringList FileManager::SUPPORTED_FORMATS = {
    tr("文本文件 (*.txt)"),
    tr("所有文件 (*)")
};

FileManager::FileManager(QTextEdit* editor, QMainWindow* parentWindow)
    : QObject(parentWindow)
    , m_editor(editor)
    , m_parentWindow(parentWindow)
    , m_currentFile()
{
}

bool FileManager::newFile()
{
    // 检查是否需要保存当前文件
    if (!maybeSave()) {
        return false;
    }

    // 清空编辑器
    m_editor->clear();
    m_editor->setFocus();

    // 重置当前文件
    setCurrentFile(QString());

    // 显示状态消息
    showStatusMessage(tr("已新建空文档"));

    // 发射信号
    emit fileLoaded();
    emit requestUpdateStats();

    return true;
}

bool FileManager::openFile(const QString& fileName)
{
    // 检查是否需要保存当前文件
    if (!maybeSave()) {
        return false;
    }

    QString fileToOpen = fileName;

    // 如果未指定文件名，显示打开对话框
    if (fileToOpen.isEmpty()) {
        fileToOpen = QFileDialog::getOpenFileName(m_parentWindow,
            tr("打开文件"),
            QString(),
            SUPPORTED_FORMATS.join(";;"));
        if (fileToOpen.isEmpty()) {
            return false; // 用户取消了对话框
        }
    }

    // 加载文件
    if (loadFile(fileToOpen)) {
        setCurrentFile(fileToOpen);
        showStatusMessage(tr("已打开 %1").arg(QFileInfo(fileToOpen).fileName()));
        emit fileLoaded();
        emit requestUpdateStats();
        return true;
    }

    return false;
}

bool FileManager::save()
{
    if (m_currentFile.isEmpty()) {
        return saveAs();
    }
    return saveToFile(m_currentFile);
}

bool FileManager::saveAs()
{
    QString fileName = QFileDialog::getSaveFileName(m_parentWindow,
        tr("另存为"),
        QString(),
        SUPPORTED_FORMATS.join(";;"));
    if (fileName.isEmpty()) {
        return false;
    }

    return saveToFile(fileName);
}

bool FileManager::maybeSave()
{
    if (!m_editor || !m_editor->document()->isModified()) {
        return true;
    }

    const QMessageBox::StandardButton ret = QMessageBox::warning(
        m_parentWindow,
        tr("未保存的更改"),
        tr("文档已修改。是否要保存修改？"),
        QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel,
        QMessageBox::Save
    );

    if (ret == QMessageBox::Save) {
        return save();
    }
    else if (ret == QMessageBox::Cancel) {
        return false;
    }

    return true; // Discard
}

bool FileManager::saveToFile(const QString& fileName)
{
    QFile file(fileName);
    if (!file.open(QFile::WriteOnly | QFile::Text)) {
        QMessageBox::warning(m_parentWindow,
            tr("保存失败"),
            tr("无法保存到 %1:\n%2")
            .arg(QFileInfo(fileName).fileName(),
                file.errorString()));
        return false;
    }

    // 获取文本内容
    const QString content = m_editor->toPlainText();
    QByteArray bytes = content.toUtf8();

    // 写入文件
    qint64 written = file.write(bytes);
    file.close();

    if (written == -1) {
        QMessageBox::warning(m_parentWindow,
            tr("保存失败"),
            tr("写入文件 %1 时出错。")
            .arg(QFileInfo(fileName).fileName()));
        return false;
    }

    // 更新状态
    m_editor->document()->setModified(false);
    setCurrentFile(fileName);

    // 显示状态消息
    showStatusMessage(tr("保存成功"));

    // 发射信号
    emit fileSaved();
    emit requestUpdateStats();

    return true;
}

bool FileManager::loadFile(const QString& fileName)
{
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        QMessageBox::warning(m_parentWindow,
            tr("打开失败"),
            tr("无法打开文件 %1:\n%2")
            .arg(QFileInfo(fileName).fileName(),
                file.errorString()));
        return false;
    }

    // 读取文件内容
    QTextStream in(&file);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    in.setCodec("UTF-8");
#endif
    const QString content = in.readAll();
    file.close();

    // 设置到编辑器
    m_editor->setPlainText(content);
    m_editor->document()->setModified(false);

    return true;
}

void FileManager::setCurrentFile(const QString& fileName)
{
    m_currentFile = fileName;
    updateWindowTitle();
    emit modificationChanged(isModified());
}

QString FileManager::displayFileName() const
{
    return m_currentFile.isEmpty() ? tr("Untitled")
        : QFileInfo(m_currentFile).fileName();
}

bool FileManager::isModified() const
{
    return m_editor && m_editor->document()->isModified();
}

void FileManager::updateWindowTitle()
{
    if (m_parentWindow) {
        QString title = QString("%1[*] - 文本编辑器").arg(displayFileName());
        m_parentWindow->setWindowTitle(title);
    }
}

void FileManager::showStatusMessage(const QString& message, int timeout)
{
    if (m_parentWindow && m_parentWindow->statusBar()) {
        m_parentWindow->statusBar()->showMessage(message, timeout);
    }
}