#ifndef CALCULATEBYTEWIDGET_H
#define CALCULATEBYTEWIDGET_H

#include <QPushButton>
#include <QObject>
#include <QWidget>
#include <QCheckBox>

class CalculateByteWidget : public QWidget
{
    Q_OBJECT
public:
    explicit CalculateByteWidget(int numBits, QWidget *parent = nullptr);
    ~CalculateByteWidget();

    void enableControl(bool flag);

private:
    QByteArray calculatedByte;
    std::vector<QCheckBox*> checkBoxes;
    QPushButton* calculateByteButton;

signals:
    void byteCalculated(const QByteArray& byte);

};

#endif // CALCULATEBYTEWIDGET_H
