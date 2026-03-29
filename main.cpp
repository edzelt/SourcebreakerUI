#include "MainWindow.h"

#include <QApplication>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    app.setApplicationName(QStringLiteral("Sourcebreaker"));
    app.setApplicationVersion(QStringLiteral("0.1.0"));
    app.setOrganizationName(QStringLiteral("edzelt"));

    MainWindow w;
    w.show();

    return app.exec();
}