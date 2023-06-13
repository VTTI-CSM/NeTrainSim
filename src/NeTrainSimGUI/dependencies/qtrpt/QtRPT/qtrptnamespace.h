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

#pragma once

#include <QObject>
#include <QMetaType>
#include <QColor>

namespace QtRptName {
    enum BandType
    {
        Undefined = 0,
        PageHeader = 1,
        ReportTitle = 2,
        DataGroupHeader = 4,//3,
        MasterHeader = 3,//4,
        MasterData = 5,
        MasterFooter = 7,//6,
        DataGroupFooter = 6,//7,
        ReportSummary = 8,
        PageFooter = 9
    };

    enum FieldType
    {
        Text,
        TextImage,
        TextRich,
        Image,
        Diagram,
        Reactangle,
        RoundedReactangle,
        Circle,
        Triangle,
        Rhombus,
        Line,
        Barcode,
        DatabaseImage,
        CrossTab
    };

    enum BorderStyle
    {
        Dashed,
        Dot_dash,
        Dot_dot_dash,
        Dotted,
        Double,
        Groove,
        Inset,
        Outset,
        Ridge,
        Solid,
        BorderNone
    };

    enum HiType
    {
        FntBold,
        FntItalic,
        FntUnderline,
        FntStrikeout,
        FntColor,
        BgColor
    };

    enum Command
    {
        None,
        Name,
        Bold,
        Italic,
        Underline,
        Strikeout,
        Font,
        FontSize,
        FontColor,
        FontName,
        Frame,
        FrameLeft,
        FrameRight,
        FrameTop,
        FrameBottom,
        FrameNo,
        FrameAll,
        FrameStyle,
        FrameWidth,
        AligmentH,
        AligmentV,
        Left,
        Top,
        Width,
        Height,
        Length,
        BackgroundColor,
        BorderColor,
        Printing,
        GroupParam,
        StartNewNumeration,
        ShowInGroup,
        StartNewPage,
        GroupLevel,
        GroupFields,
        AutoHeight,
        ArrowStart,
        ArrowEnd,
        IgnoreRatioAspect,
        BarcodeType,
        BarcodeFrameType,
        TextWrap,
        TextRotate,
        DSName,
        RenderingMode
    };
}

Q_DECLARE_METATYPE(QtRptName::FieldType)

struct GraphValue {
    QString caption;  //for Pie, for Line - ignore
    double valueX;    //for Line only
    double valueY;
};

struct GraphData {
    QList<GraphValue> valueList;
    QColor color;
    QString graphDS;
    QString caption;  //for Pie - ignore
};

typedef QList<GraphData> GraphDataList;
