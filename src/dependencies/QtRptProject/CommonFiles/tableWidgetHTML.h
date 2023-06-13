#ifndef TABLEWIDGETHTML_H
#define TABLEWIDGETHTML_H

#include <QObject>
#include <QtGui>
#include <QStringList>
#include <QTableWidget>

class TableWidgetToHtml : public QObject {
	Q_OBJECT

public:
        TableWidgetToHtml(QObject* parent = 0);
        ~TableWidgetToHtml();

	// converts QTableView contents to HTML
        QString table(const QTableWidget *table) const;

	// returns a list of temporary image
	// files created during the conversion
	QStringList imageFiles() const { return tmpFiles; }

private:
	QString header(const QTableView* table, int section, Qt::Orientation orientation = Qt::Horizontal) const;

	mutable QStringList tmpFiles;
};

#endif // TABLEWIDGETHTML_H
