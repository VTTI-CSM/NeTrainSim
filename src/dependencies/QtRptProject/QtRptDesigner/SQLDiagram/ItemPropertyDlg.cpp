#include "ItemPropertyDlg.h"
#include "ui_ItemPropertyDlg.h"

#include <QDebug>

ItemPropertyDlg::ItemPropertyDlg(QWidget *parent)
: QDialog(parent), ui(new Ui::ItemPropertyDlg)
{
    ui->setupUi(this);
}

void ItemPropertyDlg::showThis(DiagramItem *item)
{
    if (item->type() == DiagramItem::Table) {
        ui->stackedWidget->setCurrentIndex(0);
        DatabaseTable *table = static_cast<DatabaseTable *>(item);
        ui->tableWidget->setRowCount( table->columnList()->columnCount() );
        for (int i=0; i<table->columnList()->columnCount(); i++) {
            QTableWidgetItem *newItem = new QTableWidgetItem(table->columnList()->column(i)->name());
            if (table->columnList()->column(i)->isRequired())
                newItem->setCheckState(Qt::Checked);
            else
                newItem->setCheckState(Qt::Unchecked);
            ui->tableWidget->setItem(i,0,newItem);
        }
        ui->lineEdit->setText( table->name() );
        ui->lineEdit->setReadOnly(true);
    }
    if (item->type() == DiagramItem::Relation) {
        ui->stackedWidget->setCurrentIndex(1);
        DatabaseRelationship *relation = static_cast<DatabaseRelationship *>(item);

        DatabaseTable *childTable = relation->childTable();
        DatabaseTable *parentTable = relation->parentTable();
        ui->edtChildTable->setText(childTable->name());
        ui->edtParentTable->setText(parentTable->name());

        ui->childColumn->addItem("-");
        ui->parentColumn->addItem("-");

        for (int i=0; i<childTable->columnList()->columnCount(); i++) {
            ui->childColumn->addItem( childTable->columnList()->column(i)->name() );

            if (childTable->columnList()->column(i) == relation->childColumn() ) {
                ui->childColumn->setCurrentIndex(i+1);
            }
        }
        for (int i=0; i<parentTable->columnList()->columnCount(); i++) {
            ui->parentColumn->addItem( parentTable->columnList()->column(i)->name() );

            if (parentTable->columnList()->column(i) == relation->parentColumn() ) {
                ui->parentColumn->setCurrentIndex(i+1);
            }
        }
    }
    //Saving
    if (exec() == QDialog::Accepted) {
        if (item->type() == DiagramItem::Table) {
            DatabaseTable *table = static_cast<DatabaseTable *>(item);
            for (int i=0; i<table->columnList()->columnCount(); i++) {
                table->columnList()->column(i)->setRequired( ui->tableWidget->item(i,0)->checkState() );
            }
        }
        if (item->type() == DiagramItem::Relation) {
            DatabaseRelationship *relation = static_cast<DatabaseRelationship *>(item);

            DatabaseTable *childTable = relation->childTable();
            DatabaseTable *parentTable = relation->parentTable();

            if (ui->childColumn->currentIndex() == 0)
                relation->setChildColumn(nullptr);
            else
                relation->setChildColumn(childTable->columnList()->column( ui->childColumn->currentIndex()-1));

            if (ui->parentColumn->currentIndex() == 0)
                relation->setParentColumn(nullptr);
            else
                relation->setParentColumn( parentTable->columnList()->column( ui->parentColumn->currentIndex()-1));
        }
    }
}

ItemPropertyDlg::~ItemPropertyDlg()
{
    delete ui;
}
