#include "ClickableLabel.h"
#include <QDesktopServices>
#include <QMouseEvent>

ClickableLabel::ClickableLabel(const QString &text,
                               const QUrl &url,
                               const QColor &textColor,
                               QWidget *parent)
    : QLabel(text, parent), m_url(url) {
    setCursor(Qt::PointingHandCursor); // Change cursor on hover

    // Set the text color using the provided QColor
    setStyleSheet(QString("QLabel { color : %1; }").arg(textColor.name()));
}

void ClickableLabel::mousePressEvent(QMouseEvent *event) {
    QLabel::mousePressEvent(event);
    QDesktopServices::openUrl(m_url); // Open the stored URL
}
