#ifndef REPORTWIDGET_H
#define REPORTWIDGET_H

#include <QWidget>
#include <KDReports.h>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>

class ReportWidget : public QWidget {
    Q_OBJECT

public:
    explicit ReportWidget(QWidget *parent = nullptr);
    ~ReportWidget() = default;

public slots:
    void createReport(const QVector<QPair<QString, QString>>& table);
    void clearReport();
    void previewReport();
    void exportToPDF();
    void printReport();

signals:
    void reportGenerated(KDReports::Report* report);

private:
    KDReports::Report* m_report;
    KDReports::PreviewWidget* m_previewWidget;
    QPushButton* m_exportButton;
    QPushButton* m_printButton;
};

#endif // REPORTWIDGET_H
