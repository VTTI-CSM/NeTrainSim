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

#include "selectcolor.h"
#include "FldPropertyDlg.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"

EditorDelegate::EditorDelegate(QObject *parent)
: QStyledItemDelegate (parent)
{
    QObject::connect(this, SIGNAL(closeEditor(QWidget *, QAbstractItemDelegate::EndEditHint)),
                     this, SLOT(editorClose_(QWidget *, QAbstractItemDelegate::EndEditHint)));
}

void EditorDelegate::editorClose_(QWidget *editor, QAbstractItemDelegate::EndEditHint hint)
{
    Q_UNUSED(editor);
    Q_UNUSED(hint);

    emit editorClose(this);
}

QWidget* EditorDelegate::createEditor(QWidget *parent,
                                      const QStyleOptionViewItem &option,
                                      const QModelIndex &index) const
{
    if (index.column() == 1) {
        int command = index.model()->data(index, Qt::UserRole).toInt();
        switch(command) {
            case Left:
            case Top:
            case Width:
            case Height:
            case FontSize:
            case Length:
            case FrameWidth: {
                auto editor = new QSpinBox(parent);
                editor->setRange(0, 999999);
                connect(editor, SIGNAL(editingFinished()), this, SLOT(commitAndCloseEditor()));
                return editor;
            }
            case FontName: {
                auto editor = new QFontComboBox(parent);
                connect(editor, SIGNAL(activated(int)), this, SLOT(commitAndCloseEditor()));
                return editor;
            }
            case TextRotate: {
                auto editor = new QComboBox(parent);
                editor->addItem(tr("0 Degres"), 0);
                editor->addItem(tr("90 Degres"), 1);
                editor->addItem(tr("180 Degres"), 2);
                editor->addItem(tr("270 Degres"), 3);
                connect(editor, SIGNAL(activated(int)), this, SLOT(commitAndCloseEditor()));
                return editor;
            }
            case AligmentH: {
                auto editor = new QComboBox(parent);
                editor->addItem(tr("Left"),Qt::AlignLeft);
                editor->addItem(tr("Center"),Qt::AlignHCenter);
                editor->addItem(tr("Right"),Qt::AlignRight);
                editor->addItem(tr("Justify"),Qt::AlignJustify);
                connect(editor, SIGNAL(activated(int)), this, SLOT(commitAndCloseEditor()));
                return editor;
            }
            case AligmentV: {
                auto editor = new QComboBox(parent);
                editor->addItem(tr("Top"),Qt::AlignTop);
                editor->addItem(tr("Center"),Qt::AlignVCenter);
                editor->addItem(tr("Bottom"),Qt::AlignBottom);
                connect(editor, SIGNAL(activated(int)), this, SLOT(commitAndCloseEditor()));
                return editor;
            }
            case BarcodeType: {
                auto editor = new QComboBox(parent);
                for (const auto &pair : BarCode::getTypeList())
                    editor->addItem(pair.second, pair.first);
                connect(editor, SIGNAL(activated(int)), this, SLOT(commitAndCloseEditor()));
                return editor;
            }
            case BarcodeFrameType: {
                auto editor = new QComboBox(parent);
                for (const auto &pair : BarCode::getFrameTypeList())
                    editor->addItem(pair.second, pair.first);
                connect(editor, SIGNAL(activated(int)), this, SLOT(commitAndCloseEditor()));
                return editor;
            }
            case BorderColor:
            case FontColor:
            case BackgroundColor: {
                auto editor = new SelectColor(parent);
                QMargins margins(1,1,1,1);
                editor->setMargins(margins);
                QObject::connect(editor->button, SIGNAL(clicked()), this, SIGNAL(btnClicked()));
                return editor;
            }
            default: return QStyledItemDelegate::createEditor(parent, option, index);
        }
    }

    return nullptr;
}

void EditorDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    if (index.column() == 1) {
        int command = index.model()->data(index, Qt::UserRole).toInt();
        switch (command) {
            case BarcodeType:
            case BarcodeFrameType:
            case TextRotate:
            case AligmentH:
            case AligmentV: {
                auto value = index.model()->data(index, Qt::EditRole).toString();
                auto ed = qobject_cast<QComboBox*>(editor);
                ed->setCurrentIndex(ed->findText(value));
                break;
            }
            case FontName: {
                auto value = index.model()->data(index, Qt::EditRole).toString();
                auto ed = qobject_cast<QFontComboBox*>(editor);
                ed->setCurrentFont(QFont(value));
                break;
            }
            case BorderColor:
            case FontColor:
            case BackgroundColor: {
                auto value = index.model()->data(index, Qt::EditRole).toString();
                auto ed = qobject_cast<SelectColor*>(editor);
                ed->setBackGroundColor(value);
                break;
            }
            default: QStyledItemDelegate::setEditorData(editor,index);
        }
    } else return;
}

void EditorDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex& index) const
{
    if (index.column() == 1) {
        int command = index.model()->data(index, Qt::UserRole).toInt();
        switch (command) {
            case BarcodeFrameType:
            case BarcodeType: {
                auto ed = qobject_cast<QComboBox*>(editor);
                model->setData(index, ed->itemData(ed->currentIndex()));
                break;
            }
            case TextRotate:
            case AligmentH:
            case AligmentV: {
                auto ed = qobject_cast<QComboBox*>(editor);
                model->setData(index, ed->currentIndex());
                break;
            }
            case FontName: {
                auto ed = qobject_cast<QFontComboBox*>(editor);
                model->setData(index, ed->currentFont().family());
                break;
            }
            case FontSize: {
                QStyledItemDelegate::setModelData(editor,model,index);
                break;
            }
            case BorderColor:
            case FontColor:
            case BackgroundColor: {
                auto ed = qobject_cast<SelectColor*>(editor);
                model->setData(index, ed->getBackGroundColor(), Qt::EditRole);
                QBrush brush(Qt::white);
                model->setData(index, brush, Qt::ForegroundRole);
                break;
            }
            default: QStyledItemDelegate::setModelData(editor,model,index);
        }
    } return;
}

void EditorDelegate::paint ( QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index ) const
{
    if (index.column() == 1) {
        if (option.state & QStyle::State_Active) {
            QStyledItemDelegate::paint(painter,option,index);
        } else {
            int command = index.model()->data(index, Qt::UserRole).toInt();
            switch (command) {
                case BorderColor:
                case FontColor:
                case BackgroundColor: {
                    QRect rect = option.rect;
                    rect.setHeight(rect.height()-2);
                    rect.setY(rect.y()+1);
                    rect.setWidth(rect.width()-4);
                    rect.setX(rect.x()+2);
                    QString value = index.model()->data(index, Qt::EditRole).toString();
                    painter->fillRect(rect,colorFromString(value));
                    break;
                }
                default:
                    QStyledItemDelegate::paint(painter,option,index);
            }
        }
    }
    else
        QStyledItemDelegate::paint(painter,option,index);
}

void EditorDelegate::commitAndCloseEditor()
{
    //auto editor = qobject_cast<QWidget*>(sender());
    //emit commitData(editor);
    //emit closeEditor(editor);
}


MainWindow* MainWindow::mw;

MainWindow::MainWindow(QStringList args, QWidget *parent)
: QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    MainWindow::mw = this;

    QSettings settings(QCoreApplication::applicationDirPath()+"/setting.ini",QSettings::IniFormat);
    #if QT_VERSION < QT_VERSION_CHECK(6,0,0)
        settings.setIniCodec("UTF-8");
    #endif

    dontSelect = false;
    m_args = args;

    this->setProperty("AllowStart", true);

    auto spacerWidget = new QWidget(this);
    spacerWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    spacerWidget->setVisible(true);

    auto lblLogo = new QLabel("", this);
    lblLogo->setObjectName("lblLogo");


    ui->mainToolBar->addWidget(spacerWidget);
    ui->mainToolBar->addWidget(lblLogo);

    m_status1 = new QLabel("Left", this);
    m_status1->setText(QString("X: %1 Y: %2").arg(0).arg(0));
    m_status1->setFixedWidth(150);
    m_status1->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    m_status2 = new QLabel("Middle", this);
    m_status2->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    m_status3 = new QLabel("Right", this);
    m_status3->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    this->statusBar()->addPermanentWidget(m_status1, 0);
    this->statusBar()->addPermanentWidget(m_status2, 1);
    this->statusBar()->addPermanentWidget(m_status3, 2);
    this->setWindowState(this->windowState() ^ Qt::WindowMaximized);


    ui->treeParams->setColumnWidth(0,150);
    ui->treeParams->setColumnWidth(1,70);
    ui->treeParams->setFocusPolicy(Qt::NoFocus);
    ui->treeWidget->setFocusPolicy(Qt::NoFocus);

    auto d = new EditorDelegate(ui->treeParams);
    QObject::connect(d, SIGNAL(editorClose(QStyledItemDelegate *)), this,  SLOT(closeEditor()));
    QObject::connect(d, SIGNAL(btnClicked()), this, SLOT(chooseColor()));
    ui->treeParams->setItemDelegate(d);

    cbFontName = new QFontComboBox(ui->toolBar);
    cbFontName->setObjectName("cbFontName");
    cbFontName->setEditable(true);
    cbFontName->setToolTip(tr("Font name"));

    cbFontSize = new QComboBox( ui->toolBar );
    cbFontSize->setObjectName("cbFontSize");
    cbFontSize->setEditable(true);
    cbFontSize->setToolTip(tr("Font size"));
    cbFontSize->addItem("6");
    cbFontSize->addItem("7");
    cbFontSize->addItem("8");
    cbFontSize->addItem("9");
    cbFontSize->addItem("10");
    cbFontSize->addItem("11");
    cbFontSize->addItem("12");
    cbFontSize->addItem("14");
    cbFontSize->addItem("16");
    cbFontSize->addItem("18");
    cbFontSize->addItem("20");
    cbFontSize->addItem("22");
    cbFontSize->addItem("24");
    cbFontSize->addItem("26");
    cbFontSize->addItem("28");
    cbFontSize->addItem("36");
    cbFontSize->addItem("48");
    cbFontSize->addItem("72");
    ui->toolBar->insertWidget(ui->actionBold,cbFontName);
    ui->toolBar->insertWidget(ui->actionBold,cbFontSize);

    cbFontSize->setCurrentIndex(cbFontSize->findText(settings.value("FontSize", 8).toString()));
    cbFontName->setCurrentIndex(cbFontName->findText(settings.value("FontName", "MS Shell Dlg 2").toString()));

    cbZoom = new QComboBox(ui->mainToolBar);
    cbZoom->setEditable(true);
    cbZoom->setFixedWidth(70);
    cbZoom->setToolTip(tr("Zoom"));
    cbZoom->addItem("50%");
    cbZoom->addItem("75%");
    cbZoom->addItem("100%");
    cbZoom->addItem("125%");
    cbZoom->addItem("150%");
    cbZoom->addItem("175%");
    cbZoom->addItem("200%");
    cbZoom->addItem("300%");
    cbZoom->setCurrentIndex(2);

    ui->mainToolBar->insertWidget(ui->actFieldLeft,cbZoom);
    ui->mainToolBar->insertSeparator(ui->actFieldLeft);

    cbFrameWidth = new QComboBox( ui->toolBar );
    cbFrameWidth->setToolTip(tr("Frame width"));
    cbFrameWidth->addItem("1");
    cbFrameWidth->addItem("2");
    cbFrameWidth->addItem("3");
    cbFrameWidth->addItem("4");
    cbFrameWidth->addItem("5");
    cbFrameWidth->addItem("6");
    cbFrameWidth->addItem("7");
    cbFrameWidth->addItem("8");
    cbFrameWidth->addItem("9");
    cbFrameWidth->addItem("10");
    ui->toolBar->addWidget(cbFrameWidth);

    QObject::connect(cbFrameWidth, SIGNAL(activated(int)), this, SLOT(changeFrameWidth()));
    QObject::connect(cbFontName, SIGNAL(activated(int)), this, SLOT(changeTextFont()));
    QObject::connect(cbFontSize, SIGNAL(activated(int)), this, SLOT(changeTextFont()));
    QObject::connect(cbZoom, SIGNAL(activated(int)), this, SLOT(changeZoom()));

    listFrameStyle = new QListWidget(this);
    listFrameStyle->hide();
    listFrameStyle->setFixedHeight(116);
    listFrameStyle->setIconSize(QSize(85, 16));
    QObject::connect(listFrameStyle, SIGNAL(itemClicked(QListWidgetItem *)), this, SLOT(setFrameStyle(QListWidgetItem *)));

    for (int i=1; i < 7; i++) {
        auto item = new QListWidgetItem(listFrameStyle);
        QIcon icn;
        icn.addPixmap(QPixmap(QString::fromUtf8(":/new/prefix1/images/fs%1.png").arg(i)), QIcon::Normal, QIcon::On);
        item->setIcon(icn);
        item->setData(Qt::UserRole,i);
        listFrameStyle->addItem(item);
    }

    this->setMouseTracking(true);

    rootItem = new QTreeWidgetItem(ui->treeWidget,0);
    icon.addPixmap(QPixmap(QString::fromUtf8(":/new/prefix1/images/report.png")), QIcon::Normal, QIcon::On);
    rootItem->setIcon(0,icon);
    rootItem->setText(0,tr("Report"));
    rootItem->setExpanded(true);



    cloneContList = new QList<QGraphicsItem*>();
    for (auto &widget : ui->toolBar->findChildren<QWidget*>())
        widget->installEventFilter(this);

    auto alignmentHGroup = new QActionGroup(this);
    alignmentHGroup->addAction(ui->actAlignLeft);
    alignmentHGroup->addAction(ui->actAlignRight);
    alignmentHGroup->addAction(ui->actAlignJustify);
    alignmentHGroup->addAction(ui->actAlignCenter);

    auto alignmentVGroup = new QActionGroup(this);
    alignmentVGroup->addAction(ui->actAlignTop);
    alignmentVGroup->addAction(ui->actAlignBottom);
    alignmentVGroup->addAction(ui->actAlignVCenter);

    auto addGroup = new QActionGroup(this);
    addGroup->addAction(ui->actionInsert_band);
    addGroup->addAction(ui->actSelect_tool);
    addGroup->addAction(ui->actAddField);
    addGroup->addAction(ui->actAddPicture);
    addGroup->addAction(ui->actMagnifying);
    addGroup->addAction(ui->actAddDiagram);
    addGroup->addAction(ui->actAddDrawing);
    addGroup->addAction(ui->actAddBarcode);
    addGroup->addAction(ui->actAddRichText);

    ui->actSelect_tool->setChecked(true);

    for (quint16 i = 0; i < MaxRecentFiles; ++i) {
        recentFileActs[i] = new QAction(this);
        recentFileActs[i]->setVisible(false);
        ui->menuFile->insertAction(ui->actionExit,recentFileActs[i]);
        connect(recentFileActs[i], SIGNAL(triggered()), this, SLOT(openRecentFile()));
    }

    separatorAct = ui->menuFile->addSeparator();

    updateRecentFileActions();
    ui->menuFile->insertAction(ui->actionExit,separatorAct);

    QObject::connect(ui->actUndo, SIGNAL(triggered()), this, SLOT(undo()));
    QObject::connect(ui->actRedo, SIGNAL(triggered()), this, SLOT(redo()));
    QObject::connect(ui->actSelect_tool, SIGNAL(triggered()), this, SLOT(setSelectionMode()));
    QObject::connect(ui->actAddDiagram, &QAction::triggered, [=] { addField(Diagram); });
    QObject::connect(ui->actAddPicture, &QAction::triggered, [=] { addField(Image); });
    QObject::connect(ui->actAddBarcode, &QAction::triggered, [=] { addField(Barcode); });
    QObject::connect(ui->actAddCrossTab, &QAction::triggered, [=] { addField(CrossTab); });
    QObject::connect(ui->actAddRichText, &QAction::triggered, [=] { addField(TextRich); });
    QObject::connect(ui->actAddField, &QAction::triggered, [=] { addField(Text); });
    QObject::connect(ui->actGroup, SIGNAL(triggered()), this, SLOT(setGroupingField()));
    QObject::connect(ui->actUngroup, SIGNAL(triggered()), this, SLOT(setGroupingField()));
    QObject::connect(ui->actionOpenReport, SIGNAL(triggered()), this, SLOT(openFile()));
    QObject::connect(ui->actionNewReport, SIGNAL(triggered()), this, SLOT(newReport()));
    QObject::connect(ui->actSaveReport, SIGNAL(triggered()), this, SLOT(saveReport()));
    QObject::connect(ui->actionExit, SIGNAL(triggered()), this, SLOT(closeProgram()));
    QObject::connect(ui->actSaveAs, SIGNAL(triggered()), this, SLOT(saveReport()));
    QObject::connect(ui->actAlignCenter, SIGNAL(triggered()), this, SLOT(changeTextFont()));
    QObject::connect(ui->actAlignLeft, SIGNAL(triggered()), this, SLOT(changeTextFont()));
    QObject::connect(ui->actAlignRight, SIGNAL(triggered()), this, SLOT(changeTextFont()));
    QObject::connect(ui->actAlignJustify, SIGNAL(triggered()), this, SLOT(changeTextFont()));
    QObject::connect(ui->actAlignTop, SIGNAL(triggered()), this, SLOT(changeTextFont()));
    QObject::connect(ui->actAlignVCenter, SIGNAL(triggered()), this, SLOT(changeTextFont()));
    QObject::connect(ui->actAlignBottom, SIGNAL(triggered()), this, SLOT(changeTextFont()));
    QObject::connect(ui->actionBold, SIGNAL(triggered()), this, SLOT(changeTextFont()));
    QObject::connect(ui->actionItalic, SIGNAL(triggered()), this, SLOT(changeTextFont()));
    QObject::connect(ui->actionUnderline, SIGNAL(triggered()), this, SLOT(changeTextFont()));
    QObject::connect(ui->actionStrikeout, SIGNAL(triggered()), this, SLOT(changeTextFont()));
    QObject::connect(ui->actLineAll, SIGNAL(triggered()), this, SLOT(changeTextFont()));
    QObject::connect(ui->actLineBottom, SIGNAL(triggered()), this, SLOT(changeTextFont()));
    QObject::connect(ui->actLineLeft, SIGNAL(triggered()), this, SLOT(changeTextFont()));
    QObject::connect(ui->actLineNo, SIGNAL(triggered()), this, SLOT(changeTextFont()));
    QObject::connect(ui->actLineRight, SIGNAL(triggered()), this, SLOT(changeTextFont()));
    QObject::connect(ui->actLineTop, SIGNAL(triggered()), this, SLOT(changeTextFont()));
    QObject::connect(ui->actCut, SIGNAL(triggered()), this, SLOT(clipBoard()));
    QObject::connect(ui->actCopy, SIGNAL(triggered()), this, SLOT(clipBoard()));
    QObject::connect(ui->actPaste, SIGNAL(triggered()), this, SLOT(clipBoard()));
    QObject::connect(ui->actBackgroundColor, SIGNAL(triggered()), this, SLOT(chooseColor()));
    QObject::connect(ui->actBorderColor, SIGNAL(triggered()), this, SLOT(chooseColor()));
    QObject::connect(ui->actFontColor, SIGNAL(triggered()), this, SLOT(chooseColor()));    
    QObject::connect(ui->actAbout, SIGNAL(triggered()), this, SLOT(showAbout()));
    QObject::connect(ui->actNewReportPage, SIGNAL(triggered()), this, SLOT(newReportPage()));
    QObject::connect(ui->actDeleteReportPage, SIGNAL(triggered()), this, SLOT(deleteReportPage()));
    //QObject::connect(ui->actFrameStyle, SIGNAL(triggered()), this, SLOT(showFrameStyle()));
    QObject::connect(ui->treeWidget, SIGNAL(itemClicked(QTreeWidgetItem *, int)), this, SLOT(selTree(QTreeWidgetItem *, int)));
    QObject::connect(ui->treeParams, SIGNAL(itemClicked(QTreeWidgetItem *, int)), this, SLOT(itemChanged(QTreeWidgetItem *, int)));
    QObject::connect(ui->tabWidget, SIGNAL(currentChanged(int)), this, SLOT(reportPageChanged(int)));
    QObject::connect(ui->actFieldBottom, SIGNAL(triggered()), this, SLOT(alignFields()));
    QObject::connect(ui->actFieldCenter, SIGNAL(triggered()), this, SLOT(alignFields()));
    QObject::connect(ui->actFieldLeft, SIGNAL(triggered()), this, SLOT(alignFields()));
    QObject::connect(ui->actFieldMiddle, SIGNAL(triggered()), this, SLOT(alignFields()));
    QObject::connect(ui->actFieldRight, SIGNAL(triggered()), this, SLOT(alignFields()));
    QObject::connect(ui->actFieldTop, SIGNAL(triggered()), this, SLOT(alignFields()));
    QObject::connect(ui->actFieldSameHeight, SIGNAL(triggered()), this, SLOT(alignFields()));
    QObject::connect(ui->actFieldSameWidth, SIGNAL(triggered()), this, SLOT(alignFields()));
    QObject::connect(ui->actSettings, SIGNAL(triggered()), this, SLOT(showSetting()));
    QObject::connect(ui->actGroupProperty, SIGNAL(triggered()), this, SLOT(openDBGroupProperty()));
    QObject::connect(ui->actCheckUpdates, SIGNAL(triggered()), this, SLOT(checkUpdates()));
    QObject::connect(ui->actPageSettings, SIGNAL(triggered()), this, SLOT(showPageSetting()));
    QObject::connect(ui->actPreview, SIGNAL(triggered()), this, SLOT(showPreview()));
    QObject::connect(ui->actDataSource, SIGNAL(triggered()), this, SLOT(showPane()));
    QObject::connect(ui->actScriptEditing, SIGNAL(triggered()), this, SLOT(showPane()));
    QObject::connect(ui->actReadmeQtRPT, &QAction::triggered, [=] {
        QDesktopServices::openUrl(QUrl::fromLocalFile(QCoreApplication::applicationDirPath()+"/ReadmeQtRPT.pdf"));
    });
    QObject::connect(ui->actReadmeQtRptDesigner, &QAction::triggered, [=] {
        QDesktopServices::openUrl(QUrl::fromLocalFile(QCoreApplication::applicationDirPath()+"/ReadmeQtRptDesigner.pdf"));
    });


    actRepTitle = new QAction(tr("Report Title"),this);
    actRepTitle->setObjectName("actRepTitle");
    icon.addPixmap(QPixmap(QString::fromUtf8(":/new/prefix1/images/reportTitle.png")), QIcon::Normal, QIcon::On);
    actRepTitle->setIcon(icon);
    QObject::connect(actRepTitle, SIGNAL(triggered()), this, SLOT(addBand()));

    actReportSummary = new QAction(tr("Report Summary"),this);
    actReportSummary->setObjectName("actReportSummary");
    icon.addPixmap(QPixmap(QString::fromUtf8(":/new/prefix1/images/reportFooter.png")), QIcon::Normal, QIcon::On);
    actReportSummary->setIcon(icon);
    QObject::connect(actReportSummary, SIGNAL(triggered()), this, SLOT(addBand()));

    actPageHeader = new QAction(tr("Page Header"),this);
    actPageHeader->setObjectName("actPageHeader");
    icon.addPixmap(QPixmap(QString::fromUtf8(":/new/prefix1/images/pageTitle.png")), QIcon::Normal, QIcon::On);
    actPageHeader->setIcon(icon);
    QObject::connect(actPageHeader, SIGNAL(triggered()), this, SLOT(addBand()));

    actPageFooter = new QAction(tr("Page Footer"),this);
    actPageFooter->setObjectName("actPageFooter");
    icon.addPixmap(QPixmap(QString::fromUtf8(":/new/prefix1/images/pageFooter.png")), QIcon::Normal, QIcon::On);
    actPageFooter->setIcon(icon);
    QObject::connect(actPageFooter, SIGNAL(triggered()), this, SLOT(addBand()));

    actMasterData = new QAction(tr("Master Data"),this);
    actMasterData->setObjectName("actMasterData");
    icon.addPixmap(QPixmap(QString::fromUtf8(":/new/prefix1/images/data.png")), QIcon::Normal, QIcon::On);
    actMasterData->setIcon(icon);
    QObject::connect(actMasterData, SIGNAL(triggered()), this, SLOT(addBand()));

    actDataGroupingHeader = new QAction(tr("Data Grouping Header"),this);
    actDataGroupingHeader->setObjectName("actDataGroupingHeader");
    icon.addPixmap(QPixmap(QString::fromUtf8(":/new/prefix1/images/dataGroupingHeader.png")), QIcon::Normal, QIcon::On);
    actDataGroupingHeader->setIcon(icon);
    QObject::connect(actDataGroupingHeader, SIGNAL(triggered()), this, SLOT(addBand()));
    actDataGroupingHeader->setEnabled(false);

    actDataGroupingFooter = new QAction(tr("Data Grouping Footer"),this);
    actDataGroupingFooter->setObjectName("actDataGroupingFooter");
    icon.addPixmap(QPixmap(QString::fromUtf8(":/new/prefix1/images/dataGroupingFooter.png")), QIcon::Normal, QIcon::On);
    actDataGroupingFooter->setIcon(icon);
    QObject::connect(actDataGroupingFooter, SIGNAL(triggered()), this, SLOT(addBand()));
    //actDataGroupingFooter->setEnabled(false);

    actMasterHeader = new QAction(tr("Master Header"),this);
    actMasterHeader->setObjectName("actMasterHeader");
    icon.addPixmap(QPixmap(QString::fromUtf8(":/new/prefix1/images/dataTitle.png")), QIcon::Normal, QIcon::On);
    actMasterHeader->setIcon(icon);
    QObject::connect(actMasterHeader, SIGNAL(triggered()), this, SLOT(addBand()));

    actMasterFooter = new QAction(tr("Master Footer"),this);
    actMasterFooter->setObjectName("actMasterFooter");
    icon.addPixmap(QPixmap(QString::fromUtf8(":/new/prefix1/images/dataFooter.png")), QIcon::Normal, QIcon::On);
    actMasterFooter->setIcon(icon);
    QObject::connect(actMasterFooter, SIGNAL(triggered()), this, SLOT(addBand()));

    //Menu for selecting bands type
    auto subBand1 = new QMenu(this);
    subBand1->setObjectName("subBand1");
    subBand1->addAction(actRepTitle);
    subBand1->addAction(actPageHeader);
    subBand1->addAction(actDataGroupingHeader);
    subBand1->addAction(actMasterHeader);
    subBand1->addAction(actMasterData);
    subBand1->addAction(actMasterFooter);
    subBand1->addAction(actDataGroupingFooter);
    subBand1->addAction(actPageFooter);
    subBand1->addAction(actReportSummary);

    ui->actionInsert_band->setMenu(subBand1);
    auto btn1 = qobject_cast<QToolButton *>(ui->toolBar_3->widgetForAction(ui->actionInsert_band));
    btn1->setPopupMode(QToolButton::InstantPopup);
    QObject::connect(subBand1, SIGNAL(aboutToShow()), this, SLOT(clickOnTBtn()));


    //Actions for drawing
    auto actDrawLine2 = new QAction(tr("Line"),this);
    actDrawLine2->setObjectName("actDrawLine2");
    icon.addPixmap(QPixmap(QString::fromUtf8(":/new/prefix1/images/line2.png")), QIcon::Normal, QIcon::On);
    actDrawLine2->setIcon(icon);
    QObject::connect(actDrawLine2, SIGNAL(triggered()), this, SLOT(addDraw()));

    auto actDrawLine3 = new QAction(tr("Line with arrow at the end"),this);
    actDrawLine3->setObjectName("actDrawLine3");
    icon.addPixmap(QPixmap(QString::fromUtf8(":/new/prefix1/images/line3.png")), QIcon::Normal, QIcon::On);
    actDrawLine3->setIcon(icon);
    QObject::connect(actDrawLine3, SIGNAL(triggered()), this, SLOT(addDraw()));

    auto actDrawLine4 = new QAction(tr("Line with arrow at the start"),this);
    actDrawLine4->setObjectName("actDrawLine4");
    icon.addPixmap(QPixmap(QString::fromUtf8(":/new/prefix1/images/line4.png")), QIcon::Normal, QIcon::On);
    actDrawLine4->setIcon(icon);
    QObject::connect(actDrawLine4, SIGNAL(triggered()), this, SLOT(addDraw()));

    auto actDrawLine5 = new QAction(tr("Line with arrows at both side"),this);
    actDrawLine5->setObjectName("actDrawLine5");
    icon.addPixmap(QPixmap(QString::fromUtf8(":/new/prefix1/images/line5.png")), QIcon::Normal, QIcon::On);
    actDrawLine5->setIcon(icon);
    QObject::connect(actDrawLine5, SIGNAL(triggered()), this, SLOT(addDraw()));

    auto actDrawRectangle = new QAction(tr("Rectangle"),this);
    actDrawRectangle->setObjectName("actDrawRectangle");
    icon.addPixmap(QPixmap(QString::fromUtf8(":/new/prefix1/images/rectangle.png")), QIcon::Normal, QIcon::On);
    actDrawRectangle->setIcon(icon);
    QObject::connect(actDrawRectangle, SIGNAL(triggered()), this, SLOT(addDraw()));

    auto actDrawRoundedRectangle = new QAction(tr("Rounded rectangle"),this);
    actDrawRoundedRectangle->setObjectName("actDrawRoundedRectangle");
    icon.addPixmap(QPixmap(QString::fromUtf8(":/new/prefix1/images/roundedReactangle.png")), QIcon::Normal, QIcon::On);
    actDrawRoundedRectangle->setIcon(icon);
    QObject::connect(actDrawRoundedRectangle, SIGNAL(triggered()), this, SLOT(addDraw()));

    auto actDrawEllipse = new QAction(tr("Ellipse"),this);
    actDrawEllipse->setObjectName("actDrawEllipse");
    icon.addPixmap(QPixmap(QString::fromUtf8(":/new/prefix1/images/ellipse.png")), QIcon::Normal, QIcon::On);
    actDrawEllipse->setIcon(icon);
    QObject::connect(actDrawEllipse, SIGNAL(triggered()), this, SLOT(addDraw()));

    auto actDrawTriangle = new QAction(tr("Triangle"),this);
    actDrawTriangle->setObjectName("actDrawTriangle");
    icon.addPixmap(QPixmap(QString::fromUtf8(":/new/prefix1/images/triangle.png")), QIcon::Normal, QIcon::On);
    actDrawTriangle->setIcon(icon);
    QObject::connect(actDrawTriangle, SIGNAL(triggered()), this, SLOT(addDraw()));

    auto actDrawRhombus = new QAction(tr("Rhombus"),this);
    actDrawRhombus->setObjectName("actDrawRhombus");
    icon.addPixmap(QPixmap(QString::fromUtf8(":/new/prefix1/images/rhombus.png")), QIcon::Normal, QIcon::On);
    actDrawRhombus->setIcon(icon);
    QObject::connect(actDrawRhombus, SIGNAL(triggered()), this, SLOT(addDraw()));

    //Menu for selecting drawing
    auto subBand2 = new QMenu(this);
    subBand2->setObjectName("subBand2");
    //subBand2->addAction(actDrawLine1);
    subBand2->addAction(actDrawLine2);
    subBand2->addAction(actDrawLine3);
    subBand2->addAction(actDrawLine4);
    subBand2->addAction(actDrawLine5);
    subBand2->addAction(actDrawRectangle);
    subBand2->addAction(actDrawRoundedRectangle);
    subBand2->addAction(actDrawEllipse);
    subBand2->addAction(actDrawTriangle);
    subBand2->addAction(actDrawRhombus);    

    ui->actAddDrawing->setMenu(subBand2);
    auto btn2 = qobject_cast<QToolButton *>(ui->toolBar_3->widgetForAction(ui->actAddDrawing));
    btn2->setPopupMode(QToolButton::InstantPopup);
    QObject::connect(subBand2, SIGNAL(aboutToShow()), this, SLOT(clickOnTBtn()));

    //Menu for container
    contMenu = new QMenu(this);
    contMenu->addAction(ui->actCut);
    contMenu->addAction(ui->actCopy);
    contMenu->addAction(ui->actPaste);
    contMenu->setFocusPolicy(Qt::NoFocus);
    contMenu->addSeparator();

    //Menu for Data Group Band
    bandMenu = new QMenu(this);
    bandMenu->addAction(ui->actPaste);
    bandMenu->addSeparator();
    bandMenu->addAction(ui->actGroupProperty);
    bandMenu->setFocusPolicy(Qt::NoFocus);
    bandMenu->addSeparator();

    xmlDoc = QSharedPointer<QDomDocument>(new QDomDocument("Report"));

    scriptEditor = new ScriptEditor(xmlDoc, this);
    scriptEditor->setVisible(false);


    sqlDesigner = new SqlDesigner(xmlDoc, this);
    QDomElement dsElement;
    sqlDesigner->addDiagramDocument(dsElement);
    sqlDesigner->setVisible(false);
    QObject::connect(sqlDesigner, SIGNAL(changed(bool)), ui->actSaveReport, SLOT(setEnabled(bool)));


    stackedWidget = new QStackedWidget(this);
    stackedWidget->setObjectName(QStringLiteral("stackedWidget"));
    stackedWidget->setMinimumWidth(200);
    stackedWidget->setMaximumWidth(800);
    stackedWidget->setVisible(false);
    ui->horizontalLayout->addWidget(stackedWidget);

    stackedWidget->addWidget(scriptEditor);
    stackedWidget->addWidget(sqlDesigner);

    ui->splitter->addWidget(stackedWidget);

    QList<int> lst;
    lst << 15 << 400 << 15;
    ui->splitter->setSizes(lst);

    ui->actShowGrid->setChecked(settings.value("ShowGrid",true).toBool());

    newReportPage();

    if (m_args.size() < 2 && settings.value("CheckUpdates",true).toBool())
        checkUpdates();

    this->installEventFilter(this);

    loadPlugin();

    if (this->property("AllowStart").toBool() == false) {
        while (!pluginsLoaders.isEmpty()) {
            auto pluginLoader = pluginsLoaders.takeFirst();
            pluginLoader->unload();
            delete pluginLoader;
        }

        qApp->quit();
    }
}

void MainWindow::runSilentMode()
{
    for (const auto &arg : m_args) {
        if (arg.toLower().contains(".pdf"))
            pdfName = arg;
        if (arg.toLower().contains(".xml"))
            fileName = arg;
    }

    if (!fileName.isEmpty() && !pdfName.isEmpty()) {
        openFile();
        showPreview();
    }
}

#include "CustomInterface.h"
bool MainWindow::loadPlugin()
{
    QDir pluginsDir(qApp->applicationDirPath() + "/plugins");
#if defined(Q_OS_WIN)
    //if (pluginsDir.dirName().toLower() == "debug" || pluginsDir.dirName().toLower() == "release")
    //    pluginsDir.cdUp();
#elif defined(Q_OS_MAC)
    if (pluginsDir.dirName() == "MacOS") {
        pluginsDir.cdUp();
        pluginsDir.cdUp();
        pluginsDir.cdUp();
    }
#endif
    //pluginsDir.cdUp();
    //pluginsDir.cd("plugins");

    QStringList filters;
    filters << "*.dll" << "*.so" << "*.dylib";
    pluginsDir.setNameFilters(filters);

    QMenu *menuPlugins = nullptr;
    if (pluginsDir.entryList(QDir::Files).size() > 0) {
        menuPlugins = new QMenu("Plugins", ui->menuService);
        ui->menuService->addMenu(menuPlugins);
    }

    for (QString &fileName : pluginsDir.entryList(QDir::Files)) {
        auto pluginLoader = new QPluginLoader(pluginsDir.absoluteFilePath(fileName), this);
        auto plugin = pluginLoader->instance();

        qDebug() << fileName << plugin;
        if (plugin) {
            plugin->setParent(this);
            plugins << plugin;
            pluginsLoaders << pluginLoader;

            auto echoInterface = qobject_cast<CustomInterface *>(plugin);

            int index = plugin->metaObject()->indexOfClassInfo("AddToMenu");
            if (QString(plugin->metaObject()->classInfo(index).value()) == "true") {
                index = plugin->metaObject()->indexOfClassInfo("PluginName");
                QString pluginName = plugin->metaObject()->classInfo(index).value();

                if (echoInterface) {
                    auto act = new QAction(pluginName, menuPlugins);
                    QObject::connect(act, &QAction::triggered, [=] {
                        echoInterface->execute(xmlDoc);
                        echoInterface->saveData(xmlDoc);

                        ui->actSaveReport->setEnabled(true);
                    });

                    menuPlugins->addAction(act);
                }
            }

            index = plugin->metaObject()->indexOfClassInfo("RunOnLoading");
            if (QString(plugin->metaObject()->classInfo(index).value()) == "true")
                if (echoInterface)
                    echoInterface->execute(xmlDoc);
        }
    }

    return true;
}

void MainWindow::checkUpdates()
{
    XYZDownloadManager dl(this);
    dl.setParent(this);
    QString urlVersion = "http://netix.dl.sourceforge.net/project/qtrpt/version.txt";
    dl.setTarget(urlVersion);
    dl.download(true);

    QEventLoop loop;
    QObject::connect(&dl,SIGNAL(done()),&loop,SLOT(quit()));
    loop.exec();
}

void MainWindow::openDBGroupProperty()
{
    auto repPage = qobject_cast<RepScrollArea *>(ui->tabWidget->currentWidget());
    auto band = qgraphicsitem_cast<ReportBand *>(repPage->scene->selectedItems().at(0));

    QScopedPointer<FldPropertyDlg> dlg(new FldPropertyDlg(this));
    if (band != nullptr && band->bandType == MasterData) {
        dlg->showThis(1,band);
        setParamTree(StartNewNumeration, band->getStartNewNumertaion());
        setParamTree(StartNewPage, band->getStartNewPage());
        setParamTree(GroupFields, band->getGroupingField());
        setReportChanged();
    }
}

void MainWindow::updateRecentFileActions()
{
    QSettings settings(QCoreApplication::applicationDirPath()+"/setting.ini",QSettings::IniFormat);
    #if QT_VERSION < QT_VERSION_CHECK(6,0,0)
        settings.setIniCodec("UTF-8");
    #endif
    QStringList files = settings.value("recentFileList").toStringList();

    int numRecentFiles = qMin(files.size(), (int)MaxRecentFiles);

    for (quint16 i = 0; i < numRecentFiles; ++i) {
        QString text = tr("&%1 %2").arg(i + 1).arg(QFileInfo(files[i]).fileName());
        recentFileActs[i]->setText(text);
        recentFileActs[i]->setData(files[i]);
        recentFileActs[i]->setVisible(true);
    }
    for (int j = numRecentFiles; j < MaxRecentFiles; ++j)
        recentFileActs[j]->setVisible(false);

    separatorAct->setVisible(numRecentFiles > 0);
}

void MainWindow::openRecentFile()
{
    auto action = qobject_cast<QAction *>(sender());
    if (action) {
        fileName = action->data().toString();
        openFile();
    }
}

void MainWindow::setCurrentFile(const QString &fileName)
{
    setWindowFilePath(fileName);

    QSettings settings(QCoreApplication::applicationDirPath()+"/setting.ini",QSettings::IniFormat);
    #if QT_VERSION < QT_VERSION_CHECK(6,0,0)
        settings.setIniCodec("UTF-8");
    #endif
    QStringList files = settings.value("recentFileList").toStringList();
    files.removeAll(fileName);
    files.prepend(fileName);

    while (files.size() > MaxRecentFiles)
        files.removeLast();

    settings.setValue("recentFileList", files);
    updateRecentFileActions();
}

void MainWindow::itemResizing(QGraphicsItem *item)
{
    if (item->type() == ItemType::GBox || item->type() == ItemType::GBand) {
        auto box = qgraphicsitem_cast<GraphicsBox*>(item);
        setParamTree(Height, box->getHeight());

        if (item->type() == ItemType::GBand) {
            auto repPage = qobject_cast<RepScrollArea *>(ui->tabWidget->widget(ui->tabWidget->currentIndex()));
            repPage->correctBandGeom(nullptr);
            setParamTree(Height, box->getHeight() - ReportBand::titleHeight);
        } else if (item->type() == ItemType::GBox) {
            auto band = qgraphicsitem_cast<ReportBand *>(item->parentItem());
            setParamTree(Width, box->getWidth());
            setParamTree(Top, box->pos().y() - band->titleHeight);
            setParamTree(Left, box->pos().x());
        }
    } else if (item->type() == ItemType::GLine) {
        auto line = static_cast<GraphicsLine*>(item);
        setParamTree(Length, (int)line->getLength());
    }

    setReportChanged();
}

void MainWindow::changeFrameWidth()
{
    listFrameStyle->close();
    execButtonCommand(FrameWidth,this->cbFrameWidth->currentText().toInt());
}

void MainWindow::showFrameStyle(QPoint pos)
{
    pos.setY(pos.y()+81);
    pos.setX(pos.x());
    listFrameStyle->move(pos);
    listFrameStyle->show();
}

void MainWindow::setFrameStyle(QListWidgetItem * item) {
    listFrameStyle->close();

    auto gItem = selectedGItem();
    if (gItem == nullptr) return;
    auto field = qgraphicsitem_cast<GraphicsBox *>(gItem);
    if (field == nullptr) return;

    setReportChanged();

    switch (item->data(Qt::UserRole).toInt()) {
    case 1:
        field->setBorder(FrameStyle,Solid);
        break;
    case 2:
        field->setBorder(FrameStyle,Dashed);
        break;
    case 3:
        field->setBorder(FrameStyle,Dotted);
        break;
    case 4:
        field->setBorder(FrameStyle,Dot_dash);
        break;
    case 5:
        field->setBorder(FrameStyle,Dot_dot_dash);
        break;
    case 6:
        field->setBorder(FrameStyle,Double);
        break;
    }
}

void MainWindow::showAbout()
{
    QScopedPointer<AboutDlg> dlg(new AboutDlg(this));
    dlg->exec();
}

void MainWindow::reportPageChanged(int index)
{
    rootItem->takeChildren();

    if (ui->tabWidget->count() == 1)
        ui->actDeleteReportPage->setEnabled(false);
    else
        ui->actDeleteReportPage->setEnabled(true);

    auto repPage = qobject_cast<RepScrollArea *>(ui->tabWidget->widget(index));
	repPage->setScale(cbZoom->currentText());
    auto allReportBand = repPage->getReportBands();

    for (auto &band : allReportBand) {
        rootItem->addChild(band->itemInTree);
        band->itemInTree->setExpanded(true);
        band->setFocus();
    }

    checkAddBandPermission();

    if (sqlDesigner != nullptr)
        sqlDesigner->setCurrentPage(index);
}

void MainWindow::newReportPage()
{
    ui->tabWidget->setUpdatesEnabled(false);
    auto repPage = new RepScrollArea(rootItem, this);

    if (sqlDesigner != nullptr && sender() == ui->actNewReportPage) {
        QDomElement dsElement;
        sqlDesigner->addDiagramDocument(dsElement);
    }

    ui->tabWidget->addTab(repPage,tr("Page %1").arg(ui->tabWidget->count()+1));
    ui->tabWidget->setCurrentIndex(ui->tabWidget->count()-1);
    ui->tabWidget->setUpdatesEnabled(true);

    if (sender() != nullptr)
        ui->actSaveReport->setEnabled(true);
}

void MainWindow::deleteReportPage()
{
    if (sqlDesigner != nullptr)
        sqlDesigner->removeDiagramDocument(ui->tabWidget->currentIndex());

    int index = ui->tabWidget->currentIndex();
    ui->tabWidget->setCurrentIndex(index-1);


    ui->tabWidget->removeTab(index);
    enableAdding();
    ui->actSaveReport->setEnabled(true);
}

void MainWindow::sceneItemAdded(QGraphicsItem *mItem)
{
    generateName(mItem);
    ui->actSelect_tool->setChecked(true);
    ui->actSaveReport->setEnabled(true);
}

void MainWindow::generateName(QGraphicsItem *mItem)
{
    auto cont = gItemToHelper(mItem);

    bool good = false;
    QString contName;
    switch(cont->getFieldType()) {
        case Barcode:
            contName = "barcode%1";
            break;
        case Text:
            contName = "field%1";
            break;
        case TextRich:
            contName = "richText%1";
            break;
        case Image:
            contName = "image%1";
            break;
        case Diagram:
            contName = "diagram%1";
            break;
        case Reactangle:
            contName = "reactangle%1";
            break;
        case Circle:
            contName = "circle%1";
            break;
        case RoundedReactangle:
            contName = "roundedReactangle%1";
            break;
        case Triangle:
            contName = "triangle%1";
            break;
        case Rhombus:
            contName = "rhombus%1";
            break;
        case QtRptName::Line:
            contName = "line%1";
            break;
        case QtRptName::CrossTab:
            contName = "crosstab%1";
            break;
        default:
            contName = "field%1";
    }

    int cf = 1;

    while (!good) {
        bool fnd = false;

        for (int t = 0; t < ui->tabWidget->count(); t++) {
            auto repPage = qobject_cast<RepScrollArea *>(ui->tabWidget->widget( t ));
            for (auto &item : repPage->scene->items()) {
                auto helper = dynamic_cast<GraphicsHelperClass*>(item);
                if (helper != nullptr) {
                    if (helper->objectName() == QString(contName).arg(cf)) {
                        fnd = true;
                        break;
                    }
                }
            }
        }

        if (fnd)
            cf += 1;
        else {
            cont->setObjectName(QString(contName).arg(cf));
            return;
        }
    }
}

bool MainWindow::checkName(const QString &name)
{
    for (int t = 0; t < ui->tabWidget->count(); t++) {
        auto repPage = qobject_cast<RepScrollArea *>(ui->tabWidget->widget( t ));
        for (auto &item : repPage->scene->items()) {
            auto helper = dynamic_cast<GraphicsHelperClass*>(item);
            if (helper != nullptr) {
                if (helper->objectName() == name) {
                    return true;
                }
            }
        }
    }

    return false;
}

void MainWindow::clickOnTBtn()
{
    if (sender()->objectName() == "subBand1")
        ui->actionInsert_band->setChecked(true);
    if (sender()->objectName() == "subBand2")
        ui->actAddDrawing->setChecked(true);
}

void MainWindow::showPageSetting()
{
    QScopedPointer<PageSettingDlg> dlg(new PageSettingDlg(this));
    auto repPage = qobject_cast<RepScrollArea *>(ui->tabWidget->widget( ui->tabWidget->currentIndex() ));

    dlg->showThis(repPage->pageSetting);
    if (dlg->result() == QDialog::Accepted) {
        repPage->pageSetting = dlg->pageSetting;
        repPage->setPaperSize(0);
        ui->actSaveReport->setEnabled(true);
    }
}

void MainWindow::showSetting()
{
    QScopedPointer<SettingDlg> dlg(new SettingDlg(this));
    dlg->showThis();

    auto repPage = qobject_cast<RepScrollArea *>(ui->tabWidget->currentWidget());
    QSettings settings(QCoreApplication::applicationDirPath()+"/setting.ini",QSettings::IniFormat);
    #if QT_VERSION < QT_VERSION_CHECK(6,0,0)
        settings.setIniCodec("UTF-8");
    #endif

    ui->actShowGrid->setChecked(settings.value("ShowGrid",true).toBool());
    repPage->showGrid(settings.value("ShowGrid",true).toBool());

    auto cbFontName = ui->toolBar->findChild<QFontComboBox*>("cbFontName");
    auto cbFontSize = ui->toolBar->findChild<QComboBox*>("cbFontSize");

    cbFontSize->setCurrentIndex(cbFontSize->findText(settings.value("FontSize", 8).toString()));
    cbFontName->setCurrentIndex(cbFontName->findText(settings.value("FontName", "MS Shell Dlg 2").toString()));
}

void MainWindow::showPane()
{
    if (sender() == ui->actDataSource && ui->actDataSource->isChecked()) {
        ui->actScriptEditing->setChecked(false);

        sqlDesigner->showDSData(ui->tabWidget->currentIndex());
    }

    if (sender() == ui->actScriptEditing && ui->actScriptEditing->isChecked()) {
        ui->actDataSource->setChecked(false);

        scriptEditor->showScript();
    }

    if (ui->actDataSource->isChecked() || ui->actScriptEditing->isChecked())
        stackedWidget->setVisible(true);
    else
        stackedWidget->setVisible(false);

    sqlDesigner->setVisible(ui->actDataSource->isChecked());
    scriptEditor->setVisible(ui->actScriptEditing->isChecked());

}

QVector<QDomElement> MainWindow::getDataSourceElements(QDomNode n)
{
    QVector<QDomElement> elementList;
    while(!n.isNull()) {
        QDomElement e = n.toElement();
        if (!e.isNull() && e.tagName() == "DataSource")
            elementList << e;

        n = n.nextSibling();
    }
    return elementList;
}

void MainWindow::closeProgram()
{
    if (ui->actSaveReport->isEnabled()) {
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, tr("Saving"),tr("The report was changed.\nSave the report?"),
                                         QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
        if (reply == QMessageBox::Yes)
            saveReport();
        else if (reply == QMessageBox::Cancel)
            return;
    }
    this->close();
}

void MainWindow::openXML(QSharedPointer<QDomDocument> xmlDoc)
{
    this->xmlDoc = xmlDoc;
    loadReport();
}

// Open file
void MainWindow::openFile()
{
    if (ui->actSaveReport->isEnabled()) {
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, tr("Saving"),tr("The report was changed.\nSave the report?"),
                                         QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
        if (reply == QMessageBox::Yes)
            saveReport();
        else if (reply == QMessageBox::Cancel)
            return;
    }
    ui->actSaveReport->setEnabled(false);

    if (sender() != nullptr && sender() == ui->actionOpenReport) {
        newReport();

        QSettings settings(QCoreApplication::applicationDirPath()+"/setting.ini",QSettings::IniFormat);
        #if QT_VERSION < QT_VERSION_CHECK(6,0,0)
            settings.setIniCodec("UTF-8");
        #endif
        QString folderPath = QApplication::applicationDirPath();
        if (settings.value("recentFileList").toStringList().count() > 0)
            folderPath = QFileInfo(settings.value("recentFileList").toStringList().at(0)).path();
        fileName = QFileDialog::getOpenFileName(this, tr("Select File"), folderPath, "XML (*.xml)");
        if (fileName.isEmpty())
            return;
    }

    QFile file(fileName);
    setCurrentFile(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        return;
    }
    if (!xmlDoc->setContent(&file)) {
        file.close();
        return;
    }
    file.close();
	
	for (auto &plugin : plugins) {
        auto echoInterface = qobject_cast<CustomInterface *>(plugin);
        if (echoInterface) {
            echoInterface->loadData(xmlDoc);
        }
    }

    loadReport();
}

void MainWindow::loadReport()
{
	QApplication::setOverrideCursor(Qt::WaitCursor);

    dontSelect = true;

    QDomElement docElem = xmlDoc->documentElement();  //get root element
    QDomElement repElem;

    while (ui->tabWidget->count() > 1)
        ui->tabWidget->removeTab(ui->tabWidget->count()-1);

    scriptEditor->clear();
    sqlDesigner->clearAll();

    int repNo = 0;
    for (int t = 0; t < docElem.childNodes().count(); t++) {
        if (docElem.tagName() == "Reports" )  //Делаем проверку для совместимости со старыми версиями
            repElem = docElem.childNodes().at(t).toElement();

        if (repElem.tagName() != "Report")
            continue;

        if (repNo != 0) newReportPage();

        auto repPage = qobject_cast<RepScrollArea *>(ui->tabWidget->widget(repNo));
        repPage->clearReport();
        repPage->pageSetting.pageFormat        = repElem.attribute("pageFormat", "A4");
        repPage->pageSetting.marginsLeft       = repElem.attribute("marginsLeft").toInt();
        repPage->pageSetting.marginsRight      = repElem.attribute("marginsRight").toInt();
        repPage->pageSetting.marginsTop        = repElem.attribute("marginsTop").toInt();
        repPage->pageSetting.marginsBottom     = repElem.attribute("marginsBottom").toInt();
        repPage->pageSetting.pageWidth         = repElem.attribute("pageWidth").toInt();
        repPage->pageSetting.pageHeight        = repElem.attribute("pageHeight").toInt();
        repPage->pageSetting.pageOrientation   = repElem.attribute("orientation").toInt();
        repPage->pageSetting.border            = repElem.attribute("border", "0").toInt();
        repPage->pageSetting.borderWidth       = repElem.attribute("borderWidth", "1").toInt();
        repPage->pageSetting.borderColor       = repElem.attribute("borderColor", "rgba(0,0,0,255)");
        repPage->pageSetting.borderStyle       = repElem.attribute("borderStyle", "solid");
        repPage->pageSetting.watermark         = repElem.attribute("watermark", "0").toInt();
        repPage->pageSetting.watermarkOpacity  = repElem.attribute("watermarkOpacity", "0.5").toDouble();

        QByteArray byteArray = QByteArray::fromBase64(repElem.attribute("watermarkPixmap").toLatin1());
        repPage->pageSetting.watermarkPixmap = QPixmap::fromImage(QImage::fromData(byteArray, "PNG"));



        ui->tabWidget->setTabText(ui->tabWidget->currentIndex(), tr("Page %1").arg(ui->tabWidget->currentIndex()+1));
        repPage->setPaperSize(0);
        repPage->setUpdatesEnabled(false);

        //гуляем во всем элементам, родителем которых является корневой
        QDomNode n = repElem.firstChild();
        while(!n.isNull()) {
            QDomElement e = n.toElement(); // try to convert the node to an element.
            if (!e.isNull() && e.tagName() == "ReportBand") {
                BandType type = QtRptName::Undefined;
                if (e.attribute("type") == "ReportTitle")
                    type = ReportTitle;
                if (e.attribute("type") == "PageHeader")
                    type = PageHeader;
                if (e.attribute("type") == "MasterHeader")
                    type = MasterHeader;
                if (e.attribute("type") == "MasterData")
                    type = MasterData;
                if (e.attribute("type") == "MasterFooter")
                    type = MasterFooter;
                if (e.attribute("type") == "PageFooter")
                    type = PageFooter;
                if (e.attribute("type") == "ReportSummary")
                    type = ReportSummary;
                if (e.attribute("type") == "DataGroupHeader")
                    type = DataGroupHeader;
                if (e.attribute("type") == "DataGroupFooter")
                    type = DataGroupFooter;

                if (type == QtRptName::Undefined) continue;

                auto repPage = qobject_cast<RepScrollArea *>(ui->tabWidget->currentWidget());
                auto bandHeight = e.attribute("height").toInt();
                auto bandName = e.attribute("name");
                auto bandNo = e.attribute("bandNo", "1").toInt();
                auto reportBand = repPage->m_addBand(type, bandMenu, bandHeight, bandName, bandNo);
                reportBand->loadParamFromXML(e);


                QDomNode c = n.firstChild();
                while(!c.isNull()) {
                    QDomElement e = c.toElement(); // try to convert the node to an element.
                    if (!e.isNull() && (e.tagName() == "TContainerField" || e.tagName() == "TContainerLine")) {
                        if (e.tagName() == "TContainerField") {
                            auto contField = new GraphicsBox();
                            repPage->scene->addItem(contField);
                            contField->setParentItem(reportBand);
                            contField->setMenu(contMenu);
                            contField->loadParamFromXML(e);
                            repPage->newFieldTreeItem(contField);
                        }
                        if (e.tagName() == "TContainerLine") {
                            auto contLine = new GraphicsLine();
                            repPage->scene->addItem(contLine);
                            contLine->setParentItem(reportBand);
                            contLine->setMenu(contMenu);
                            contLine->loadParamFromXML(e);
                            repPage->newFieldTreeItem(contLine);
                        }

                        //QCoreApplication::processEvents();
                    }
                    c = c.nextSibling();
                }
            }

            n = n.nextSibling();
        }

        QVector<QDomElement> dsElements = getDataSourceElements( repElem.firstChild() );
        sqlDesigner->loadDataSources(repNo, dsElements);

        scriptEditor->showScript();

        repPage->setUpdatesEnabled(true);
        //QCoreApplication::processEvents();

        repNo++;
    }

    ui->treeWidget->clearSelection();
    this->setFocus();
    ui->tabWidget->setCurrentIndex(0);
    reportPageChanged(0);
    this->cbZoom->setCurrentIndex(2);
    ui->actSaveReport->setEnabled(false);
    this->setWindowTitle("QtRPT Designer "+fileName);
    enableAdding();

    auto repPage = qobject_cast<RepScrollArea *>(ui->tabWidget->widget(0));
    QCoreApplication::processEvents();
    repPage->scene->update();
    repPage->scene->m_undoStack->clear();

    dontSelect = false;

    QApplication::restoreOverrideCursor();
}

// Select color from dialog and set param
void MainWindow::chooseColor()
{
    auto item = selectedGItem();
    if (item == nullptr) return;

    auto ed = qobject_cast<EditorDelegate*>(sender());
    Command command;
    if (ed != nullptr)
        command = (Command)ui->treeParams->currentItem()->data(1,Qt::UserRole).toInt();
    else
        command = getCommand(sender());

    auto helper = gItemToHelper(item);
    QColor color = helper->getColorValue(command);
    QScopedPointer<QColorDialog> dlg(new QColorDialog(color, this));
    if (dlg->exec() == QDialog::Accepted) {
        color = dlg->selectedColor();
        ui->actSaveReport->setEnabled(true);
    } else return;

    execButtonCommand(command, color);
}

void MainWindow::changeTextFont()
{
    if (selectedGItem() == nullptr)
        return;
    auto gItem = qgraphicsitem_cast<GraphicsBox *>(selectedGItem());
    if (gItem == nullptr)
        return;
    if (gItem->type() != GBox)
        return;

    auto action = qobject_cast<QAction *>(sender());
    auto cmb = qobject_cast<QComboBox *>(sender());
    Command command = getCommand(sender());
    QVariant v;
    if (action != nullptr) {
        v = action->isChecked();
        if (action == ui->actAlignLeft) v=0;
        if (action == ui->actAlignRight) v=2;
        if (action == ui->actAlignCenter) v=1;
        if (action == ui->actAlignJustify) v=3;
        if (action == ui->actAlignTop) v=0;
        if (action == ui->actAlignVCenter) v=1;
        if (action == ui->actAlignBottom) v=2;
    }
    if (cmb != nullptr)
        v = cmb->itemText(cmb->currentIndex());

    execButtonCommand(command,v);
    ui->actSaveReport->setEnabled(true);
}

// Определяем, тип команды в зависимости от нажатой кнопки
Command MainWindow::getCommand(QObject *widget)
{
    if (widget == nullptr)
        return None;
    auto action = qobject_cast<QAction*>(widget);
    auto cmb = qobject_cast<QComboBox*>(widget);
    if (action != nullptr) {
        if (action == ui->actionBold) return Bold;
        else if (action == ui->actionItalic) return Italic;
        else if (action == ui->actionUnderline) return Underline;
        else if (action == ui->actionStrikeout) return Strikeout;
        else if (action == ui->actLineLeft) return FrameLeft;
        else if (action == ui->actLineRight) return FrameRight;
        else if (action == ui->actLineBottom) return FrameBottom;
        else if (action == ui->actLineTop) return FrameTop;
        else if (action == ui->actLineNo) return FrameNo;
        else if (action == ui->actLineAll) return FrameAll;
        else if (action == ui->actAlignLeft) return AligmentH;
        else if (action == ui->actAlignRight) return AligmentH;
        else if (action == ui->actAlignCenter) return AligmentH;
        else if (action == ui->actAlignJustify) return AligmentH;
        else if (action == ui->actAlignTop) return AligmentV;
        else if (action == ui->actAlignVCenter) return AligmentV;
        else if (action == ui->actAlignBottom) return AligmentV;
        else if (action == ui->actFrameStyle) return FrameStyle;
        else if (action == ui->actBorderColor) return BorderColor;
        else if (action == ui->actBackgroundColor) return BackgroundColor;
        else if (action == ui->actFontColor) return FontColor;
        else return None;
    }
    if (cmb != nullptr) {
        if (cmb == cbFontSize)
            return FontSize;
        if (cmb == cbFontName)
            return FontName;
        if (cmb == cbFrameWidth)
            return FrameWidth;
        return None;
    }
    return None;
}

void MainWindow::undo()
{
    auto repPage = qobject_cast<RepScrollArea *>(ui->tabWidget->currentWidget());
    qDebug() << tr("Going to make undo: ")<<repPage->scene->m_undoStack->undoText();
    repPage->scene->m_undoStack->undo();
    showParamState();
}

void MainWindow::redo()
{
    auto repPage = qobject_cast<RepScrollArea *>(ui->tabWidget->currentWidget());
    qDebug() << tr("Going to make redo: ")<<repPage->scene->m_undoStack->undoText();
    repPage->scene->m_undoStack->redo();
    showParamState();
}

void MainWindow::setGroupingField()
{
    auto repPage = qobject_cast<RepScrollArea *>(ui->tabWidget->currentWidget());
    QString groupName = "";
    if (sender() == ui->actGroup) {
        groupName = "group%1";
        bool good = false;
        int cf = 1;
        while (!good) {
            bool fnd = false;
            for (auto &item : repPage->scene->items()) {
                if (item->type() == ItemType::GLine || item->type() == ItemType::GBox) {
                    auto helper = gItemToHelper(item);
                    if (helper->getGroupName() == QString(groupName).arg(cf)) {
                        fnd = true;
                        break;
                    }
                }
            }

            if (fnd)
                cf += 1;
            else {
                groupName = QString(groupName).arg(cf);
                break;
            }
        }
    }

    for (auto &item : repPage->scene->items()) {
        if (item->type() == ItemType::GLine || item->type() == ItemType::GBox) {
            auto helper = gItemToHelper(item);
            auto helperSelected = gItemToHelper(selectedGItem());

            if (helper == nullptr || helperSelected == nullptr)
                continue;

            if (sender() == ui->actUngroup && helper->getGroupName() == helperSelected->getGroupName()) {
                helper->setGroupName("");
                helper->helperSelect(false);
            }
            if (sender() == ui->actGroup && helper->helperIsSelected())
                helper->setGroupName(groupName);
        }
    }
}

QGraphicsItem *MainWindow::selectedGItem()
{
    auto repPage = qobject_cast<RepScrollArea *>(ui->tabWidget->currentWidget());
    if (repPage->scene->itemsSelected().isEmpty())
        return nullptr;

    return repPage->scene->itemsSelected().at(0);
}

GraphicsHelperClass *MainWindow::gItemToHelper(QGraphicsItem *item)
{
    auto helper = dynamic_cast<GraphicsHelperClass*>(item);
    return helper;
}

// Container's selection
void MainWindow::sceneItemSelectionChanged(QGraphicsItem *item)
{
    if (dontSelect)
        return;

    auto repPage = qobject_cast<RepScrollArea *>(ui->tabWidget->currentWidget());
    auto scene = repPage->scene;

    if (!item) {
        // unselect all
        scene->blockSignals(true);
        for (auto &m_item : scene->items()) {
            bool isLine = m_item->type() == ItemType::GLine;
            bool isBox = m_item->type() == ItemType::GBox;
            bool isBand = m_item->type() == ItemType::GBand;

            if (isLine || isBox || isBand) {
                 auto helper = dynamic_cast<GraphicsHelperClass*>(m_item);
                 helper->helperSelect(false);
            }
        }

        ui->treeParams->clear();
        showParamState();
        scene->blockSignals(false);
        return;
    }

    bool isLine = item->type() == ItemType::GLine;
    bool isBox = item->type() == ItemType::GBox;
    bool isBand = item->type() == ItemType::GBand;
    if (!isLine && !isBox && !isBand)
        return;

    scene->blockSignals(true);

    auto calling_helper = dynamic_cast<GraphicsHelperClass*>(item);

    if (QApplication::keyboardModifiers() != Qt::ControlModifier) {
        for (auto &m_item : scene->items()) {
            if (item != m_item) {
                bool isLine = m_item->type() == ItemType::GLine;
                bool isBox = m_item->type() == ItemType::GBox;
                bool isBand = m_item->type() == ItemType::GBand;

                if (isLine || isBox || isBand) {
                     auto helper = dynamic_cast<GraphicsHelperClass*>(m_item);

                     if (!helper->getGroupName().isEmpty() && helper->getGroupName() == calling_helper->getGroupName())
                        helper->helperSelect(true);

                     //Un-select containters
                     if (!calling_helper->getGroupName().isEmpty() && helper->getGroupName() != calling_helper->getGroupName())
                         helper->helperSelect(false);
                     if (calling_helper->getGroupName().isEmpty())
                         helper->helperSelect(false);
                }
            }
        }

        ui->treeParams->clear();
        showParamState();
    }
    scene->blockSignals(false);
}

void MainWindow::alignFields()
{
    auto repPage = qobject_cast<RepScrollArea *>(ui->tabWidget->currentWidget());
    auto etalon = qgraphicsitem_cast<GraphicsBox*>(selectedGItem());

    for (auto &item : repPage->scene->items()) {
        if (item->type() == ItemType::GBox) {
            auto box = qgraphicsitem_cast<GraphicsBox*>(item);
            if (box->isSelected() && box != etalon) {
                if (sender() == ui->actFieldLeft)
                    box->setPos(etalon->pos().x(), item->pos().y());
                if (sender() == ui->actFieldRight)
                    box->setPos(etalon->pos().x()+etalon->getWidth()-box->getWidth(), item->pos().y());
                if (sender() == ui->actFieldMiddle)
                    box->setPos(etalon->pos().x()+etalon->getWidth()/2-box->getWidth()/2, item->pos().y());
                if (sender() == ui->actFieldTop)
                    box->setPos(item->pos().x(), etalon->pos().y());
                if (sender() == ui->actFieldBottom)
                    box->setPos(item->pos().x(), etalon->pos().y()+etalon->getHeight()-box->getHeight());
                if (sender() == ui->actFieldCenter)
                    box->setPos(item->pos().x(), etalon->pos().y()+etalon->getHeight()/2-box->getHeight()/2);
                if (sender() == ui->actFieldSameHeight)
                    box->setHeight(etalon->getHeight());
                if (sender() == ui->actFieldSameWidth)
                    box->setWidth(etalon->getWidth());
            }
        }
    }

    ui->actSaveReport->setEnabled(true);
}

static void xmlWrite(QXmlStreamWriter *w, QDomElement e)
{
    // note that this only writes elements, text and attributes (comments and CDATA ignored)

    w->writeStartElement(e.nodeName());

    // sort attributes by name

    auto map = e.attributes();
    QList<QString> attrNames;
    for (int i = 0; i < map.count(); i++) {
        QDomAttr a = map.item(i).toAttr();
        attrNames.append(a.name());
    }

    std::sort(attrNames.begin(), attrNames.end());

    // write sorted attributes

    for (int i = 0; i < attrNames.count(); i++) {
        QString name = attrNames.at(i);
        QString value = e.attribute(name);
        w->writeAttribute(name, value);
    }

    // recursively write children (text and elements)

    QDomNode child = e.firstChild();
    while (!child.isNull()) {
        if (child.isText())
            w->writeCharacters(child.toText().data());
        else if (child.isElement())
            xmlWrite(w, child.toElement());

        child = child.nextSibling();
    }

    w->writeEndElement();
}

QSharedPointer<QDomDocument> MainWindow::saveXML()
{
    xmlDoc->clear();
    QDomElement docElem = xmlDoc->createElement("Reports");
    docElem.setAttribute("lib","QtRPT");
    docElem.setAttribute("programmer","Aleksey Osipov");
    docElem.setAttribute("email","aliks-os@ukr.net");
    xmlDoc->appendChild(docElem);

    //before saving, we are deleting all nodes
    QDomNode n = docElem.firstChild();
    while(!n.isNull()) {
        n.parentNode().removeChild(n);
        n = docElem.firstChild();
    }

    scriptEditor->saveParamToXML(xmlDoc, docElem);

    for (quint16 rp = 0; rp < ui->tabWidget->count(); rp++) {
        auto repPage = qobject_cast<RepScrollArea *>(ui->tabWidget->widget(rp));
        QDomElement repElem = xmlDoc->createElement("Report");
        repElem.setAttribute("pageNo",QString::number(rp+1));
        repElem.setAttribute("pageFormat",repPage->pageSetting.pageFormat);
        repElem.setAttribute("pageHeight",repPage->pageSetting.pageHeight);
        repElem.setAttribute("pageWidth",repPage->pageSetting.pageWidth);
        repElem.setAttribute("marginsLeft",repPage->pageSetting.marginsLeft);
        repElem.setAttribute("marginsRight",repPage->pageSetting.marginsRight);
        repElem.setAttribute("marginsTop",repPage->pageSetting.marginsTop);
        repElem.setAttribute("marginsBottom",repPage->pageSetting.marginsBottom);
        repElem.setAttribute("orientation",repPage->pageSetting.pageOrientation);
        repElem.setAttribute("border",repPage->pageSetting.border);
        repElem.setAttribute("borderWidth",repPage->pageSetting.borderWidth);
        repElem.setAttribute("borderColor",repPage->pageSetting.borderColor);
        repElem.setAttribute("borderStyle",repPage->pageSetting.borderStyle);
        repElem.setAttribute("watermark",repPage->pageSetting.watermark);
        repElem.setAttribute("watermarkOpacity",repPage->pageSetting.watermarkOpacity);

        QByteArray byteArray;
        QBuffer buffer(&byteArray);
        buffer.open(QIODevice::WriteOnly);

        repPage->pageSetting.watermarkPixmap.save(&buffer, "PNG");

        QString s = byteArray.toBase64();

        repElem.setAttribute("watermarkPixmap",s);
        docElem.appendChild(repElem);

        for (auto &gItem : repPage->scene->items(Qt::AscendingOrder))
            if (gItem->type() == ItemType::GBand)
                setXMLProperty(&repElem, gItem, 1);

        for (auto &gItem : repPage->scene->items(Qt::AscendingOrder))
            if (gItem->type() == ItemType::GBox || gItem->type() == ItemType::GLine)
                setXMLProperty(&repElem, gItem, 1);

        setXMLProperty(&repElem, sqlDesigner, 0); //Set XML for DataSource
    }

    for (auto &plugin : plugins) {
        auto echoInterface = qobject_cast<CustomInterface *>(plugin);
        if (echoInterface) {
            echoInterface->saveData(xmlDoc);
        }
    }

    return xmlDoc;
}

void MainWindow::saveReport()
{
    saveXML();

    if (fileName.isEmpty() || fileName.isNull() || sender() == ui->actSaveAs) {
        QString tmpfileName = QFileDialog::getSaveFileName(this, tr("Save File"), "", tr("XML Files (*.xml)"));
        if (tmpfileName.isEmpty() || tmpfileName.isNull() ) return;
        fileName = tmpfileName;
    }

    QFile file(fileName);
    if (file.open(QIODevice::WriteOnly)) {
        setCurrentFile(fileName);

//        QXmlStreamWriter w(&file);
//        w.setAutoFormatting(true);
//        w.setAutoFormattingIndent(2);
//        w.writeStartDocument();
//        xmlWrite(&w, xmlDoc->documentElement());
//        w.writeEndDocument();

        QTextStream stream(&file);
        xmlDoc->save(stream, 2, QDomNode::EncodingFromTextStream);

        file.close();
        ui->actSaveReport->setEnabled(false);
        this->setWindowTitle("QtRPT Designer "+fileName);
    } else {
        QMessageBox::warning(this, tr("Error"), file.errorString());
    }
}

bool MainWindow::setXMLProperty(QDomElement *repElem, void *ptr, int type)
{
    QGraphicsItem *gItem = nullptr;
    QDomElement elem;

    if (type == 0) {
        auto sqlDesigner = static_cast<SqlDesigner *>(ptr);
        auto elemList = sqlDesigner->saveParamToXML(xmlDoc);
        for (const auto &elem : elemList) {
            if (!elem.isNull())
                repElem->appendChild(elem);
        }
    } else {
        gItem = static_cast<QGraphicsItem *>(ptr);
    }

    if (gItem != nullptr && gItem->type() == ItemType::GBand) {
        auto band = static_cast<ReportBand *>(ptr);
        elem = band->saveParamToXML(xmlDoc);
        repElem->appendChild(elem);
    }
    if (gItem != nullptr && (gItem->type() == ItemType::GBox || gItem->type() == ItemType::GLine)) {
        auto item = static_cast<GraphicsBox *>(gItem);
        auto line = static_cast<GraphicsLine *>(gItem);
        auto band = static_cast<GraphicsBox *>(gItem->parentItem());

        QString parent = band->objectName();
        QDomNodeList nodelist = repElem->elementsByTagName("ReportBand");

        for (quint16 i = 0; i != nodelist.count(); i++) {
            QDomNode prn = nodelist.item(i).toElement();
            if (prn.toElement().attribute("name") == parent) {
                if (gItem->type() == ItemType::GBox)
                    elem = item->saveParamToXML(xmlDoc);
                if (gItem->type() == ItemType::GLine)
                    elem = line->saveParamToXML(xmlDoc);
                prn.appendChild(elem);
            }
        }
    }
    return false;
}

// Show param of the container
void MainWindow::showParamState()
{
    auto repPage = qobject_cast<RepScrollArea *>(ui->tabWidget->currentWidget());
    if (repPage->scene->selectedItems().isEmpty())
        return;

    ui->treeParams->clear();

    bool enbl1 = false;
    bool enbl2 = false;

    ui->actBackgroundColor->setEnabled(false);
    this->cbFontName->setEnabled(false);
    this->cbFontSize->setEnabled(false);
    this->cbFrameWidth->setEnabled(false);
    ui->actionBold->setEnabled(false);
    ui->actionItalic->setEnabled(false);
    ui->actionUnderline->setEnabled(false);
    ui->actionStrikeout->setEnabled(false);
    ui->actFontColor->setEnabled(false);

    if (repPage->scene->selectedItems().at(0)->type() == ItemType::GBand) {
        auto band = static_cast<ReportBand *>(repPage->scene->selectedItems().at(0));
        ui->actBackgroundColor->setEnabled(false);
        ui->actAlignBottom->setChecked(false);
        ui->actAlignCenter->setChecked(false);
        ui->actAlignJustify->setChecked(false);
        ui->actAlignLeft->setChecked(false);
        ui->actAlignRight->setChecked(false);
        ui->actAlignTop->setChecked(false);
        ui->actAlignVCenter->setChecked(false);
        ui->actionBold->setChecked(false);
        ui->actionItalic->setChecked(false);
        ui->actionUnderline->setChecked(false);
        ui->actionStrikeout->setChecked(false);
        ui->actLineAll->setChecked(false);
        ui->actLineNo->setChecked(false);
        ui->actLineBottom->setChecked(false);
        ui->actLineLeft->setChecked(false);
        ui->actLineRight->setChecked(false);
        ui->actLineTop->setChecked(false);

        setParamTree(Name, band->objectName());
        setParamTree(Height, band->getHeight() - band->titleHeight);
        switch(band->bandType) {
            case DataGroupHeader: {
                break;
            }
            case DataGroupFooter: {
                setParamTree(GroupLevel, band->getGroupLevel());
                break;
            }
            case MasterData: {
                setParamTree(DSName, band->getDSName());
                setParamTree(GroupParam, tr("Group param"));
                setParamTree(StartNewNumeration, band->getStartNewNumertaion(), true);
                setParamTree(StartNewPage, band->getStartNewPage(), true);
                setParamTree(GroupFields, band->getGroupingField(), true);
                break;
            }
            case MasterHeader:
            case MasterFooter: {
                setParamTree(ShowInGroup, band->getShowInGroup());
                break;
            }
            default:
                break;
        }
    }
    if (repPage->scene->selectedItems().at(0)->type() == ItemType::GBox) {
        auto field = static_cast<GraphicsBox *>(repPage->scene->selectedItems().at(0));
        auto band = static_cast<ReportBand *>(field->parentItem());
        enbl1 = true;
        enbl2 = true;

        int al = 0;
        int alV = 0;
        if (field->getAlignment() & Qt::AlignLeft) al = 0;
        else if (field->getAlignment() & Qt::AlignHCenter) al = 1;
        else if (field->getAlignment() & Qt::AlignRight) al = 2;
        else if (field->getAlignment() & Qt::AlignJustify) al = 3;

        if (field->getAlignment() & Qt::AlignTop) alV = 0;
        else if (field->getAlignment() & Qt::AlignVCenter) alV = 1;
        else if (field->getAlignment() & Qt::AlignBottom) alV = 2;

        bool top = field->borderIsCheck(FrameTop);
        bool bottom = field->borderIsCheck(FrameBottom);
        bool left = field->borderIsCheck(FrameLeft);
        bool right = field->borderIsCheck(FrameRight);

        //Sets params in the tree
        setParamTree(Name, field->objectName());
        setParamTree(Height, field->getHeight());
        setParamTree(Width, field->getWidth());
        setParamTree(Left, field->getPos().x());
        setParamTree(Top, field->getPos().y() - band->titleHeight);
        setParamTree(Printing, field->getPrinting());

        switch (field->getFieldType()) {
            case Circle:
            case Triangle:
            case Rhombus:
            case RoundedReactangle:
            case Reactangle: {
                ui->actBackgroundColor->setEnabled(true);
                ui->actBorderColor->setEnabled(true);
                this->cbFrameWidth->setEnabled(true);

                setParamTree(BackgroundColor, field->getColorValue(BackgroundColor));
                setParamTree(BorderColor, field->getColorValue(BorderColor));
                setParamTree(FrameWidth, field->getBorderWidth());
                break;
            }
            case Diagram: {
                break;
            }
            case Barcode: {
                setParamTree(BarcodeType, field->getBarcodeType());
                setParamTree(BarcodeFrameType, field->getBarcodeFrameType());
                break;
            }
            case Image: {
                ui->actBackgroundColor->setEnabled(false);
                setParamTree(AligmentH, al);
                setParamTree(AligmentV, alV);
                setParamTree(IgnoreRatioAspect, field->getIgnoreAspectRatio());
                break;
            }
            case TextImage: {
                ui->actBackgroundColor->setEnabled(false);
                setParamTree(AligmentH, al);
                setParamTree(AligmentV, alV);

                setParamTree(Frame, tr("Frame"));
                setParamTree(FrameLeft, left, true);
                setParamTree(FrameRight, right, true);
                setParamTree(FrameTop, top, true);
                setParamTree(FrameBottom, bottom, true);
                break;
            }
            case DatabaseImage: {
                ui->actBackgroundColor->setEnabled(false);
                setParamTree(AligmentH, al);
                setParamTree(AligmentV, alV);
                setParamTree(AutoHeight, field->getAutoHeight());
                break;
            }
            case Text: {
                ui->actBackgroundColor->setEnabled(true);
                ui->actBorderColor->setEnabled(true);
                this->cbFontName->setEnabled(true);
                this->cbFontSize->setEnabled(true);
                this->cbFrameWidth->setEnabled(true);
                ui->actionBold->setEnabled(true);
                ui->actionItalic->setEnabled(true);
                ui->actionUnderline->setEnabled(true);
                ui->actionStrikeout->setEnabled(true);
                ui->actFontColor->setEnabled(true);

                setParamTree(AligmentH, al);
                setParamTree(AligmentV, alV);
                setParamTree(AutoHeight, field->getAutoHeight());
                setParamTree(RenderingMode, field->getRenderingMode());
                setParamTree(BackgroundColor, field->getColorValue(BackgroundColor));
                setParamTree(BorderColor, field->getColorValue(BorderColor));
                setParamTree(Font, tr("Font"));
                setParamTree(FontName, field->getFont().family(), true);
                setParamTree(FontSize, field->getFont().pointSize(), true);
                setParamTree(Bold, field->getFont().bold(), true);
                setParamTree(Italic, field->getFont().italic(), true);
                setParamTree(Underline, field->getFont().underline(), true);
                setParamTree(Strikeout, field->getFont().strikeOut(), true);
                setParamTree(FontColor, field->getColorValue(FontColor), true);
                setParamTree(Frame, tr("Frame"));
                setParamTree(FrameLeft, left, true);
                setParamTree(FrameRight, right, true);
                setParamTree(FrameTop, top, true);
                setParamTree(FrameBottom, bottom, true);
                setParamTree(FrameWidth, field->getBorderWidth());
                setParamTree(TextWrap, field->textWrap());
                setParamTree(TextRotate, field->textRotate());
                break;
            }
            case TextRich: {
                enbl2 = false;
                ui->actBackgroundColor->setEnabled(false);
                this->cbFontName->setEnabled(false);
                this->cbFontSize->setEnabled(false);
                ui->actionBold->setEnabled(false);
                ui->actionItalic->setEnabled(false);
                ui->actionUnderline->setEnabled(false);
                ui->actionStrikeout->setEnabled(false);
                ui->actFontColor->setEnabled(false);
                break;
            }
            default: {
            }
        }
    }
    if (repPage->scene->selectedItems().at(0)->type() == ItemType::GLine) {
        this->cbFrameWidth->setEnabled(true);

        auto line = static_cast<GraphicsLine *>(repPage->scene->selectedItems().at(0));
        setParamTree(BorderColor, line->getColorValue(BorderColor));
        setParamTree(FrameWidth, line->getBorderWidth());
        setParamTree(Name, line->objectName());
        setParamTree(Length, (int)line->getLength());
        setParamTree(Printing, line->getPrinting());
        setParamTree(ArrowStart, line->getArrow(ArrowStart));
        setParamTree(ArrowEnd, line->getArrow(ArrowEnd));
    }

    ui->actAlignBottom->setEnabled(enbl2);
    ui->actAlignCenter->setEnabled(enbl2);
    ui->actAlignJustify->setEnabled(enbl2);
    ui->actAlignLeft->setEnabled(enbl2);
    ui->actAlignRight->setEnabled(enbl2);
    ui->actAlignTop->setEnabled(enbl2);
    ui->actAlignVCenter->setEnabled(enbl2);

    ui->actLineAll->setEnabled(enbl1);
    ui->actLineNo->setEnabled(enbl1);
    ui->actLineBottom->setEnabled(enbl1);
    ui->actLineLeft->setEnabled(enbl1);
    ui->actLineRight->setEnabled(enbl1);
    ui->actLineTop->setEnabled(enbl1);
}

// Поиск ветки в дереве параметра
QTreeWidgetItem *MainWindow::findItemInTree(Command command)
{
    QTreeWidgetItemIterator it(ui->treeParams);
    while (*it) {
        QTreeWidgetItem *item = (*it);
        if (item->data(1,Qt::UserRole) == command)
            return item;
        ++it;
    }
    return nullptr;
}

void MainWindow::newReport()
{
    if (ui->actSaveReport->isEnabled()) {
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, tr("Saving"),tr("The report was changed.\nSave the report?"),
                                         QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
        if (reply == QMessageBox::Yes)
            saveReport();
        else if (reply == QMessageBox::Cancel)
            return;
    }
    while (ui->tabWidget->count() > 1)
        ui->tabWidget->removeTab(ui->tabWidget->count()-1);

    auto repPage = qobject_cast<RepScrollArea *>(ui->tabWidget->widget(0));
    repPage->clearReport();
    QTimer::singleShot(10, repPage->scene, SLOT(update()));

    cbZoom->setEditText("100%");
    qDeleteAll(rootItem->takeChildren());

    ui->actSaveReport->setEnabled(false);
    fileName = "";
    this->setWindowTitle("QtRPT Designer "+fileName);

    for (auto &action : ui->actionInsert_band->menu()->actions())
        action->setEnabled(true);

    for (auto &plugin : plugins) {
        auto echoInterface = qobject_cast<CustomInterface *>(plugin);
        if (echoInterface) {
            echoInterface->clear(xmlDoc);
        }
    }

    scriptEditor->clear();
    sqlDesigner->clearAll();
    enableAdding();
    repPage->scene->m_undoStack->clear();
}

QGraphicsItemList MainWindow::getSelectedItems()
{
    QGraphicsItemList list;
    auto repPage = qobject_cast<RepScrollArea *>(ui->tabWidget->currentWidget());
    for (auto &item : repPage->scene->items()) {
        bool isLine = item->type() == ItemType::GLine;
        bool isBox = item->type() == ItemType::GBox;
        bool isBand = item->type() == ItemType::GBand;

        if (isBox || isBand || isLine)
            if (gItemToHelper(item)->helperIsSelected())
                list.append(item);
    }
    return list;
}

GraphicsHelperList MainWindow::getSelectedHelperItems()
{
    GraphicsHelperList list;
    auto repPage = qobject_cast<RepScrollArea *>(ui->tabWidget->currentWidget());
    for (auto &item : repPage->scene->items())
        if (item->type() == ItemType::GBox || item->type() == ItemType::GBand || item->type() == ItemType::GLine)
            if (gItemToHelper(item)->helperIsSelected())
                list.append(gItemToHelper(item));
    return list;
}

void MainWindow::execButtonCommand(Command command, QVariant value)
{
    if (command == None) return;
    if (selectedGItem() == nullptr) return;

    auto repPage = qobject_cast<RepScrollArea *>(ui->tabWidget->currentWidget());

    //before changing params gets params
    BArrayList oldList = ParamCommand::getBArrayFromContList(getSelectedHelperItems());

    for (auto &item : getSelectedItems())
        processCommand(command, value, item);

    setParamTree(command, value);
    repPage->scene->update();
    ui->actSaveReport->setEnabled(true);

    //gets new params
    BArrayList newList = ParamCommand::getBArrayFromContList(getSelectedHelperItems());
    QList<PairCont> lst = ParamCommand::compoundArrays(oldList,newList);

    auto scene = qobject_cast<GraphicsScene*>(repPage->scene);
    scene->m_undoStack->push(new ParamCommand( lst, repPage->scene ));
}

void MainWindow::processCommand(Command command, QVariant value, QGraphicsItem *item) {
    ReportBand *band = nullptr;
    GraphicsLine *line = nullptr;
    GraphicsBox *box = nullptr;

    auto helper = gItemToHelper(item);
    if (helper == nullptr) return;
    if (item->type() == ItemType::GBand)
        band = static_cast<ReportBand *>(item);
    if (item->type() == ItemType::GBox)
        box = static_cast<GraphicsBox*>(item);
    if (item->type() == ItemType::GLine)
        line = static_cast<GraphicsLine*>(item);

    QFont fnt;
    if (box != nullptr)
        fnt = box->getFont();

    switch(command) {
        case None: {
            return;
            break;
        }
        case Name: {
            helper->setObjectName(value.toString());
            helper->itemInTree->setText(0,value.toString());
            break;
        }
        case Bold: {
            fnt.setBold(value.toBool());
            break;
        }
        case Italic: {
            fnt.setItalic(value.toBool());
            break;
        }
        case Underline: {
            fnt.setUnderline(value.toBool());
            break;
        }
        case Strikeout: {
            fnt.setStrikeOut(value.toBool());
            break;
        }
        case FontSize: {
            fnt.setPointSizeF(value.toInt());
            break;
        }
        case FontName: {
            fnt.setFamily(value.toString());
            break;
        }
        case FrameLeft:
        case FrameRight:
        case FrameTop:
        case FrameBottom: {
            box->setBorder(command,QColor(),value.toBool());
            break;
        }
        case FrameNo: {
            box->setBorder(command,0);
            break;
        }
        case FrameAll: {
            box->setBorder(command,0);
            break;
        }
        case FrameWidth: {
            if (box != nullptr)
                box->setBorder(command,value);
            if (line != nullptr)
                line->setBorder(command,value);
            break;
        }
        case AligmentH: {
            Qt::Alignment vAl;
            if (box->getAlignment() & Qt::AlignTop)
                vAl = Qt::AlignTop;
            else if (box->getAlignment() & Qt::AlignVCenter)
                vAl = Qt::AlignVCenter;
            else if (box->getAlignment() & Qt::AlignBottom)
                vAl = Qt::AlignBottom;
            else
                vAl = Qt::AlignVCenter;

            if (value.toInt() == 0)
                box->setAlignment(Qt::AlignLeft | vAl);
            if (value.toInt() == 1)
                box->setAlignment(Qt::AlignHCenter | vAl);
            if (value.toInt() == 2)
                box->setAlignment(Qt::AlignRight | vAl);
            if (value.toInt() == 3)
                box->setAlignment(Qt::AlignJustify | vAl);
            break;
        }
        case BarcodeType: {
            box->setBarcodeType((BarCode::BarcodeTypes)value.toInt());
            break;
        }
        case BarcodeFrameType: {
            box->setBarcodeFrameType((BarCode::FrameTypes)value.toInt());
            break;
        }
        case AligmentV: {
            Qt::Alignment hAl;
            if (box->getAlignment() & Qt::AlignLeft)
                hAl = Qt::AlignLeft;
            else if (box->getAlignment() & Qt::AlignRight)
                hAl = Qt::AlignRight;
            else if (box->getAlignment() & Qt::AlignHCenter)
                hAl = Qt::AlignHCenter;
            else if (box->getAlignment() & Qt::AlignJustify)
                hAl = Qt::AlignJustify;
            else
                hAl = Qt::AlignLeft;

            if (value.toInt() == 0)
                box->setAlignment(hAl | Qt::AlignTop);
            if (value.toInt() == 1)
                box->setAlignment(hAl | Qt::AlignVCenter);
            if (value.toInt() == 2)
                box->setAlignment(hAl | Qt::AlignBottom);
            break;
        }
        case Left: {
            QPointF r = box->getPos();
            r.setX(value.toInt());
            box->setPos(r);
            break;
        }
        case Top: {
            auto band = static_cast<ReportBand *>(box->parentItem());
            QPointF r = box->pos();
            r.setY(value.toInt() + band->titleHeight);
            box->setPos(r);
            break;
        }
        case Width: {
            box->setWidth(value.toInt());
            break;
        }
        case Height: {
            if (band != nullptr) {
                band->setHeight(value.toInt());
                auto repPage = qobject_cast<RepScrollArea *>(ui->tabWidget->widget(ui->tabWidget->currentIndex()));
                repPage->correctBandGeom(nullptr);
            }
            if (box != nullptr)
                box->setHeight(value.toInt());
            break;
        }
        case Length: {
            line->setLength(value.toInt());
            break;
        }
        case Printing: {
            helper->setPrinting(value.toBool() == true ? "1" : "0");
            break;
        }
        case IgnoreRatioAspect: {
            box->setIgnoreAspectRatio(value.toBool());
            break;
        }
        case ArrowStart: {
            line->setArrow(ArrowStart,value.toBool());
            break;
        }
        case ArrowEnd: {
            line->setArrow(ArrowEnd,value.toBool());
            break;
        }
        case DSName: {
            band->setDSName(value.toString());
            break;
        }
        case StartNewNumeration: {
            band->setStartNewNumeration(value.toBool());
            break;
        }
        case ShowInGroup: {
            band->setShowInGroup(value.toBool());
            break;
        }
        case StartNewPage: {
            band->setStartNewPage(value.toBool());
            break;
        }
        case GroupLevel: {
            band->setGroupLevel(value.toInt());
            auto repPage = qobject_cast<RepScrollArea *>(ui->tabWidget->currentWidget());
            repPage->correctBandGeom();
            break;
        }
        case GroupFields: {
            band->setGroupingField(value.toString());
            break;
        }
        case AutoHeight: {
            box->setAutoHeight(value.toBool());
            break;
        }
        case RenderingMode: {
            box->setRenderingMode(value.toBool());
            break;
        }
        case TextRotate: {
            box->setTextRotate(value.toInt());
            break;
        }
        case TextWrap: {
            box->setTextWrap(value.toBool());
            break;
        }
        case FontColor: {
            helper->setColorValue(FontColor, value.value<QColor>() );
            break;
        }
        case BackgroundColor: {
            helper->setColorValue(BackgroundColor, value.value<QColor>() );
            break;
        }
        case BorderColor: {
            helper->setBorder(BorderColor, value.toString());
            break;
        }
        default: break;
    }

    if (box != nullptr)
        box->setFont(fnt);
}

//Sets params in the tree
void MainWindow::setParamTree(Command command, QVariant value, bool child)
{
    if (command == None) return;

    QTreeWidgetItem *item = findItemInTree(command);
    QTreeWidgetItem *parentNode = nullptr;
    if (item == nullptr && !child)
        item = new QTreeWidgetItem(ui->treeParams);

    switch (command) {
        case Name: {
            item->setText(0,tr("Name"));
            item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable);
            item->setText(1,value.toString());
            break;
        }
        case AligmentH: {
            item->setText(0,tr("Aligment hor"));
            item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable);
            switch(value.toInt()) {
                case 0: {
                    item->setText(1,tr("Left"));
                    ui->actAlignLeft->setChecked(true);
                    break;
                }
                case 1: {
                    item->setText(1,tr("Center"));
                    ui->actAlignCenter->setChecked(true);
                    break;
                }
                case 2: {
                    item->setText(1,tr("Right"));
                    ui->actAlignRight->setChecked(true);
                    break;
                }
                case 3: {
                    item->setText(1,tr("Justify"));
                    ui->actAlignJustify->setChecked(true);
                    break;
                }
                default: item->setText(1,"");
            }
            break;
        }
        case AligmentV: {
            item->setText(0,tr("Aligment ver"));
            item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable);
            switch(value.toInt()) {
                case 0: {
                    item->setText(1,tr("Top"));
                    ui->actAlignTop->setChecked(true);
                    break;
                }
                case 1: {
                    item->setText(1,tr("Center"));
                    ui->actAlignVCenter->setChecked(true);
                    break;
                }
                case 2: {
                    item->setText(1,tr("Bottom"));
                    ui->actAlignBottom->setChecked(true);
                    break;
                }
                default: item->setText(1,"");
            }
            break;
        }
        case Height: {
            item->setText(0,tr("Height"));
            item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable);
            item->setText(1, value.toString());
            break;
        }
        case Width: {
            item->setText(0,tr("Width"));
            item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable);
            item->setText(1,QString::number(value.toInt()-1));
            break;
        }
        case Left: {
            item->setText(0,tr("Left"));
            item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable);
            item->setText(1, value.toString());
            break;
        }
        case Length: {
            item->setText(0,tr("Length"));
            item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable);
            item->setText(1, value.toString());
            break;
        }
        case Top: {
            item->setText(0,tr("Top"));
            item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable);
            item->setText(1, value.toString());
            break;
        }
        case Frame:
        case Font: {
            item->setText(0,value.toString());
            item->setText(1,"");
            break;
        }
        case FrameLeft: {
            parentNode = findItemInTree(Frame);
            if (item == nullptr)
                item = new QTreeWidgetItem(parentNode);
            item->setText(0,tr("Left"));
            if (value.toBool()) item->setCheckState(1,Qt::Checked);
            else item->setCheckState(1,Qt::Unchecked);
            ui->actLineLeft->setChecked(value.toBool());
            break;
        }
        case FrameRight: {
            parentNode = findItemInTree(Frame);
            if (item == nullptr)
                item = new QTreeWidgetItem(parentNode);
            item->setText(0,tr("Right"));
            if (value.toBool()) item->setCheckState(1,Qt::Checked);
            else item->setCheckState(1,Qt::Unchecked);
            ui->actLineRight->setChecked(value.toBool());
            break;
        }
        case FrameTop: {
            parentNode = findItemInTree(Frame);
            if (item == nullptr)
                item = new QTreeWidgetItem(parentNode);
            item->setText(0,tr("Top"));
            if (value.toBool()) item->setCheckState(1,Qt::Checked);
            else item->setCheckState(1,Qt::Unchecked);
            ui->actLineTop->setChecked(value.toBool());
            break;
        }
        case FrameBottom: {
            parentNode = findItemInTree(Frame);
            if (item == nullptr)
                item = new QTreeWidgetItem(parentNode);
            item->setText(0,tr("Bottom"));
            if (value.toBool()) item->setCheckState(1,Qt::Checked);
            else item->setCheckState(1,Qt::Unchecked);
            ui->actLineBottom->setChecked(value.toBool());
            break;
        }
        case FrameNo: {
            setParamTree(FrameLeft,false,true);
            setParamTree(FrameRight,false,true);
            setParamTree(FrameTop,false,true);
            setParamTree(FrameBottom,false,true);
            ui->actLineAll->setChecked(false);
            ui->actLineNo->setChecked(true);
            break;
        }
        case FrameAll: {
            setParamTree(FrameLeft,true,true);
            setParamTree(FrameRight,true,true);
            setParamTree(FrameTop,true,true);
            setParamTree(FrameBottom,true,true);
            ui->actLineAll->setChecked(true);
            break;
        }
        case FrameWidth: {
            item->setText(0,tr("FrameWidth"));
            item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable);
            item->setText(1, value.toString());
            this->cbFrameWidth->setCurrentIndex(this->cbFrameWidth->findText(value.toString()));
            break;
        }
        case FontName: {
            parentNode = findItemInTree(Font);
            if (item == nullptr)
                item = new QTreeWidgetItem(parentNode);
            item->setText(0,tr("Name"));
            item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable);
            item->setText(1,value.toString());
            cbFontName->setCurrentFont(QFont(value.toString()));
            break;
        }
        case FontSize: {
            parentNode = findItemInTree(Font);
            if (item == nullptr)
                item = new QTreeWidgetItem(parentNode);
            item->setText(0,tr("Size"));
            item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable);
            value = value.toInt();
            item->setText(1,value.toString());
            cbFontSize->setEditText(value.toString());
            break;
        }
        case Bold: {
            parentNode = findItemInTree(Font);
            if (item == nullptr)
                item = new QTreeWidgetItem(parentNode);
            item->setText(0,tr("Bold"));
            if (value.toBool()) item->setCheckState(1,Qt::Checked);
            else item->setCheckState(1,Qt::Unchecked);
            ui->actionBold->setChecked(value.toBool());
            break;
        }
        case Italic: {
            parentNode = findItemInTree(Font);
            if (item == nullptr)
                item = new QTreeWidgetItem(parentNode);
            item->setText(0,tr("Italic"));
            if (value.toBool()) item->setCheckState(1,Qt::Checked);
            else item->setCheckState(1,Qt::Unchecked);
            ui->actionItalic->setChecked(value.toBool());
            break;
        }
        case Underline: {
            parentNode = findItemInTree(Font);
            if (item == nullptr)
                item = new QTreeWidgetItem(parentNode);
            item->setText(0,tr("Underline"));
            if (value.toBool()) item->setCheckState(1,Qt::Checked);
            else item->setCheckState(1,Qt::Unchecked);
            ui->actionUnderline->setChecked(value.toBool());
            break;
        }
        case Strikeout: {
            parentNode = findItemInTree(Font);
            if (item == nullptr)
                item = new QTreeWidgetItem(parentNode);
            item->setText(0,tr("Strikeout"));
            if (value.toBool()) item->setCheckState(1,Qt::Checked);
            else item->setCheckState(1,Qt::Unchecked);
            ui->actionStrikeout->setChecked(value.toBool());
            break;
        }
        case Printing: {
            item->setText(0,tr("Printing"));
            if (value.toBool()) item->setCheckState(1,Qt::Checked);
            else item->setCheckState(1,Qt::Unchecked);
            break;
        }
        case DSName: {
            item->setText(0,tr("DataSourceName"));
            item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable);
            item->setText(1,value.toString());
            break;
        }
        case ShowInGroup: {
            item->setText(0,tr("Show In Group"));
            if (value.toBool()) item->setCheckState(1,Qt::Checked);
            else item->setCheckState(1,Qt::Unchecked);
            break;
        }
        case GroupParam: {
            item->setText(0,value.toString());
            item->setText(1,"");
            break;
        }
        case StartNewNumeration: {
            parentNode = findItemInTree(GroupParam);
            if (item == nullptr)
                item = new QTreeWidgetItem(parentNode);
            item->setText(0,tr("Start New Numeration"));
            if (value.toBool()) item->setCheckState(1,Qt::Checked);
            else item->setCheckState(1,Qt::Unchecked);
            break;
        }

        case StartNewPage: {
            parentNode = findItemInTree(GroupParam);
            if (item == nullptr)
                item = new QTreeWidgetItem(parentNode);
            item->setText(0,tr("Start New Page"));
            if (value.toBool()) item->setCheckState(1,Qt::Checked);
            else item->setCheckState(1,Qt::Unchecked);
            break;
        }
        case GroupFields: {
            parentNode = findItemInTree(GroupParam);
            if (item == nullptr)
                item = new QTreeWidgetItem(parentNode);
            item->setText(0,tr("Groups field"));
            item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable);
            item->setText(1, value.toString());
            break;
        }
        case GroupLevel: {
            item->setText(0,tr("Group Level"));
            item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable);
            item->setText(1, value.toString());
            break;
        }
        case AutoHeight: {
            item->setText(0,tr("AutoHeight"));
            if (value.toBool()) item->setCheckState(1,Qt::Checked);
            else item->setCheckState(1,Qt::Unchecked);
            break;
        }
        case RenderingMode: {
            item->setText(0,tr("HTML rendering"));
            if (value.toBool()) item->setCheckState(1,Qt::Checked);
            else item->setCheckState(1,Qt::Unchecked);
            break;
        }
        case IgnoreRatioAspect: {
            item->setText(0,tr("IgnoreRatioAspect"));
            if (value.toBool()) item->setCheckState(1,Qt::Checked);
            else item->setCheckState(1,Qt::Unchecked);
            break;
        }
        case ArrowStart: {
            item->setText(0,tr("ArrowStart"));
            if (value.toBool()) item->setCheckState(1,Qt::Checked);
            else item->setCheckState(1,Qt::Unchecked);
            break;
        }
        case ArrowEnd: {
            item->setText(0,tr("ArrowEnd"));
            if (value.toBool()) item->setCheckState(1,Qt::Checked);
            else item->setCheckState(1,Qt::Unchecked);
            break;
        }
        case TextWrap: {
            item->setText(0,tr("TextWrap"));
            if (value.toBool()) item->setCheckState(1,Qt::Checked);
            else item->setCheckState(1,Qt::Unchecked);
            break;
        }
        case TextRotate: {
            item->setText(0,tr("TextRotate"));
            item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable);
            switch(value.toInt()) {
                case 0: {
                    item->setText(1,tr("0 Degres"));
                    break;
                }
                case 1: {
                    item->setText(1,tr("90 Degres"));
                    break;
                }
                case 2: {
                    item->setText(1,tr("180 Degres"));
                    break;
                }
                case 3: {
                    item->setText(1,tr("270 Degres"));
                    break;
                }
                default: item->setText(1,"");
            }
            break;
        }
        case BackgroundColor: {
            item->setText(0,tr("BackgroundColor"));
            item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsEditable);
            item->setText(1,colorToString(value.toString()));
            break;
        }
        case BorderColor: {
            item->setText(0,tr("BorderColor"));
            item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsEditable);
            item->setText(1,colorToString(value.toString()));
            break;
        }
        case FontColor: {
            parentNode = findItemInTree(Font);
            if (item == nullptr)
                item = new QTreeWidgetItem(parentNode);
            item->setText(0,tr("FontColor"));
            item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsEditable);
            item->setText(1,colorToString(value.toString()));
            break;
        }
        case BarcodeType: {
            item->setText(0,tr("BarcodeType"));
            item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable);
            item->setText(1,BarCode::getTypeName((BarCode::BarcodeTypes)value.toInt() ));
            break;
        }
        case BarcodeFrameType: {
            item->setText(0,tr("BarcodeFrameType"));
            item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable);
            item->setText(1,BarCode::getFrameTypeName((BarCode::FrameTypes)value.toInt() ));
            break;
        }
        default: {
            return;
        }
    }

    GraphicsBox *box = nullptr;
    auto selItem = selectedGItem();
    if (selItem && selItem->type() == ItemType::GBox)
        box = static_cast<GraphicsBox *>(selItem);

    if (box != nullptr) {
        bool top = box->borderIsCheck(FrameTop);
        bool bottom = box->borderIsCheck(FrameBottom);
        bool left = box->borderIsCheck(FrameLeft);
        bool right = box->borderIsCheck(FrameRight);

        if (top && bottom && left && right) {
            ui->actLineAll->setChecked(true);
            ui->actLineNo->setChecked(false);
        } else
            ui->actLineAll->setChecked(false);

        if (!top && !bottom && !left && !right) {
            ui->actLineAll->setChecked(false);
            ui->actLineNo->setChecked(true);
        } else
            ui->actLineNo->setChecked(false);
    }

    item->setData(1,Qt::UserRole,command);

    if (parentNode == nullptr)
        ui->treeParams->addTopLevelItem(item);
    else
        parentNode->addChild(item);
}

void MainWindow::selTree(QTreeWidgetItem *tItem, int)
{
    auto repPage = qobject_cast<RepScrollArea *>(ui->tabWidget->currentWidget());
    repPage->scene->unselectAll();

    for (auto &item : repPage->scene->items()) {
        if (item == nullptr) {
            continue;
        } else {
            if (item->type() == ItemType::GLine) {
                auto line = static_cast<GraphicsLine *>(item);
                if (line->itemInTree == tItem)
                    line->setSelected(true);
            }
            if (item->type() == ItemType::GBox) {
                auto box = static_cast<GraphicsBox *>(item);
                if (box->itemInTree == tItem)
                    box->setSelected(true);
            }
            if (item->type() == ItemType::GBand) {
                auto band = static_cast<ReportBand *>(item);
                if (band->itemInTree == tItem)
                    band->setSelected(true);
            }
        }
    }
}

void MainWindow::setSelectionMode()
{
    auto repPage = qobject_cast<RepScrollArea *>(ui->tabWidget->currentWidget());
    repPage->scene->setMode(GraphicsScene::Mode::SelectObject);
}

void MainWindow::enableAdding()
{
    auto repPage = qobject_cast<RepScrollArea *>(ui->tabWidget->currentWidget());
    ui->actAddField->setEnabled(repPage->allowField());
    ui->actAddRichText->setEnabled(repPage->allowField());
    ui->actAddPicture->setEnabled(repPage->allowField());
    ui->actAddDiagram->setEnabled(repPage->allowField());
    ui->actAddBarcode->setEnabled(repPage->allowField());
    ui->actAddDrawing->setEnabled(repPage->allowField());
    ui->actAddCrossTab->setEnabled(repPage->allowField());
    //ui->actAddCrossTabBD->setEnabled(repPage->allowField());
}

void MainWindow::addBand()
{
    BandType type = Undefined;
    if (sender()->objectName() == "actRepTitle") type = ReportTitle;
    if (sender()->objectName() == "actReportSummary") type = ReportSummary;
    if (sender()->objectName() == "actPageHeader") type = PageHeader;
    if (sender()->objectName() == "actPageFooter") type = PageFooter;
    if (sender()->objectName() == "actMasterData") type = MasterData;
    if (sender()->objectName() == "actMasterFooter") type = MasterFooter;
    if (sender()->objectName() == "actMasterHeader") type = MasterHeader;
    if (sender()->objectName() == "actDataGroupingHeader") type = DataGroupHeader;
    if (sender()->objectName() == "actDataGroupingFooter") type = DataGroupFooter;

    ui->actSelect_tool->setChecked(true);
    ui->actSaveReport->setEnabled(true);

    checkAddBandPermission();

    auto repPage = qobject_cast<RepScrollArea *>(ui->tabWidget->currentWidget());
    if (repPage != nullptr) {
        repPage->m_addBand(type, bandMenu);
        enableAdding();
    }
}

void MainWindow::checkAddBandPermission()
{
    this->actRepTitle->setEnabled(false);
    this->actReportSummary->setEnabled(false);
    this->actPageHeader->setEnabled(false);
    this->actPageFooter->setEnabled(false);
    this->actMasterData->setEnabled(false);
    this->actMasterFooter->setEnabled(false);
    this->actMasterHeader->setEnabled(false);
    this->actDataGroupingHeader->setEnabled(false);
    this->actDataGroupingFooter->setEnabled(false);

    int cntRT  = 0;
    int cntRS  = 0;
    int cntPH  = 0;
    int cntPF  = 0;
    int cntMD  = 0;
    int cntMF  = 0;
    int cntMH  = 0;
    int cntDGH = 0;
    int cntDGF = 0;

    auto repPage = qobject_cast<RepScrollArea *>(ui->tabWidget->widget(ui->tabWidget->currentIndex()));
    auto allReportBand = repPage->getReportBands();

    for (auto &band : allReportBand) {
        if (band->bandType == ReportTitle) cntRT++;
        if (band->bandType == ReportSummary) cntRS++;
        if (band->bandType == PageHeader) cntPH++;
        if (band->bandType == PageFooter) cntPF++;
        if (band->bandType == MasterData) cntMD++;
        if (band->bandType == MasterFooter) cntMF++;
        if (band->bandType == MasterHeader) cntMH++;
        if (band->bandType == DataGroupHeader) cntDGH++;
        if (band->bandType == DataGroupFooter) cntDGF++;
    }

    if (cntRT < 1) this->actRepTitle->setEnabled(true);
    if (cntRS < 1) this->actReportSummary->setEnabled(true);
    if (cntPH < 1) this->actPageHeader->setEnabled(true);
    if (cntPF < 1) this->actPageFooter->setEnabled(true);
    if (cntMD < 6) this->actMasterData->setEnabled(true);
    if (cntMF < 1) this->actMasterFooter->setEnabled(true);
    if (cntMH < 6) this->actMasterHeader->setEnabled(true);
    if (cntDGH < 1) this->actDataGroupingHeader->setEnabled(true);
    if (cntDGF < 3) this->actDataGroupingFooter->setEnabled(true);
}

void MainWindow::addField(FieldType type)
{
    auto repPage = qobject_cast<RepScrollArea *>(ui->tabWidget->widget( ui->tabWidget->currentIndex() ));
    repPage->scene->newFieldType(type);
    repPage->scene->newFieldMenu(contMenu);
    repPage->scene->setMode(GraphicsScene::Mode::DrawContainer);
    //убрать? ui->actSelect_tool->setChecked(true);
    //убрать? ui->actSaveReport->setEnabled(true);
}

void MainWindow::addDraw()
{
    FieldType fieldType = Text;
    if (sender()->objectName() == "actDrawRectangle")
        fieldType = Reactangle;
    if (sender()->objectName() == "actDrawRoundedRectangle")
        fieldType = RoundedReactangle;
    if (sender()->objectName() == "actDrawEllipse")
        fieldType = Circle;
    if (sender()->objectName() == "actDrawTriangle")
        fieldType = Triangle;
    if (sender()->objectName() == "actDrawRhombus")
        fieldType = Rhombus;

    auto repPage = qobject_cast<RepScrollArea *>(ui->tabWidget->widget( ui->tabWidget->currentIndex() ));
    repPage->scene->newFieldType(fieldType);

    if (QtRPT::getDrawingFields().contains(fieldType)) {
        addField(fieldType);
    } else {
        fieldType = QtRptName::FieldType::Line;
        repPage->scene->newFieldType(fieldType);
        repPage->scene->newFieldMenu(contMenu);
        repPage->scene->setMode(GraphicsScene::Mode( int(GraphicsScene::Mode::DrawLine) ));

        if (sender()->objectName() == "actDrawLine2") {
            repPage->scene->newLineArrowStart = false;
            repPage->scene->newLineArrowEnd = false;
        }
        if (sender()->objectName() == "actDrawLine3") {
            repPage->scene->newLineArrowStart = false;
            repPage->scene->newLineArrowEnd = true;
        }
        if (sender()->objectName() == "actDrawLine4") {
            repPage->scene->newLineArrowStart = true;
            repPage->scene->newLineArrowEnd = false;
        }
        if (sender()->objectName() == "actDrawLine5") {
            repPage->scene->newLineArrowStart = true;
            repPage->scene->newLineArrowEnd = true;
        }
    }
}

void MainWindow::showPreview()
{
    for (auto &plugin : plugins) {
        auto echoInterface = qobject_cast<CustomInterface *>(plugin);

        int index = plugin->metaObject()->indexOfClassInfo("ShowReport");
        if (QString(plugin->metaObject()->classInfo(index).value()) == "true") {
            if (echoInterface) {
                // We use plugin's report preview
                echoInterface->showReport(xmlDoc);
                return;
            }
        }
    }

    auto report = QtRPT::createSPtr(this);

    if (fileName.isEmpty())
        report->loadReport(*xmlDoc);
    else
        report->loadReport(fileName);

    if (pdfName.isEmpty())
        report->printExec();
    else
        report->printPDF(pdfName, false);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    Q_UNUSED(event);

    if (ui->actSaveReport->isEnabled()) {
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, tr("Saving"),tr("The report was changed.\nSave the report?"),
                                         QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
        if (reply == QMessageBox::Yes)
            saveReport();
        else if (reply == QMessageBox::Cancel)
            event->ignore();
    }
}

void MainWindow::changeZoom()
{
    auto repPage = qobject_cast<RepScrollArea *>(ui->tabWidget->currentWidget());
    repPage->setScale(cbZoom->currentText());
}

void MainWindow::sceneClick()
{
    if (ui->actMagnifying->isChecked()) {
        qreal scale;
        auto repPage = qobject_cast<RepScrollArea *>(ui->tabWidget->currentWidget());
        if (QApplication::keyboardModifiers() != Qt::ShiftModifier) scale = repPage->getScale()+0.25;
        else scale = repPage->getScale()-0.25;
        cbZoom->setEditText(QString::number(scale*100)+"%");
        repPage->setScale(cbZoom->currentText());
    }
}

bool MainWindow::eventFilter(QObject *obj, QEvent *e)
{
    if (e->type() == QEvent::Wheel ) {
        QWheelEvent *m = static_cast< QWheelEvent * >( e );
        if (QApplication::keyboardModifiers() == Qt::ControlModifier && obj->objectName() == "RepScrollArea") {
            qreal scale;
            auto repPage = qobject_cast<RepScrollArea *>(ui->tabWidget->currentWidget());
            if (m->angleDelta().y() > 0 )
                scale = repPage->getScale()+0.25;
            else
                scale = repPage->getScale()-0.25;

            cbZoom->setEditText(QString::number(scale*100)+"%");
            repPage->setScale(cbZoom->currentText());
            return true;
        }
    }
    if (e->type() == QEvent::KeyPress) {
        if (ui->actMagnifying->isChecked()) {
            QKeyEvent *m = static_cast< QKeyEvent * >( e );
            if (m->key() == Qt::Key_Shift)
                setCursor(QCursor(QPixmap(QString::fromUtf8(":/new/prefix1/images/zoom_out.png"))));
        }
    }
    if (e->type() == QEvent::KeyRelease) {
        if (ui->actMagnifying->isChecked()) {
            QKeyEvent *m = static_cast< QKeyEvent * >( e );
            if (m->key() == Qt::Key_Shift)
                setCursor(QCursor(QPixmap(QString::fromUtf8(":/new/prefix1/images/zoom_in.png"))));
        }
    }

    if (e->type() == QEvent::Enter) {
        if (obj->objectName() == "RepScrollArea") {
            if (ui->actMagnifying->isChecked()) {
                if (QApplication::keyboardModifiers() == Qt::ShiftModifier)
                    setCursor(QCursor(QPixmap(QString::fromUtf8(":/new/prefix1/images/zoom_out.png"))));
                else
                    setCursor(QCursor(QPixmap(QString::fromUtf8(":/new/prefix1/images/zoom_in.png"))));
            }
        }
    }
    if (e->type() == QEvent::Leave) {
        if (obj->objectName() == "RepScrollArea")
            setCursor(Qt::ArrowCursor);
    }
    if (e->type() == QMouseEvent::MouseButtonPress) {
        if (!ui->actMagnifying->isChecked()) {
            QToolButton *tb = qobject_cast<QToolButton*>(obj);
            if (tb != 0 && tb->actions().at(0) == ui->actFrameStyle)
                showFrameStyle(tb->pos());
        }
    }
    return QWidget::eventFilter(obj,e);
}

//Process click on CheckBox
void MainWindow::itemChanged(QTreeWidgetItem *item, int column)
{
    if (column == 1 ) {
        if (selectedGItem() == nullptr) return;
            QVariant v;
            Command command = (Command)item->data(1,Qt::UserRole).toInt();
            switch (command) {
                case TextWrap:
                case IgnoreRatioAspect:
                case ArrowStart:
                case ArrowEnd:
                case AutoHeight:
                case RenderingMode:
                case StartNewPage:
                case ShowInGroup:
                case StartNewNumeration:
                case Printing:
                case Bold:
                case Italic:
                case Underline:
                case Strikeout:
                case FrameLeft:
                case FrameRight:
                case FrameTop:
                case FrameBottom: {
                    if (item->checkState(1) == Qt::Checked) v = true;
                    else v = false;
                    break;
                }
                default: {
                    return;
                }
            }
            execButtonCommand(command,v);
    }
}

//Change param in paramTree
void MainWindow::closeEditor()
{
    auto item = ui->treeParams->currentItem();
    if (item == nullptr) return;
    if (selectedGItem() == nullptr) return;

    QVariant v;
    auto command = static_cast<Command>(item->data(1,Qt::UserRole).toInt());
    switch (command) {
        case Name: {
            if (checkName(item->text(1))) {
                auto field = static_cast<GraphicsBox *>(selectedGItem());
                auto delegate = static_cast<EditorDelegate *>(sender());
                if (delegate != nullptr) {
                    delegate->blockSignals(true);

                    QString str = QString(tr("The name '%1' already used!")).arg(item->text(1));
                    QMessageBox::warning(this, tr("Error"), str);

                    item->setText(1, field->objectName());
                    delegate->blockSignals(false);
                }

                return;
            } else {
                v = item->text(1);
            }
            break;
        }
        case TextRotate:
        case DSName:
        case GroupFields:
        case GroupLevel:
        case Left:
        case Top:
        case Height:
        case Length:
        case AligmentH:
        case AligmentV:
        case BarcodeType:
        case BarcodeFrameType:
        case FrameWidth:
        case FontName:
        case FontSize: {
            v = item->text(1);
            break;
        }
        case Width: {
            v = item->text(1).toInt()+1;
            break;
        }
        default:
            return;
    }
    execButtonCommand(command, v);
}

void MainWindow::clipBoard()
{
    auto repPage = qobject_cast<RepScrollArea *>(ui->tabWidget->currentWidget());

    if (sender() == ui->actCopy) {
        pasteCopy = true;
        cloneContList->clear();
        for (auto &item : repPage->scene->items()) {
            if (item->type() == ItemType::GLine || item->type() == ItemType::GBox) {
                auto helper = gItemToHelper(item);
                if (helper->helperIsSelected())
                    cloneContList->append(item);
            }
        }
        ui->actPaste->setEnabled(true);
    }
    if (sender() == ui->actCut) {
        pasteCopy = false;
        cloneContList->clear();
        for (auto &item : repPage->scene->items()) {
            if (item->type() == ItemType::GLine || item->type() == ItemType::GBox) {
                auto helper = gItemToHelper(item);
                if (helper->helperIsSelected()) {
                    cloneContList->append(item);
                    item->setVisible(false);
                }
            }
        }
        ui->actPaste->setEnabled(true);
    }
    if (sender() == ui->actPaste) {
        if (cloneContList->isEmpty()) return;
        auto scene = repPage->scene;
        ReportBand *band = nullptr;
        if (!scene->selectedItems().isEmpty()) {
            auto item = scene->selectedItems().at(0);
            if (item->type() == ItemType::GBand)
                band = static_cast<ReportBand*>(item);
            else
                band = static_cast<ReportBand*>(item->parentItem());
        }
        if (band == nullptr) return;

        QList<QGraphicsItem*> newContList;
        ui->treeWidget->setSelectionMode(QAbstractItemView::MultiSelection);
        for (auto &item : *cloneContList) {
            item->setSelected(false);
            if (item->type() == ItemType::GBox) {
                auto box = static_cast<GraphicsBox *>(item);
                box->setSelected(false);

                if (pasteCopy == true) {
                    auto newBox = box->clone();
                    generateName(newBox);
                    scene->addItem(newBox);
                    newBox->setParentItem(band);
                    repPage->newFieldTreeItem(newBox);
                    newBox->setSelected(true);
                    newBox->setMenu(contMenu);
                    newBox->setParentItem(band);
                    newContList << newBox;
                } else {
                    box->setParentItem(band);
                    box->setVisible(true);
                    box->setSelected(true);
                    auto tItem = box->itemInTree->parent()->takeChild(box->itemInTree->parent()->indexOfChild(box->itemInTree));
                    band->itemInTree->addChild(tItem);
                    band->itemInTree->setExpanded(true);
                }
            }
            if (item->type() == ItemType::GLine) {
                auto line = static_cast<GraphicsLine *>(item);
                line->setSelected(false);

                if (pasteCopy == true) {
                    auto newLine = line->clone();
                    generateName(newLine);
                    scene->addItem(newLine);
                    newLine->setParentItem(band);
                    repPage->newFieldTreeItem(newLine);
                    newLine->setSelected(true);
                    newLine->setMenu(contMenu);
                    newLine->setParentItem(band);
                } else {
                    line->setParentItem(band);
                    line->setVisible(true);
                    line->setSelected(true);
                    QTreeWidgetItem *tItem = line->itemInTree->parent()->takeChild(line->itemInTree->parent()->indexOfChild(line->itemInTree));
                    band->itemInTree->addChild(tItem);
                    band->itemInTree->setExpanded(true);
                }
            }
        }

        for (auto &item : newContList) {
            auto box = static_cast<GraphicsBox *>(item);
            box->setSelected(true);
        }

        showParamState();

        ui->treeWidget->setSelectionMode(QAbstractItemView::SingleSelection);
        ui->actSaveReport->setEnabled(true);
    }
}

MainWindow::~MainWindow()
{
    while (!pluginsLoaders.isEmpty()) {
        auto pluginLoader = pluginsLoaders.takeFirst();
        pluginLoader->unload();
        delete pluginLoader;
    }

    delete ui;
}

void MainWindow::mousePos(QPointF pos)
{
    m_status1->setText(QString("X: %1 Y: %2").arg(pos.x()).arg(pos.y()));
}

void MainWindow::setReportChanged()
{
    ui->actSaveReport->setEnabled(true);
}

QStringList MainWindow::getWords()
{
    return m_words;
}

void MainWindow::setWords(QStringList words)
{
    m_words = words;
}

QStringList MainWindow::getArguments()
{
    return m_args;
}
