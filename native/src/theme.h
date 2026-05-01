#pragma once

#include <QApplication>
#include <QPalette>
#include <QStyleFactory>

namespace commitly {

inline void applyDarkTheme(QApplication *app) {
    app->setStyle(QStyleFactory::create("Fusion"));

    QPalette darkPalette;
    darkPalette.setColor(QPalette::Window, QColor(30, 30, 30));
    darkPalette.setColor(QPalette::WindowText, Qt::white);
    darkPalette.setColor(QPalette::Base, QColor(25, 25, 25));
    darkPalette.setColor(QPalette::AlternateBase, QColor(35, 35, 35));
    darkPalette.setColor(QPalette::ToolTipBase, Qt::white);
    darkPalette.setColor(QPalette::ToolTipText, Qt::white);
    darkPalette.setColor(QPalette::Text, Qt::white);
    darkPalette.setColor(QPalette::Button, QColor(45, 45, 45));
    darkPalette.setColor(QPalette::ButtonText, Qt::white);
    darkPalette.setColor(QPalette::BrightText, Qt::red);
    darkPalette.setColor(QPalette::Link, QColor(42, 130, 218));

    darkPalette.setColor(QPalette::Highlight, QColor(42, 130, 218));
    darkPalette.setColor(QPalette::HighlightedText, Qt::black);

    app->setPalette(darkPalette);

    app->setStyleSheet(
        "QToolTip { color: #ffffff; background-color: #2a82da; border: 1px solid white; }"
        "QSplitter::handle { background-color: #444; }"
        "QHeaderView::section { background-color: #333; color: white; border: 1px solid #444; padding: 4px; }"
        "QListView, QTreeView, QTableView { border: none; background-color: #1e1e1e; }"
        "QScrollBar:vertical { width: 12px; background: #222; }"
        "QScrollBar::handle:vertical { background: #555; min-height: 20px; }"
    );
}

} // namespace commitly
