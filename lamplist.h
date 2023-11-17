#ifndef LAMPLIST_H
#define LAMPLIST_H

#include <QFile>
#include <QDataStream>
#include <QList>

struct Node
{
    quint32 id;      // Уникальный идентификатор узла
    QString text;    // Текстовая строка
    quint32 status;  // Битовая маска статуса узла
    quint16 mode;    // Текущий режим
    quint8 levelHost; // Уровень мощности в хосте (в процентах)
    quint8 levelNode; // Уровень мощности (в процентах)
    quint16 voltage;  // Напряжение питания (в В/100)
    quint16 current;  // Ток потребления (в А/100)
    quint32 energy;   // Потребленная энергия (в Ватт-часах)
    quint32 worktime; // Время работы узла (в часах)
};

class LampList
{
public:
    LampList();
    void init();

private:
    bool writeNodesToFile(const QString& fileName, const QList<Node>& nodes);
};

#endif // LAMPLIST_H
