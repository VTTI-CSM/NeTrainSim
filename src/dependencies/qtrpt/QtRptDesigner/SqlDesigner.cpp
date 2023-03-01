/*
Name: QtRpt
Version: 3.0.0
Web-site: https://qtrpt.sourceforge.io
Programmer: Aleksey Osipov
E-mail: aliks-os@ukr.net
Web-site: http://www.aliks-os.tk

Copyright 2012-2022 Aleksey Osipov

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#include "SqlDesigner.h"
#include "ui_SqlDesigner.h"
#include <QMessageBox>
#include <QDrag>
#include <QGraphicsSceneDragDropEvent>
#include <QGraphicsTextItem>
#include <QGraphicsProxyWidget>
#include <QFileDialog>
#include <QActionGroup>
#include "SQLHighlighter.h"
#include "XmlViewModel.h"

SqlDesigner::SqlDesigner(QSharedPointer<QDomDocument> xmlDoc, QWidget *parent)
: QWidget(parent), ui(new Ui::SqlDesigner)
{
    ui->setupUi(this);
    m_xmlDoc = xmlDoc;

    QSettings settings(QCoreApplication::applicationDirPath()+"/setting.ini",QSettings::IniFormat);
    #if QT_VERSION < QT_VERSION_CHECK(6,0,0)
        settings.setIniCodec("UTF-8");
    #endif

    ui->sqlEditor->setAcceptDrops(true);
    ui->graphicsView->setAcceptDrops(true);
    ui->sqlEditor->installEventFilter(this);
    ui->tablesTree->viewport()->installEventFilter(this);
    ui->xmlFieldsTable->viewport()->installEventFilter(this);

    auto actGroup = new QActionGroup(this);
    actGroup->addAction(ui->actAddRelationship);
    actGroup->addAction(ui->actSelect);
    m_currentPageNo = -1;

    new SQLHighlighter(ui->sqlEditor->document(), &settings);

    QObject::connect(ui->rbCustom, SIGNAL(clicked()), this, SLOT(rbChecked()));
    QObject::connect(ui->rbSql, SIGNAL(clicked()), this, SLOT(rbChecked()));
    QObject::connect(ui->rbXml, SIGNAL(clicked()), this, SLOT(rbChecked()));
    QObject::connect(ui->btnCheck, SIGNAL(clicked()), this, SLOT(connectDB()));
    QObject::connect(ui->btnClose, SIGNAL(clicked()), this, SLOT(btnClose()));
    QObject::connect(ui->actAddRelationship, SIGNAL(triggered()), this, SLOT(addRelation()));
    QObject::connect(ui->actClearDiagram, SIGNAL(triggered()), this, SLOT(clearDiagram()));
    QObject::connect(ui->actSelect, SIGNAL(triggered()), this, SLOT(select()));
    QObject::connect(ui->actUndo, SIGNAL(triggered()), this, SLOT(undo()));
    QObject::connect(ui->actRedo, SIGNAL(triggered()), this, SLOT(redo()));
    QObject::connect(ui->actDelete, SIGNAL(triggered()), this, SLOT(deleteSelected()));
    QObject::connect(ui->btnSelectXML, SIGNAL(clicked()), this, SLOT(selectXMLFile()));
    QObject::connect(ui->btnPreviewXMLData, SIGNAL(clicked()), this, SLOT(previewXMLData()));
}

void SqlDesigner::connectDB()
{
    if (db.isOpen()) {
        QString connection = db.connectionName();
        db.close();
        db = QSqlDatabase();
        db.removeDatabase(connection);
    }
    db = QSqlDatabase::addDatabase(ui->cmbType->currentText(),"DB1");
    db.setDatabaseName(ui->edtDBName->text());
    db.setHostName(ui->edtHost->text());
    db.setUserName(ui->edtUserName->text());
    db.setPassword(ui->edtPassword->text());
    if(!ui->edtPort->text().isEmpty())
        db.setPort(ui->edtPort->text().toInt());

    db.open();
    if (!db.isOpen()) {
        QMessageBox::warning(this, tr("Error"), db.lastError().text(), QMessageBox::Ok);
    } else {
        refreshTable(&db);
        QMessageBox::information(this, tr("Info"), tr("Connected"), QMessageBox::Ok);        
    }
}

void SqlDesigner::refreshTable(QSqlDatabase *db)
{
    ui->tablesTree->clear();
    QIcon icon;
    for (const auto &tableName : db->tables(QSql::Tables)) {
        auto tableItem = new QTreeWidgetItem(ui->tablesTree);
        tableItem->setText(0,tableName);
        icon.addPixmap(QPixmap(":/new/prefix1/images/table.png"), QIcon::Normal, QIcon::On);
        tableItem->setIcon(0, icon);
        tableItem->setData(0,Qt::UserRole,3);  //Table
    }    
}

QVector<QDomElement> SqlDesigner::saveParamToXML(QSharedPointer<QDomDocument> xmlDoc)
{
    QVector<QDomElement> elemList;

    QDomElement elem;
    if (ui->rbSql->isChecked()) {
        elem = buildDomElem();
        currentScene->save(xmlDoc, elem);

        elemList << elem;
    }
    if (ui->rbXml->isChecked()) {

    }

//    elemList << elemINLINE;
    // --- Test INLINE DS---
//    QString delimiter = "|";
//    int recCount = 5;
    elem = xmlDoc->createElement("DataSource");
    elem = elemINLINE;
//    elem.setAttribute("name","DB3");
//    elem.setAttribute("type","INLINE");
//    elem.setAttribute("delimiter",delimiter);
//    elem.setAttribute("rowCount","5");

//    for (int i = 0; i < recCount+1; i++) {
//        QDomElement e = xmlDoc->createElement("Row");
//        if (i == 0) {  // It is a column's names
//            e.setAttribute("value","Col|Col2|Col3|Col4");
//        } else {  // It is a values
//            e.setAttribute("value", QString("C1_%1|C2_%2|C3_%3|C4_%4").arg(i).arg(i*10).arg(i*100).arg(i*1000) );
//        }

//        elem.appendChild(e);
//    }

    elemList << elem;
    // --- Test INLINE DS---

    return elemList;
}

QDomElement SqlDesigner::buildDomElem()
{
    QDomElement elem;
    if (ui->rbSql->isChecked()) {
        elem = m_xmlDoc->createElement("DataSource");
        elem.setAttribute("name","DB1");
        elem.setAttribute("type","SQL");
        elem.setAttribute("dbType",ui->cmbType->currentText());
        elem.setAttribute("dbName",ui->edtDBName->text());
        elem.setAttribute("dbHost",ui->edtHost->text());
        elem.setAttribute("dbUser",ui->edtUserName->text());
        elem.setAttribute("dbPassword",ui->edtPassword->text());
        elem.setAttribute("dbCoding",ui->edtConnectionCoding->text());
        elem.setAttribute("charsetCoding",ui->edtCharsetCoding->text());
        elem.setAttribute("dbConnectionName",ui->edtConName->text());
        elem.setAttribute("dbPort",ui->edtPort->text());
        QDomText t = m_xmlDoc->createTextNode(ui->sqlEditor->toPlainText());
        elem.appendChild(t);
    }
    if (ui->rbXml->isChecked()) {

    }
    return elem;
}

void SqlDesigner::selectXMLFile()
{
    QString folderPath = QApplication::applicationDirPath();
    QString fileName = QFileDialog::getOpenFileName(this, tr("Select File"), folderPath, "XML (*.xml);;User Interface files (*.ui)");
    if (fileName.isEmpty())
        return;

    QFile file(fileName);
    if (file.open(QIODevice::ReadOnly)) {
        QDomDocument document;
        if (document.setContent(&file)) {
            XMLViewModel *model = new XMLViewModel(&document, this);
            ui->xmlStructTree->setModel(model);

        } else {
            qDebug() << "not set";
        }
        file.close();
    } else {
        qDebug() << "not found";
    }
}

void SqlDesigner::previewXMLData()
{
    qDebug() << "preview xml data";
}

void SqlDesigner::rbChecked()
{
    if (ui->rbCustom->isChecked())
        ui->stackedWidget->setCurrentIndex(0);
    if (ui->rbSql->isChecked())
        ui->stackedWidget->setCurrentIndex(1);
    if (ui->rbXml->isChecked())
        ui->stackedWidget->setCurrentIndex(2);

    emit changed(true);
}

void SqlDesigner::btnClose()
{
    setVisible(false);
    auto act1 = this->parentWidget()->parentWidget()->findChild<QAction *>("actDataSource");
    auto act2 = this->parentWidget()->parentWidget()->findChild<QAction *>("actSaveReport");
    if (act1 != nullptr) {
        act1->setChecked(false);
        act2->setEnabled(true);
    }
}

#include "CommonClasses.h"
#include "column.h"
#include "columnlist.h"
bool SqlDesigner::eventFilter(QObject *obj, QEvent *e)
{
    if (obj == ui->tablesTree->viewport()) {
        if (e->type() == QEvent::MouseButtonPress) {
            QMouseEvent *me = static_cast<QMouseEvent *>(e);
            if (me->buttons() & Qt::LeftButton) {
                auto item = ui->tablesTree->itemAt(me->pos());

                if (item) {
                    auto colLst = new ColumnList(nullptr);

                    QSqlQuery q("select * from "+item->text(0)+" LIMIT 0, 0",db);
                    QSqlRecord rec = q.record();
                    for (int i=0; i<rec.count(); i++) {
                        auto col = new Column(colLst);
                        col->setName(rec.fieldName(i));
                        col->setDataType( QVariant::typeToName(rec.field(i).type()) );
                        colLst->appendColumn(col);
                    }
                    currentScene->passColumnsForDroping(colLst);
                }
            }
        }
    }
    if (obj == ui->sqlEditor) {
        if (e->type() == QEvent::DragEnter) {
            QDragEnterEvent *de = static_cast<QDragEnterEvent*>(e);
            de->accept();
            return true;
        }
        if (e->type() == QEvent::Drop) {
            QDropEvent *de = static_cast<QDropEvent*>(e);
            const QMimeData *mimeData = de->mimeData();
            QByteArray encoded = mimeData->data("application/x-qabstractitemmodeldatalist");
            QDataStream stream(&encoded, QIODevice::ReadOnly);
            while (!stream.atEnd()) {
                 int row, col;
                 QMap<int,  QVariant> roleDataMap;
                 stream >> row >> col >> roleDataMap;
                 QString dropped = roleDataMap[0].toString();
                 ui->sqlEditor->textCursor().insertText(dropped);

             }
        }
    }
    if (obj == ui->xmlFieldsTable->viewport()) {
        if (e->type() == QEvent::Drop) {
            qDebug() << "DROP";
            auto de = static_cast<QDropEvent*>(e);
            const QMimeData *mimeData = de->mimeData();
            QByteArray encoded = mimeData->data("application/x-qabstractitemmodeldatalist");
            QDataStream stream(&encoded, QIODevice::ReadOnly);
            while (!stream.atEnd()) {
                int row, col;
                QMap<int,  QVariant> roleDataMap;
                stream >> row >> col >> roleDataMap;
                //for (int i=0; i<roleDataMap.size(); i++)
                //    qDebug() << roleDataMap[i].toString();
                ui->xmlFieldsTable->setRowCount(ui->xmlFieldsTable->rowCount()+1);

                QString fieldName = roleDataMap[0].toString().replace("("+roleDataMap[3].toString()+")","");
                QTableWidgetItem *item = new QTableWidgetItem(fieldName);
                ui->xmlFieldsTable->setItem(ui->xmlFieldsTable->rowCount()-1,0,item);

                QString fieldPath = "path";
                item = new QTableWidgetItem(fieldPath);
                ui->xmlFieldsTable->setItem(ui->xmlFieldsTable->rowCount()-1,1,item);
            }
            return true;
        }
    }

    return QWidget::eventFilter(obj,e);
}

void SqlDesigner::addRelation()
{
    currentScene->setMode(DiagramDocument::AddRelation);
}

void SqlDesigner::newDiagramDocument()
{
    if (currentScene != nullptr)
        delete currentScene;

    QDomElement dsElement;
    currentScene = addDiagramDocument(dsElement);
}

void SqlDesigner::removeDiagramDocument(int pageNo)
{
    if (pageNo < diagramDocumentList.size()) {
        if (diagramDocumentList[pageNo].document != nullptr)
            delete diagramDocumentList[pageNo].document;
        diagramDocumentList.removeAt(pageNo);
    }
}

void SqlDesigner::clearAll()
{
    for (auto &documentSet : diagramDocumentList)
        delete documentSet.document;
    diagramDocumentList.clear();
    m_currentPageNo = 0;
}

DiagramDocument* SqlDesigner::addDiagramDocument(QDomElement e)
{
    DocumentSet documentSet = newDocumentSet(e);
    diagramDocumentList.append(documentSet);
    return documentSet.document;
}

DocumentSet SqlDesigner::newDocumentSet(QDomElement e)
{
    auto scene = new DiagramDocument(this);
    scene->load(e);
    DocumentSet documentSet;
    documentSet.document = scene;
    documentSet.element = e;

    QUndoStack *undoStack = scene->undoStack();

    QRectF sceneRect = scene->sceneRect().united(QRectF(QPointF(0, 0), QPointF(100, 100)));
    scene->setSceneRect(sceneRect);
    QObject::connect(scene, SIGNAL(modeChanged(DiagramDocument::Mode)),
                     SLOT(updateMode(DiagramDocument::Mode)));
    QObject::connect(scene, SIGNAL(sqlChanged(QString)), this, SLOT(sqlChanged(QString)));
    QObject::connect(undoStack, SIGNAL(canUndoChanged(bool)), ui->actUndo, SLOT(setEnabled(bool)));
    QObject::connect(undoStack, SIGNAL(canRedoChanged(bool)), ui->actRedo, SLOT(setEnabled(bool)));

    ui->actUndo->setEnabled(undoStack->canUndo());
    ui->actRedo->setEnabled(undoStack->canRedo());
    ui->graphicsView->setScene(scene);

    return documentSet;
}

// Call on loading XML event
void SqlDesigner::loadDataSources(int pageNo, QVector<QDomElement> dsList)
{
    QDomElement dsElement;

    for (const auto &e : dsList) {
        if (e.attribute("type") == "SQL") {
            dsElement = e;
        }
        if (e.attribute("type") == "XML") {

        }
        if (e.attribute("type") == "INLINE") {
            elemINLINE = e;
        }
    }

    if (pageNo < diagramDocumentList.size()) {
        delete diagramDocumentList[pageNo].document;

        DocumentSet documentSet = newDocumentSet(dsElement);
        diagramDocumentList[pageNo] = documentSet;
    } else {
        addDiagramDocument(dsElement);
    }

    currentScene = diagramDocumentList[pageNo].document;
}

void SqlDesigner::setCurrentPage(int pageNo)
{
    if (m_currentPageNo != pageNo && m_currentPageNo >= 0 && m_currentPageNo < diagramDocumentList.size()) {
        diagramDocumentList[m_currentPageNo].document = currentScene;
        QDomElement e = buildDomElem();
        diagramDocumentList[m_currentPageNo].element = e;
    }

    if (pageNo < diagramDocumentList.size()) {
        currentScene = diagramDocumentList[pageNo].document;
        showDSData(diagramDocumentList[pageNo].element);

        ui->graphicsView->setScene(currentScene);
        auto undoStack = currentScene->undoStack();
        ui->actUndo->setEnabled(undoStack->canUndo());
        ui->actRedo->setEnabled(undoStack->canRedo());
    }

    m_currentPageNo = pageNo;
}

void SqlDesigner::showDSData(QDomElement e)
{
    ui->cmbType->clear();
    ui->cmbType->addItems(QSqlDatabase::drivers());
    //clear values
    ui->sqlEditor->setText("");
    ui->edtDBName->setText("");
    ui->edtHost->setText("");
    ui->edtUserName->setText("");
    ui->edtPassword->setText("");
    ui->edtConnectionCoding->setText("");
    ui->edtCharsetCoding->setText("");
    ui->edtConName->setText("");
    ui->edtPort->setText("");

    if (!e.isNull() && e.attribute("type") == "SQL") {
        ui->rbSql->setChecked(true);
        ui->stackedWidget->setCurrentIndex(1);
        ui->sqlEditor->setText(e.text().trimmed());
        ui->edtDBName->setText(e.attribute("dbName"));
        ui->edtHost->setText(e.attribute("dbHost"));
        ui->edtUserName->setText(e.attribute("dbUser"));
        ui->edtPassword->setText(e.attribute("dbPassword"));
        ui->edtConnectionCoding->setText(e.attribute("dbCoding"));
        ui->edtCharsetCoding->setText(e.attribute("charsetCoding"));
        ui->cmbType->setCurrentIndex(ui->cmbType->findText(e.attribute("dbType")));
        ui->edtConName->setText(e.attribute("dbConnectionName"));
        ui->edtPort->setText(e.attribute("dbPort"));
    } else if (!e.isNull() && e.attribute("type") == "XML") {
        ui->rbXml->isChecked();
        ui->stackedWidget->setCurrentIndex(2);
    } else {
        ui->rbCustom->setChecked(true);
        ui->stackedWidget->setCurrentIndex(0);
    }
}

void SqlDesigner::showDSData(int pageNo)
{
    m_currentPageNo = pageNo;
    showDSData(diagramDocumentList[pageNo].element);
}

void SqlDesigner::clearDiagram()
{
    newDiagramDocument();
}

void SqlDesigner::undo()
{
    auto undoStack = currentScene->undoStack();
    undoStack->undo();
}

void SqlDesigner::redo()
{
    auto undoStack = currentScene->undoStack();
    undoStack->redo();
}

void SqlDesigner::select()
{
    currentScene->setMode(DiagramDocument::Select);
}

void SqlDesigner::deleteSelected()
{
    currentScene->deleteSelectedItems();
    emit changed(true);
}

void SqlDesigner::updateMode(DiagramDocument::Mode mode)
{
    if (mode == DiagramDocument::Select)
        ui->actSelect->setChecked(true);
}

void SqlDesigner::sqlChanged(const QString value)
{
    ui->sqlEditor->clear();
    ui->sqlEditor->setText(value);
    emit changed(true);
}

SqlDesigner::~SqlDesigner()
{
    delete ui;
}
