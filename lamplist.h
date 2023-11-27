#ifndef LAMPLIST_H
#define LAMPLIST_H

typedef unsigned short USHORT;
typedef unsigned int UINT;
typedef unsigned char UCHAR;

#include <cstdint>
#include <string>
#include <QFile>
#include <QDataStream>
#include <QList>

struct NodeParameter
{
    USHORT id;
    USHORT size;
};

struct Node
{
    UINT id;                // Уникальный идентификатор узла
    std::string text;       // Текстовая строка
    UINT status;            // Битовая маска статуса узла
    USHORT mode;            // Текущий режим
    UCHAR levelHost;        // Уровень мощности в хосте (в процентах)
    UCHAR levelNode;        // Уровень мощности (в процентах)
    USHORT voltage;         // Напряжение питания (в В/100)
    USHORT current;         // Ток потребления (в А/100)
    UINT energy;            // Потребленная энергия (в Ватт-часах)
    UINT worktime;          // Время работы узла (в часах)
};

class LampList
{
public:
    LampList();
    void init(int num, int level);

private:
    QList<Node> nodes;
    QByteArray deviceArray;
    QList<NodeParameter> parameterTypes;

private:
    bool writeNodesToFile(const QList<Node> &nodes);
    bool writeNodesToByteArray(const QList<Node> &nodes);
};

#endif // LAMPLIST_H
