#include "AppMainWindow.h"

#include <kddockwidgets/KDDockWidgets.h>

#include <QApplication>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("SourcebreakerUI"));
    app.setOrganizationName(QStringLiteral("edzelt"));

    // Инициализация KDDockWidgets — после QApplication, до создания окна
    KDDockWidgets::initFrontend(KDDockWidgets::FrontendType::QtWidgets);

    AppMainWindow w;
    w.show();

    return app.exec();
}