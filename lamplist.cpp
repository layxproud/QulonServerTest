#include "lamplist.h"
#include <QDebug>

LampList::LampList()
{

}

void LampList::init()
{
    QList<Node> nodes;
    Node testNode;
    testNode.id = 0x1234;
    testNode.text = "Sample Node";
    testNode.status = 0b01010101;
    testNode.mode = 0xABCD;
    testNode.levelHost = 50;
    testNode.levelNode = 75;
    testNode.voltage = 220;
    testNode.current = 200;
    testNode.energy = 100;
    testNode.worktime = 24 * 3600;
    nodes.append(testNode);

    if (writeNodesToFile("nodes.dat", nodes))
        qDebug() << "Nodes written to file successfully.";
    else
        qDebug() << "Error writing nodes to file.";
}

bool LampList::writeNodesToFile(const QString &fileName, const QList<Node> &nodes)
{
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly))
    {
        return false;
    }

    QDataStream out(&file);
    out.setByteOrder(QDataStream::LittleEndian);

    // Запись заголовка файла состояния
    out.writeRawData("STATE2.DAT\0\0\0\0", 16);
    out << static_cast<quint32>(nodes.size()); // Количество узлов
    out << static_cast<quint32>(10); // Количество параметров
    out << static_cast<quint32>(sizeof(Node)); // Длина блока параметров для одного узла
    out << static_cast<quint32>(0x00000020 + 10 * 8); // Смещение таблицы параметров узлов

    // Запись таблицы параметров узлов
    QList<quint16> parameterTypes = {
        0xFF00, 0xFF01, 0xFF02, 0xFF03, 0xFF10, 0xFF11, 0xFF12, 0xFF13, 0xFF14, 0xFF15
    };

    for (quint16 type : parameterTypes)
    {
        out << quint16(0); // Смещение параметра
        out << static_cast<quint16>(sizeof(quint16)); // Размер параметра
        out << type; // Тип параметра
        out << quint16(0); // Зарезервировано
    }

    // Запись блока параметров для каждого узла
    for (const Node& node : nodes)
    {
        QByteArray nodeData;
        QDataStream nodeStream(&nodeData, QIODevice::WriteOnly);
        nodeStream.setByteOrder(QDataStream::LittleEndian);

        // Сериализация данных узла в QByteArray
        nodeStream << node.id << node.text << node.status << node.mode
                   << node.levelHost << node.levelNode << node.voltage
                   << node.current << node.energy << node.worktime;

        // Запись данных узла в файл
        out.writeRawData(nodeData.constData(), nodeData.size());
    }

    qDebug () << &out;
    file.close();
    return true;
}
