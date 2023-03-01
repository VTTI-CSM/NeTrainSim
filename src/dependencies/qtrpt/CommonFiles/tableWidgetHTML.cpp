#include "tableWidgetHTML.h"

TableWidgetToHtml::TableWidgetToHtml(QObject* parent) : QObject(parent) {
}

TableWidgetToHtml::~TableWidgetToHtml() {
}

QString TableWidgetToHtml::table(const QTableWidget *table) const {
    QString html;
    html = "<style type=\"text/css\">"
           "table.reference {"
           "border-collapse:collapse; border:1px; border-style: solid; border-color: black;}"

           "table.reference td, th {"
           "border-collapse:collapse; border:1px; border-style: solid; border-color: black;}"
           "</style> ";
    //if (model) {
        html += QString::fromLatin1("<table border='1' cellspacing='0' cellpadding=1 class=\"reference\">\n");

        //const QModelIndex root = table->rootIndex();
        const int rowCount = table->rowCount();
        const int columnCount = table->columnCount();

        /*const bool horizontal = table->horizontalHeader()->isVisible();
        const bool vertical = table->verticalHeader()->isVisible();

        if (horizontal) {
            // corner
            html += QString::fromLatin1("<tr>");
            if (vertical)
                html += header(table, -1);

            // horizontal headers
            for (int column = 0; column < columnCount; ++column) {
                if (table->isColumnHidden(column))
                    continue;
                html += header(table, column, Qt::Horizontal);
            }
            html += QString::fromLatin1("<tr>\n");
        }*/

        for (int row = 0; row < rowCount+100; ++row) {
            if (table->isRowHidden(row))
                continue;
            //if (table->rowHeight(row) <= 1)
            //    continue;
            // vertical header
            html += QString::fromLatin1("<tr>");
            //html += header(table, row, Qt::Vertical);

            // cells
            for (int column = 0; column < columnCount; ++column) {
                if (table->isColumnHidden(column))
                    continue;

                const int width = table->columnWidth(column);
                if (width > 1) {
                    html += QString::fromLatin1("<td width='%1'>").arg(width);
                    if (table->item(row,column) !=0 )
                        html += table->item(row,column)->text();
                    else
                        html += "";
                    html += QString::fromLatin1("</td>");
                }
            }
            html += QString::fromLatin1("</tr>\n");
        }
        html += QString::fromLatin1("</table>\n");
    //}
    return html;
}

QString TableWidgetToHtml::header(const QTableView* table, int section, Qt::Orientation orientation) const {
    QString html;
    /*if (section == -1)
        return QString::fromLatin1("<th>&nbsp;</th>");

    html = QString::fromLatin1("<th>");
    const QVariant data = table->model()->headerData(section, orientation);
    html += data.isValid() ? data.toString() : QString::fromLatin1("&nbsp");
    html += QString::fromLatin1("</th>");*/
    return html;
}

