#include <QApplication>
#include <QFont>
#include "ui/MainWindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    app.setApplicationName("Always CD Ripper");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("YOUICHI SAIJO");

    // フォント設定
    QFont font("Segoe UI", 9);
    app.setFont(font);

    MainWindow w;
    w.show();

    return app.exec();
}
