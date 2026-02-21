#include "reportwidget.h"
#include <QFileDialog>
#include <QPrinter>
#include <QPrintDialog>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QMessageBox>
#include <qstandarditemmodel.h>

ReportWidget::ReportWidget(QWidget *parent)
    : QWidget(parent),
    m_report(new KDReports::Report(this)),
    m_previewWidget(new KDReports::PreviewWidget(this)),
    m_exportButton(new QPushButton("Export as PDF", this)),
    m_printButton(new QPushButton("Print", this))
{
    m_report->setDocumentName("Empty Report");
    m_previewWidget->setReport(m_report);

    // Add placeholder content
    KDReports::TextElement placeholder("No report content available.");
    placeholder.setPointSize(12);
    m_report->addElement(placeholder);
    m_report->setPageSize(QPageSize::Letter);

    // Layout setup
    auto buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_exportButton);
    buttonLayout->addWidget(m_printButton);

    auto mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(m_previewWidget);
    mainLayout->addLayout(buttonLayout);

    setLayout(mainLayout);

    // Connect buttons to their respective slots
    connect(m_exportButton, &QPushButton::clicked,
            this, &ReportWidget::exportToPDF);
    connect(m_printButton, &QPushButton::clicked,
            this, &ReportWidget::printReport);
}


void ReportWidget::createReport(const QVector<QPair<QString, QString>> &table)
{
    // Ensure report is reset before adding new content
    clearReport(); // Ensure clean state

    if (!m_report) {
        qDebug() << "m_report is null after reset!";
        return;
    }

    m_report->setReportMode(KDReports::Report::WordProcessing);
    m_report->setPageOrientation(QPageLayout::Orientation::Landscape);
    m_report->setPageSize(QPageSize::Letter);
    m_report->setMargins(5.0, 10, 10, 10);
    m_report->setDocumentName("NeTrainSim Report");

    // --- HEADER SECTION ---
    KDReports::Header& header = m_report->header();

    // Report Title
    // Use HTML for better alignment (NeTrainSim on the left, Date on the right)
    QString currentDateTime =
        QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");

    QString headerContent = R"(
    <div style='width: 100%; margin-top: 0px;'>
        <!-- First row: NeTrainSim (Bold, Larger) -->
        <div style='font-size: 16px; font-weight: bold; margin-bottom: 2px;'>NeTrainSim</div>

        <!-- Second row: Open-Source Train Network Simulator (Left) and Date (Right) -->
        <div style='display: flex; justify-content: space-between; align-items: center; font-size: 12px;'>
            <div>Open-Source Train Network Simulator</div>
            <div>Generated: )" + currentDateTime + R"(</div>
        </div>
    </div>
)";

    // Add the formatted header
    KDReports::HtmlElement headerElement(headerContent);
    header.addElement(headerElement);

    // --- TITLE SECTION ---
    KDReports::TextElement title("NeTrainSim Data Report");
    title.setPointSize(16);
    title.setBold(true);
    m_report->addElement(title, Qt::AlignCenter);

    // --- DATA SECTION ---
    const QVector<QString>& headers = {"Key", "Value"};
    if (headers.isEmpty()) {
        return;
    }

    // Determine row count
    int rowCount = table.size();;

    // Find the maximum header length for alignment
    int maxHeaderLength = 0;
    for (const QString& header : headers) {
        if (header.length() > maxHeaderLength) {
            maxHeaderLength = header.length();
        }
    }

    // Add data as paragraphs with aligned values
    for (int row = 0; row < rowCount; ++row) {
        QString rowData;

        QString firstCell = table[row].first;
        QString secondCell = table[row].second;

        // Check empty row
        if ((firstCell.isNull() || firstCell.trimmed().isEmpty()) &&
            (secondCell.isNull() || secondCell.trimmed().isEmpty()))
        {
            continue;
        }
        rowData += firstCell.leftJustified(15, ' ');  // Align first cell width
        if (!secondCell.trimmed().isEmpty()) {
            rowData += " | " + secondCell;  // Separate other cells by "|"
        }


        KDReports::TextElement rowElement(rowData);
        rowElement.setPointSize(10); // Reduce font size for compactness
        rowElement.setFontFamily("Courier New"); // Monospace font for better alignment
        m_report->addElement(rowElement);

    }

    // --- FOOTER SECTION ---
    KDReports::Footer& footer = m_report->footer();

    // Contact & License Information
    KDReports::TextElement contactElement("NeTrainSim | "
                                          "Open-Source Train Simulation");
    contactElement.setPointSize(9);
    footer.addElement(contactElement, Qt::AlignCenter);

    // Get the current year dynamically
    QString currentYear = QString::number(QDate::currentDate().year());

    // Create the license text with the dynamic year
    QString licenseText = "© " +
                          currentYear + " NeTrainSim Project. "
                                        "Licensed under GNU GPL v3 License.";

    // Add the license element
    KDReports::TextElement licenseElement(licenseText);
    licenseElement.setPointSize(8);
    footer.addElement(licenseElement, Qt::AlignCenter);

    emit reportGenerated(m_report);
    m_previewWidget->setReport(m_report);
}



void ReportWidget::clearReport()
{
    if (m_report) {
        delete m_report; // Delete existing report
        m_report = nullptr;
    }

    m_report = new KDReports::Report(this);
}



void ReportWidget::previewReport()
{
    if (m_report) {
        KDReports::PreviewDialog preview(m_report);
        preview.exec();
    }
}

void ReportWidget::exportToPDF()
{
    if (!m_report) {
        QMessageBox::warning(this, "Export Error", "No report "
                                                   "available to export.");
        return;
    }

    QString filePath = QFileDialog::getSaveFileName(this,
                                                    "Export Report to PDF",
                                                    QString(), "*.pdf");
    if (filePath.isEmpty()) {
        return; // User canceled
    }

    if (!filePath.endsWith(".pdf")) {
        filePath += ".pdf";
    }

    m_report->exportToFile(filePath);
    QMessageBox::information(this, "Export Successful", "Report exported to "
                                                            + filePath);
}

void ReportWidget::printReport()
{
    if (!m_report) {
        QMessageBox::warning(this, "Print Error", "No report "
                                                  "available to print.");
        return;
    }

    QPrinter printer;
    QPrintDialog printDialog(&printer, this);

    if (printDialog.exec() == QDialog::Accepted) {
        m_report->print(&printer);
    }
}
