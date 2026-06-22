#include "gui/main_window_style.h"

QString main_window_style_sheet() {
    return QString::fromUtf8(R"qss(
        QMainWindow {
            background: #fafafa;
        }

        QWidget#root {
            background: #fafafa;
            color: #1f2937;
            font-family: "Segoe UI";
            font-size: 14px;
        }

        QLabel#periodLabel {
            color: #111827;
            font-size: 16px;
            font-weight: 700;
        }

        QLabel#statusLabel {
            background: #fff3e8;
            border: none;
            border-radius: 5px;
            color: #9a4a12;
            font-weight: 650;
            padding: 6px 10px;
        }

        QLabel#statusLabel[recording="true"] {
            background: #e8f7ee;
            color: #166b3b;
        }

        QLabel#totalLabel {
            background: transparent;
            border: none;
            padding: 2px 0;
            font-size: 34px;
            font-weight: 700;
            color: #111827;
        }

        QLabel#countLabel {
            background: transparent;
            border: none;
            padding: 14px 0 0 4px;
            color: #7a8493;
            font-size: 14px;
            font-weight: 500;
        }

        QLabel#sectionLabel {
            color: #344054;
            font-size: 14px;
            font-weight: 700;
            padding: 4px 0 0 2px;
        }

        QPushButton {
            border: 1px solid #d9dee7;
            border-radius: 5px;
            background: #ffffff;
            padding: 7px 12px;
            min-height: 28px;
            color: #344054;
            font-size: 14px;
            font-weight: 600;
        }

        QPushButton:hover {
            background: #f9fafb;
            border-color: #98a2b3;
        }

        QPushButton:disabled {
            color: #98a2b3;
            background: #f2f4f7;
            border-color: #eaecf0;
        }

        QPushButton#recordingButton {
            background: #1f7a4d;
            border-color: #1f7a4d;
            color: #ffffff;
            font-weight: 700;
            padding-left: 14px;
            padding-right: 14px;
        }

        QPushButton#recordingButton[recording="true"] {
            background: #a15c18;
            border-color: #a15c18;
        }

        QPushButton#modeButton[selected="true"] {
            background: #1f2937;
            border-color: #1f2937;
            color: #ffffff;
            font-weight: 700;
        }

        QPushButton#modeButton[selected="false"] {
            background: #ffffff;
        }

        QPushButton#secondaryButton {
            color: #315c50;
        }

        QToolButton#iconButton {
            background: transparent;
            border: 1px solid transparent;
            border-radius: 5px;
            color: #475467;
            font-size: 19px;
            font-weight: 600;
        }

        QToolButton#iconButton:hover {
            background: #f1f4f7;
            border-color: #e2e7ef;
            color: #111827;
        }

        QToolButton#iconButton:pressed {
            background: #e8edf3;
        }

        QTabWidget#contentTabs::pane {
            border: none;
            background: #ffffff;
            top: 0;
        }

        QTabBar::tab {
            background: transparent;
            border: none;
            border-bottom: 2px solid transparent;
            color: #7a8493;
            font-size: 14px;
            font-weight: 650;
            min-width: 88px;
            padding: 9px 4px 10px 4px;
        }

        QTabBar::tab:selected {
            border-bottom-color: #111827;
            color: #111827;
        }

        QTabBar::tab:!selected:hover {
            color: #344054;
        }

        QTableWidget {
            background: #ffffff;
            alternate-background-color: #fbfbfc;
            border: none;
            border-radius: 0;
            selection-background-color: #edf4ff;
            selection-color: #111827;
            gridline-color: transparent;
            font-size: 15px;
        }

        QTableWidget::item {
            padding: 9px 12px;
            border: none;
        }

        QHeaderView::section {
            background: #ffffff;
            color: #8a94a3;
            border: none;
            border-bottom: 1px solid #edf0f4;
            padding: 9px 12px;
            font-size: 13px;
            font-weight: 700;
        }

        QStatusBar {
            background: #fafafa;
            color: #667085;
            padding-left: 4px;
        }
    )qss");
}
