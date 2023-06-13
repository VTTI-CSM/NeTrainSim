/**
 * @file AboutWindow.h
 * @brief This file contains the declaration of the AboutWindow class.
 *        The AboutWindow class is a QMainWindow subclass that represents
 *        a window displaying information about the application.
 * @author Ahmed Aredah
 * @date 6/7/2023
 */

#ifndef ABOUTWINDOW_H
#define ABOUTWINDOW_H

#include <QMainWindow>

namespace Ui {
class AboutWindow;
}

/**
 * @class AboutWindow
 * @brief The AboutWindow class represents a window displaying information about the application.
 */
class AboutWindow : public QMainWindow
{
    Q_OBJECT

public:
    /**
     * @brief Constructs an AboutWindow object.
     * @param parent The parent widget.
     */
    explicit AboutWindow(QWidget *parent = nullptr);

    /**
     * @brief Destroys the AboutWindow object.
     */
    ~AboutWindow();

private:
    Ui::AboutWindow *ui; /**< The user interface for the AboutWindow. */
};

#endif // ABOUTWINDOW_H
