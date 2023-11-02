#ifndef CHECKBOXHEADER_H
#define CHECKBOXHEADER_H

#include <QObject>
#include <QHeaderView>
#include <QCheckBox>
#include <QPainter>
#include <QMouseEvent>

class CheckBoxHeader : public QHeaderView
{
    Q_OBJECT
    QCheckBox *checkBox;
public:
    CheckBoxHeader(Qt::Orientation orientation, QWidget * parent = 0)
        : QHeaderView(orientation, parent)
    {
        checkBox = new QCheckBox(this);
        connect(checkBox, &QCheckBox::stateChanged, this, &CheckBoxHeader::checkBoxChanged);
    }
protected:
    void paintSection(QPainter *painter, const QRect &rect, int logicalIndex) const override
    {
        painter->save();
        QHeaderView::paintSection(painter, rect, logicalIndex);
        painter->restore();
        if (logicalIndex == 0)
        {
            QStyleOptionButton option;
            option.rect = checkBox->geometry();
            option.state = QStyle::State_Enabled | (checkBox->isChecked() ? QStyle::State_On : QStyle::State_Off);
            this->style()->drawPrimitive(QStyle::PE_IndicatorCheckBox, &option, painter);
        }
    }
    void mousePressEvent(QMouseEvent *event) override
    {
        if (logicalIndexAt(event->pos()) == 0)
        {
            if (checkBox->geometry().contains(event->pos()))
            {
                checkBox->click();
                event->ignore();
                return;
            }
        }
        QHeaderView::mousePressEvent(event);
    }
    QSize sectionSizeFromContents(int logicalIndex) const override
    {
        QSize size = QHeaderView::sectionSizeFromContents(logicalIndex);
        if (logicalIndex == 0)
        {
            size.setWidth(checkBox->sizeHint().width());
        }
        return size;
    }
public slots:
    void checkBoxChanged(int state)
    {
        for (int i = 0; i<count(); ++i)
        {
            setSectionResizeMode(i, QHeaderView::Interactive);
        }
        emit checkBoxClicked(state);
    }
signals:
    void checkBoxClicked(int);
};

#endif // CHECKBOXHEADER_H
