#ifndef SETTINGSWINDOW_H
#define SETTINGSWINDOW_H

#include "qlineedit.h"
#include <QMainWindow>

namespace Ui {
class settingsWindow;
}

class settingsWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit settingsWindow(QWidget *parent = nullptr);
    ~settingsWindow();

private slots:
    void on_pushButton_browse_clicked();

    void on_pushButton_save_clicked();

private:
    Ui::settingsWindow *ui;

    void browseFolder(QLineEdit* theLineEdit, const QString& theHelpMessage);
    void loadSavedSettings();
};

#endif // SETTINGSWINDOW_H
