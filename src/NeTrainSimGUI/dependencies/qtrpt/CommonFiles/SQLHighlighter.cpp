// $Id: SQLHighlighter.cpp 20 2006-10-10 08:24:36Z tb $
// Copyright 2006 Timo Bingmann
// Modified 2011 Aleksey Osipov
// SQL syntax highlight plugin for QTextEdit
//

#include "SQLHighlighter.h"

FontHihgligthParam SQLHighlighter::getFontColor(int type)
{
    FontHihgligthParam fh;
    for (int i = 0; i < lst.size(); i++) {
        if (lst.at(i).type == type)
            return lst.at(i);
    }

    return fh;
}

void SQLHighlighter::fillArray()
{
    FontHihgligthParam fh;

    fh.type = Comment;
    fh.remark = "comments";
    fh.bold = false;
    fh.italic = false;
    fh.color = "green";
    lst.append(fh);

    fh.type = QuotedString;
    fh.remark = "quoted strings";
    fh.bold = false;
    fh.italic = false;
    fh.color = "black";
    lst.append(fh);

    fh.type = PrimaryKeyword;
    fh.remark = "primary keywords";
    fh.bold = false;
    fh.italic = false;
    fh.color = "blue";
    lst.append(fh);

    fh.type = TypeKeyword;
    fh.remark = "type keywords";
    fh.bold = false;
    fh.italic = false;
    fh.color = "darkGreen";
    lst.append(fh);

    fh.type = ExoticKeywords;
    fh.remark = "more exotic keywords";
    fh.bold = false;
    fh.italic = false;
    fh.color = "darkBlue";
    lst.append(fh);
}

void SQLHighlighter::saveSettings(QSettings *settings)
{
    settings->beginGroup("SQLEHighligheter");
    settings->beginWriteArray("Params");

    for (int i = 0; i < lst.size(); ++i) {
        settings->setArrayIndex(i);
        FontHihgligthParam fh = lst.at(i);
        settings->setValue("Bold", fh.bold);
        settings->setValue("Italic", fh.italic);
        settings->setValue("Color", fh.color);
        settings->setValue("Remark", fh.remark);
        settings->setValue("Type", fh.type);
    }
    settings->endArray();
    settings->endGroup();
}

// largely based on the highlight example in qt's docs
SQLHighlighter::SQLHighlighter(class QTextDocument *parent, QSettings *settings) : QSyntaxHighlighter(parent)
{
    //qRegisterMetaType<FontHihgligth>("FontHihgligth");
    if (settings == nullptr) {
        fillArray();
    } else {
        //get Font params from INI file
        settings->beginGroup("SQLEHighligheter");
        int listSize = settings->beginReadArray("Params");
        if (listSize > 0) {
            lst.clear();
            for (int i = 0; i < listSize; ++i) {
                settings->setArrayIndex(i);
                FontHihgligthParam fh;
                fh.type = (FontHihgligth)settings->value("Type").toInt();
                fh.remark = settings->value("Remark").toString();
                fh.bold = settings->value("Bold").toBool();
                fh.italic = settings->value("Italic").toBool();
                fh.color = settings->value("Color").toString();
                lst.append(fh);
            }
        } else {
            fillArray();
        }
        settings->endArray();
        settings->endGroup();
    }

    // comments
    commentFormat.setForeground(QColor(getFontColor(Comment).color));
    commentFormat.setFontItalic(getFontColor(Comment).italic);
    if (getFontColor(Comment).bold) {
        commentFormat.setFontWeight(QFont::Bold);
    } else {
        commentFormat.setFontWeight(QFont::Normal);
    }

	rules.push_back( Rule("--[^\n]*", commentFormat) );
    rules.push_back( Rule("//[^\n]*", commentFormat) );

    commentStartExpression = QRegularExpression("/\\*");
    commentEndExpression = QRegularExpression("\\*/");

    // quoted strings
	QTextCharFormat quotationFormat;
    quotationFormat.setForeground(QColor(getFontColor(QuotedString).color));
    quotationFormat.setFontItalic(getFontColor(QuotedString).italic);
    if (getFontColor(QuotedString).bold) {
        quotationFormat.setFontWeight(QFont::Bold);
    } else {
        quotationFormat.setFontWeight(QFont::Normal);
    }

	rules.push_back( Rule("\".*\"", quotationFormat) );
	rules.push_back( Rule("'.*'", quotationFormat) );

    // primary keywords
	QTextCharFormat keywordFormat;
    keywordFormat.setForeground(QColor(getFontColor(PrimaryKeyword).color));
    keywordFormat.setFontItalic(getFontColor(PrimaryKeyword).italic);
    if (getFontColor(PrimaryKeyword).bold) {
        keywordFormat.setFontWeight(QFont::Bold);
    } else {
        keywordFormat.setFontWeight(QFont::Normal);
    }

	QString str = "begin commit rollback select from where explain insert into values update create delete drop grant revoke lock set truncate as join on table view order group by having";

	str.replace(" ", "|");
	rules.push_back( Rule("\\b(" + str + ")\\b", keywordFormat) );

    // type keywords
	QTextCharFormat typeFormat;
    typeFormat.setForeground(QColor(getFontColor(TypeKeyword).color));
    typeFormat.setFontItalic(getFontColor(TypeKeyword).italic);
    if (getFontColor(TypeKeyword).bold)
        typeFormat.setFontWeight(QFont::Bold);
    else
        typeFormat.setFontWeight(QFont::Normal);

    str = "anydata anydataset anytype array bfile bigint binary_double binary_float binary_integer bit blob boolean cfile char character clob date datetime day dburitype dec decimal double enum float float4 float8 flob httpuritype int int16 int2 int24 int4 int8 integer interval lob long longblob longlong longtext mediumblob mediumtext mlslabel month national nchar nclob newdate null number numeric nvarchar object pls_integer precision raw real record rowid second short single smallint time timestamp tiny tinyblob tinyint tinytext urifactorytype uritype urowid utc_date utc_time utc_timestamp varchar varchar2 varray varying xmltype year zone";

	str.replace(" ", "|");

	rules.push_back( Rule("\\b(" + str + ")\\b", typeFormat) );

    // more exotic keywords
    keywordFormat.setForeground(QColor(getFontColor(ExoticKeywords).color));
    keywordFormat.setFontItalic(getFontColor(ExoticKeywords).italic);
    if (getFontColor(ExoticKeywords).bold) {
        keywordFormat.setFontWeight(QFont::Bold);
    } else {
        keywordFormat.setFontWeight(QFont::Normal);
    }

    str = "abort abs absolute access action ada add admin after aggregate alias all allocate alter analyse analyze and any are asc asensitive assertion assignment asymmetric at atomic audit authorization avg backward before between bigint binary bit_length bitvar both breadth c cache call called cardinality cascade cascaded case cast catalog catalog_name chain change char_length character_length character_set_catalog character_set_name character_set_schema characteristics check checked checkpoint class class_origin close cluster coalesce cobol collate collation collation_catalog collation_name collation_schema column column_name command_function command_function_code comment committed completion compress condition condition_number connect connection connection_name constraint constraint_catalog constraint_name constraint_schema constraints constructor contains continue conversion convert copy corresponding count createdb createuser cross cube current current_date current_path current_role current_time current_timestamp current_user cursor cursor_name cycle data database databases datetime_interval_code datetime_interval_precision day_hour day_microsecond day_minute day_second deallocate declare default deferrable deferred defined definer delayed delimiter delimiters depth deref desc describe descriptor destroy destructor deterministic diagnostics dictionary disconnect dispatch distinct distinctrow div do domain dual dynamic dynamic_function dynamic_function_code each else elseif elsif enclosed encoding encrypted end end-exec equals escape escaped every except exception exclusive exec execute existing exists exit external extract false fetch file final first for force foreign fortran forward found free freeze full fulltext function g general generated get global go goto granted grouping handler hierarchy high_priority hold host hour hour_microsecond hour_minute hour_second identified identity if ignore ilike immediate immutable implementation implicit in increment index indicator infile infix inherits initial initialize initially inner inout input insensitive instance instantiable instead int1 int3 intersect invoker is isnull isolation iterate k key key_member key_type keys kill lancompiler language large last lateral leading leave left length less level like limit lines listen load local localtime localtimestamp location locator loop low_priority lower m map match max maxextents maxvalue mediumint message_length message_octet_length message_text method middleint min minus minute minute_microsecond minute_second minvalue mod mode modifies modify module more move mumps name names natural new next no no_write_to_binlog noaudit nocompress nocreatedb nocreateuser none not nothing notify notnull nowait nullable nullif octet_length of off offline offset oids old online only open operation operator optimize option optionally options or ordinality out outer outfile output overlaps overlay overriding owner pad parameter parameter_mode parameter_name parameter_ordinal_position parameter_specific_catalog parameter_specific_name parameter_specific_schema parameters partial pascal password path pctfree pendant placing pli position postfix prefix preorder prepare preserve primary prior privileges procedural procedure public purge raid0 read reads recheck recursive ref references referencing regexp reindex relative release rename repeat repeatable replace require reset resource restrict result return returned_length returned_octet_length returned_sqlstate returns right rlike role rollup routine routine_catalog routine_name routine_schema row row_count rowlabel rownum rows rule savepoint scale schema schema_name schemas scope scroll search second_microsecond section security self sensitive separator sequence serializable server_name session session_user setof sets share show similar simple size some soname source space spatial specific specific_name specifictype sql sqlcode sqlerror sqlexception sqlstate sqlwarning ssl stable start starting state statement static statistics stdin stdout storage straight_join strict structure style subclass_origin sublist substring successful sum symmetric synonym sysdate sysid system system_user table_name temp template temporary terminate terminated than then timezone_hour timezone_minute to toast trailing transaction transaction_active transactions_committed transactions_rolled_back transform transforms translate translation treat trigger true type uid undo union unique unlock unsigned usage use user using validate varbinary varcharacter when whenever while with write x509 xor year_month zerofillzgium";

	str.replace(" ", "|");
	rules.push_back( Rule("\\b(" + str + ")\\b", keywordFormat) );

}

void SQLHighlighter::highlightBlock(const QString &text)
{
    for (const Rule &rule : qAsConst(rules)) {
        QRegularExpressionMatchIterator matchIterator = rule.pattern.globalMatch(text);
        while (matchIterator.hasNext()) {
            QRegularExpressionMatch match = matchIterator.next();
            setFormat(match.capturedStart(), match.capturedLength(), rule.format);
        }
    }

    setCurrentBlockState(0);

    int startIndex = 0;
    if (previousBlockState() != 1)
        startIndex = text.indexOf(commentStartExpression);


    while (startIndex >= 0) {
        QRegularExpressionMatch match = commentEndExpression.match(text, startIndex);
        int endIndex = match.capturedStart();
        int commentLength = 0;
        if (endIndex == -1) {
            setCurrentBlockState(1);
            commentLength = text.length() - startIndex;
        } else {
            commentLength = endIndex - startIndex
                            + match.capturedLength();
        }
        setFormat(startIndex, commentLength, commentFormat);
        startIndex = text.indexOf(commentStartExpression, startIndex + commentLength);
    }
}

