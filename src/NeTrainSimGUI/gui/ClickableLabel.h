#ifndef CLICKABLELABEL_H
#define CLICKABLELABEL_H

#include <QLabel>
#include <QUrl>

class ClickableLabel : public QLabel {
    Q_OBJECT

public:
    explicit ClickableLabel(const QString &text,
                            const QUrl &url,
                            const QColor &textColor,
                            QWidget *parent = nullptr);

protected:
    void mousePressEvent(QMouseEvent *event) override;

private:
    QUrl m_url; // Private member to store the URL
};

#endif // CLICKABLELABEL_H
