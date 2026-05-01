#pragma once

#include <QDialog>
#include <QTabWidget>

namespace commitly {

class PreferencesDialog : public QDialog
{
    Q_OBJECT
public:
    PreferencesDialog(QWidget *parent = nullptr);

private:
    void setupAppearanceTab(QWidget *tab);
    void setupGitTab(QWidget *tab);
};

} // namespace commitly
