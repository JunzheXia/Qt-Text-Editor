#include "QtWidgetsApplication.h"
#include "StringProcessor.h"
#include "KMPMatcher.h"
#include "FindReplaceController.h"
#include "FontTextMenu.h"
#include "FileManager.h" 

#include <QMessageBox>
#include <QGridLayout>
#include <QTextEdit>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QFileInfo>
#include <QLabel>
#include <QShortcut>
#include <QTimer>
#include <QMenu> 

QtWidgetsApplication::QtWidgetsApplication(QWidget* parent)
    : QMainWindow(parent)
    , m_editor(nullptr)
    , m_fileManager(nullptr)     
    , m_processor(nullptr)
    , m_findController(nullptr)
    , m_fontController(nullptr)
    , m_statsLabel(nullptr)
    , m_findAction(nullptr)
    , m_replaceAction(nullptr)
    , m_deleteAction(nullptr)
    , m_fontAction(nullptr)
    , m_textSizeAction(nullptr)
    , m_shortcutFindNext(nullptr)
    , m_shortcutFindPrev(nullptr)
    , m_shortcutReplaceNext(nullptr)
    , m_shortcutReplacePrev(nullptr)
{
    ui.setupUi(this);

    initEditor();

    initActions();

    initControllers();

    initStats();

    initShortcuts();

    initFontMenu();

    updateStats();
}

QtWidgetsApplication::~QtWidgetsApplication()
{
    delete m_findController;
    delete m_processor;
    delete m_fontController;
    delete m_fileManager; 
}


void QtWidgetsApplication::initEditor()
{
    m_editor = findChild<QTextEdit*>("TextEdit");

    m_editor->setParent(ui.centralWidget);

    if (!ui.centralWidget->layout()) {
        QGridLayout* layout = new QGridLayout(ui.centralWidget);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->addWidget(m_editor, 0, 0, 1, 1); // 占据整个网格
        layout->setRowStretch(0, 1);
        layout->setColumnStretch(0, 1);
    }

    m_editor->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}


void QtWidgetsApplication::initActions()
{
    // 获取UI中的动作
    m_findAction = ui.Find;
    m_replaceAction = ui.Replace;
    m_fontAction = ui.Font;
    m_textSizeAction = ui.TextSize;
    m_deleteAction = ui.Delete;

    // 设置动作提示
    if (m_findAction) {
        m_findAction->setStatusTip(tr("查找（提示：可使用 F3 查找下一个）"));
    }
    if (m_replaceAction) {
        m_replaceAction->setStatusTip(tr("替换（提示：F3 查找下一个，F4 替换下一个）"));
    }

    // 连接删除动作
    if (m_deleteAction) {
        connect(m_deleteAction, &QAction::triggered, this, &QtWidgetsApplication::on_Delete_triggered);
    }
}

void QtWidgetsApplication::initControllers()
{
    // 创建文件管理器
    m_fileManager = new FileManager(m_editor, this);

    // 连接文件管理器信号
    connect(m_fileManager, &FileManager::requestUpdateStats,
        this, &QtWidgetsApplication::updateStats);

    // 创建查找/替换控制器
    m_findController = new FindReplaceController(m_editor, this);
    connect(m_findController, &FindReplaceController::requestUpdate,
        this, &QtWidgetsApplication::updateStats);
}

void QtWidgetsApplication::initStats()
{
    m_processor = new StringProcessor();
    m_statsLabel = new QLabel(this);
    m_statsLabel->setText(tr("总: 0 中文: 0 英文: 0 数字: 0 符号: 0"));
    statusBar()->addPermanentWidget(m_statsLabel);

    // 文本变化时更新统计
    connect(m_editor, &QTextEdit::textChanged, this, &QtWidgetsApplication::updateStats);
}

void QtWidgetsApplication::initShortcuts()
{
    // 查找快捷键
    m_shortcutFindNext = new QShortcut(QKeySequence(Qt::Key_F3), this);
    m_shortcutFindPrev = new QShortcut(QKeySequence(Qt::SHIFT | Qt::Key_F3), this);
    m_shortcutReplaceNext = new QShortcut(QKeySequence(Qt::Key_F4), this);
    m_shortcutReplacePrev = new QShortcut(QKeySequence(Qt::SHIFT | Qt::Key_F4), this);

    // 连接快捷键
    connect(m_shortcutFindNext, &QShortcut::activated, this, [this]() {
        if (m_findController) m_findController->findNext();
        });
    connect(m_shortcutFindPrev, &QShortcut::activated, this, [this]() {
        if (m_findController) m_findController->findPrev();
        });
    connect(m_shortcutReplaceNext, &QShortcut::activated, this, [this]() {
        if (m_findController) m_findController->replaceNext();
        });
    connect(m_shortcutReplacePrev, &QShortcut::activated, this, [this]() {
        if (m_findController) m_findController->replacePrev();
        });
}

void QtWidgetsApplication::initFontMenu()
{
    QMenu* formatMenu = findChild<QMenu*>("MenuText");

    // 创建字体控制器
    m_fontController = new FontTextMenu(m_editor, this, m_fontAction, m_textSizeAction, formatMenu);

    // 连接字体控制器的信号
    connect(m_fontController, &FontTextMenu::fontChanged, this, [this](const QFont& font) {
        statusBar()->showMessage(tr("字体已设置: %1, %2pt").arg(font.family()).arg(font.pointSizeF()), 3000);
        });

    connect(m_fontController, &FontTextMenu::fontSizeChanged, this, [this](qreal size) {
        statusBar()->showMessage(tr("字号已设置: %1 pt").arg(size), 3000);
        });
}


void QtWidgetsApplication::on_NewFile_triggered()
{
    m_fileManager->newFile();
}

void QtWidgetsApplication::on_OpenFile_triggered()
{
    m_fileManager->openFile();
}

void QtWidgetsApplication::on_SaveFile_triggered()
{
    m_fileManager->save();
}


void QtWidgetsApplication::on_Find_triggered()
{
    showTemporaryHint(tr("查找: 按 F3 查找下一个, Shift+F3 查找上一个"), 4000);
    if (m_findController) m_findController->find();
}

void QtWidgetsApplication::on_Replace_triggered()
{
    showTemporaryHint(tr("替换: 按 F4 替换下一个, Shift+F4 替换上一个"), 5000);
    if (m_findController) m_findController->replace();
}

void QtWidgetsApplication::on_Delete_triggered()
{
    if (!m_findController) return;

    auto ret = QMessageBox::question(this, tr("删除匹配"),
        tr("确定要删除所有与最近一次查找匹配的字符串吗？此操作不可撤销。"),
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if (ret != QMessageBox::Yes) return;

    showTemporaryHint(tr("正在删除所有匹配..."), 2000);
    m_findController->deleteAllMatches();
}


void QtWidgetsApplication::updateStats()
{
    if (!m_processor || !m_editor) return;

    auto res = m_processor->process(m_editor->toPlainText());
    m_statsLabel->setText(tr("总: %1  中文: %2  英文: %3  数字: %4  符号: %5")
        .arg(res.total).arg(res.chinese).arg(res.letters).arg(res.digits).arg(res.symbols));
}

void QtWidgetsApplication::showTemporaryHint(const QString& hint, int timeout)
{
    if (!m_statsLabel) return;

    QString prevText = m_statsLabel->text();
    m_statsLabel->setText(prevText + QStringLiteral("    [提示] ") + hint);
    QTimer::singleShot(timeout, this, [this]() { updateStats(); });
}