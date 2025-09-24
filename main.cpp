#include <QApplication>
#include <QStyleFactory>
#include <QDir>
#include "mainwindow.h"
#include "database.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // Set application properties
    app.setApplicationName("Smart Home Dashboard");
    app.setApplicationVersion("2.0");
    app.setOrganizationName("SmartHome Inc.");

    // db 연결
    Database& db = Database::instance();
    if (!db.connect("127.0.0.1", "hometer", "root", "1111")) {
        return -1; // 연결 실패 시 종료
    }

    // Create and show main window
    MainWindow window;
    window.show();

    return app.exec();
}
