#include "app_icon.h"
#include "database.h"
#include "main_window.h"

#include <QApplication>
#include <QMessageBox>

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    app.setWindowIcon(make_app_icon());

    DatabaseConnection database;

    if (!open_database(database, "blackbox.db")) {
        QMessageBox::critical(nullptr, "BlackBox", "Failed to open blackbox.db.");
        return 1;
    }

    if (!create_activity_segments_table(database)) {
        QMessageBox::critical(
            nullptr,
            "BlackBox",
            "Failed to create activity_segments table.");
        return 1;
    }

    MainWindow window(database);
    window.show();

    return app.exec();
}
