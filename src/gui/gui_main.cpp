#include "gui/app_icon.h"
#include "gui/main_window.h"
#include "settings/app_settings.h"
#include "storage/database.h"

#include <QApplication>
#include <QMessageBox>

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    app.setWindowIcon(make_app_icon());

    DatabaseConnection database;
    const AppSettings settings = load_app_settings();

    if (!open_database(database, settings.database_path)) {
        QMessageBox::critical(
            nullptr,
            "BlackBox",
            "Failed to open " + QString::fromStdString(settings.database_path) + ".");
        return 1;
    }

    if (!create_activity_segments_table(database)) {
        QMessageBox::critical(
            nullptr,
            "BlackBox",
            "Failed to create activity_segments table.");
        return 1;
    }

    MainWindow window(database, settings);
    window.show();

    return app.exec();
}
