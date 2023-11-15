#include "calculatebytewidget.h"

#include <QLabel>
#include <QLayout>
#include <QDebug>

CalculateByteWidget::CalculateByteWidget(int numBits, QWidget *parent)
    : QWidget{parent}
{
    int numBytes = (numBits + 7) / 8;
    calculatedByte.resize(numBytes);
    calculatedByte.fill(0);

    QGridLayout* layout = new QGridLayout(this);

    for (int i = 0; i < numBits; ++i) {
        QLabel* label = new QLabel(QString::number(i), this);
        QCheckBox* checkBox = new QCheckBox(this);
        checkBoxes.push_back(checkBox);

        layout->addWidget(label, 0, numBits - i - 1);
        layout->addWidget(checkBox, 1, numBits - i - 1);

        connect(checkBox, &QCheckBox::stateChanged, this, [this, i](int state){
            if (state == Qt::Checked) {
                calculatedByte[i / 8] |= (1 << (i % 8));
            } else {
                calculatedByte[i / 8] &= ~(1 << (i % 8));
            }
        });
    }

    // Create QPushButton
    calculateByteButton = new QPushButton("Calculate", this);
    layout->addWidget(calculateByteButton, 1, numBits);

    connect(calculateByteButton, &QPushButton::clicked, [=]() {
        qDebug() << "calculatedByte:" << calculatedByte;
        emit byteCalculated(calculatedByte);
    });

    enableControl(false);
}

CalculateByteWidget::~CalculateByteWidget()
{

}

void CalculateByteWidget::enableControl(bool flag)
{
    for (auto checkBox : checkBoxes)
        checkBox->setEnabled(flag);

    calculateByteButton->setEnabled(flag);
}
