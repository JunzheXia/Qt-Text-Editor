#include "QtWidgetsApplication.h"
#include <QtWidgets/QApplication>
#include <qfile.h>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);


    QFile f(":qdarkstyle/dark/darkstyle.qss");
    f.open(QFile::ReadOnly | QFile::Text);
    QTextStream ts(&f);
    qApp->setStyleSheet(ts.readAll());


    QtWidgetsApplication window;
    QApplication::setStyle("Fusion");
    window.show();
    return app.exec();
}
