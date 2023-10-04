#ifndef IMPORTSHPWINDOW_H
#define IMPORTSHPWINDOW_H

#include "ui_importshpwindow.h"
#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class ImportSHPWindow; }
QT_END_NAMESPACE

/**
 * \class ImportSHPWindow
 * \brief A class to handle the importing of Shapefiles (SHP).
 *
 * This class provides a user interface for importing and handling Shapefiles
 * within the QMainWindow context. It is designed to interact with the UI
 * elements defined in "ui_importshpwindow.h".
 */
class ImportSHPWindow : public QMainWindow
{
    Q_OBJECT

public:
    /**
     * \brief Constructor that initializes the
     *        ImportSHPWindow with a given parent widget.
     * \param parent The parent widget for this window. Default is nullptr.
     */
    ImportSHPWindow(QWidget *parent = nullptr);

    /**
     * \brief Destructor that cleans up the ImportSHPWindow.
     */
    ~ImportSHPWindow();

private slots:
    void on_pushButton_browseSHP_clicked();

    void on_pushButton_import_clicked();

private:

    /**
     * \brief A pointer to the user interface elements for this window.
     */
    Ui::ImportSHPWindow *ui;

    QStringList proj;

    bool checkCRSValidSelection();

    void setConversionVisibility(bool visible);

    bool isSubstringInQStringList(const QString& substring,
                                  const QStringList& list);
};
#endif // IMPORTSHPWINDOW_H
