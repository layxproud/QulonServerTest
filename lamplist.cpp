#include "lamplist.h"
#include <QDebug>

#define SWAP_HL_UINT(i) ((i&0xFF)<<24)|((i&0xFF00)<<8)|((i&0xFF0000)>>8)|((i&0xFF000000)>>24)
#define SWAP_HL_SHORT(i) ((i&0xFF)<<8)|((i&0xFF00)>>8)

LampList::LampList(QObject *parent)
    : QObject{parent}
{
}

void LampList::init(int num, int level, UCHAR status)
{
    nodes.clear();

    for (int i = 1; i <= num; ++i)
    {
        Node newNode;
        newNode.id = SWAP_HL_UINT(i);
        newNode.status = status;
        newNode.mode = SWAP_HL_SHORT(0);
        newNode.levelHost = level;
        newNode.levelNode = 0;
        newNode.voltage = SWAP_HL_SHORT(220);
        newNode.current = SWAP_HL_SHORT(200);
        newNode.energy = SWAP_HL_UINT(100);
        newNode.worktime = SWAP_HL_UINT(24 * 3600);
        nodes.append(newNode);
    }



    updateNodes();
}

QByteArray LampList::getFile()
{
    return deviceArray;
}

Node* LampList::getNodeById(UINT id)
{
    UINT swappedID = SWAP_HL_UINT(id);
    for (Node &node : nodes)
    {
        if (node.id == swappedID)
            return &node;
    }
    return nullptr;
}

void LampList::updateNodes()
{
    if (!writeNodesToFile(nodes))
    {
        qDebug() << "Error writing nodes to file.";
        return;
    }

    prevNodes = nodes;

    emit nodesUpdated();
}

bool LampList::isNodesListEmpty() const
{
    if (nodes.isEmpty())
        return true;
    else
        return false;
}

int LampList::getNodesListSize() const
{
    return nodes.size();
}

void LampList::restoreInitialState()
{
    nodes = prevNodes;
}

bool LampList::writeNodesToFile(const QList<Node> &nodes)
{
    deviceArray.clear();

    // Запись заголовка файла состояния
    deviceArray.append("STATE2.DAT\0\0\0\0", 16);
    // Количество узлов
    UINT swappedNodesSize = SWAP_HL_UINT(nodes.size());
    deviceArray.append(reinterpret_cast<const char*>(&swappedNodesSize), sizeof(UINT));
    // Количество параметров
    UINT swappedParamSize = SWAP_HL_UINT(parameterTypes.size());
    deviceArray.append(reinterpret_cast<const char*>(&swappedParamSize), sizeof(UINT));
    // Длина блока параметров для одного узла
    UINT swappedNodeSize = SWAP_HL_UINT(24);
    deviceArray.append(reinterpret_cast<const char*>(&swappedNodeSize), sizeof(UINT));
    // Смещение таблицы параметров узлов
    UINT swappedNodesOffset = SWAP_HL_UINT(0x00000020 + parameterTypes.size() * 8);
    deviceArray.append(reinterpret_cast<const char*>(&swappedNodesOffset), sizeof(UINT));

    USHORT prevOffset = 0;

    // Запись таблицы параметров узлов
    for (const NodeParameter &parameter : parameterTypes)
    {
        // Смещение параметра i в блоке параметров узла
        USHORT swappedParamOffset = SWAP_HL_SHORT(prevOffset);
        deviceArray.append(reinterpret_cast<const char*>(&swappedParamOffset), sizeof(USHORT));
        // Размер параметра i в байтах
        USHORT swappedParamSize = SWAP_HL_SHORT(parameter.size);
        deviceArray.append(reinterpret_cast<const char*>(&swappedParamSize), sizeof(USHORT));
        // Тип параметра i
        USHORT swappedParamType = SWAP_HL_SHORT(parameter.id);
        deviceArray.append(reinterpret_cast<const char*>(&swappedParamType), sizeof(USHORT));
        // Зарезервировано
        USHORT swappedParamReserved = SWAP_HL_SHORT(0);
        deviceArray.append(reinterpret_cast<const char*>(&swappedParamReserved), sizeof(USHORT));

        prevOffset += parameter.size;
    }

    // Запись блока параметров для каждого узла
    for (const Node &node : nodes)
    {
        QByteArray nodeData;
        // Идентификатор
        nodeData.append(reinterpret_cast<const char*>(&node.id), sizeof(UINT));
        // Битовая маска
        nodeData.append(reinterpret_cast<const char*>(&node.status), sizeof(UCHAR));
        // Текущий режим
        nodeData.append(reinterpret_cast<const char*>(&node.mode), sizeof(USHORT));
        // Уровень мощности в хосте
        nodeData.append(reinterpret_cast<const char*>(&node.levelHost), sizeof(UCHAR));
        // Уровень мощности
        nodeData.append(reinterpret_cast<const char*>(&node.levelNode), sizeof(UCHAR));
        // Напряжение питания
        nodeData.append(reinterpret_cast<const char*>(&node.voltage), sizeof(USHORT));
        // Ток потребления
        nodeData.append(reinterpret_cast<const char*>(&node.current), sizeof(USHORT));
        // Потребленная энергия
        nodeData.append(reinterpret_cast<const char*>(&node.energy), sizeof(UINT));
        // Время работы узла
        nodeData.append(reinterpret_cast<const char*>(&node.worktime), sizeof(UINT));

        // Добавление данных узла в общий QByteArray
        deviceArray.append(nodeData);
    }

    // Сохранение QByteArray в файл
    QFile file("output.dat");
    if (file.open(QIODevice::WriteOnly))
    {
        file.write(deviceArray);
        file.close();
        return true;
    }
    else return false;
}

bool LampList::writeNodesToByteArray(const QList<Node> &nodes)
{
    deviceArray.clear();

    // Запись заголовка файла состояния
    deviceArray.append("STATE2.DAT\0\0\0\0", 16);
    // Количество узлов
    UINT swappedNodesSize = SWAP_HL_UINT(nodes.size());
    deviceArray.append(reinterpret_cast<const char*>(&swappedNodesSize), sizeof(UINT));
    // Количество параметров
    UINT swappedParamSize = SWAP_HL_UINT(parameterTypes.size());
    deviceArray.append(reinterpret_cast<const char*>(&swappedParamSize), sizeof(UINT));
    // Длина блока параметров для одного узла
    UINT swappedNodeSize = SWAP_HL_UINT(24);
    deviceArray.append(reinterpret_cast<const char*>(&swappedNodeSize), sizeof(UINT));
    // Смещение таблицы параметров узлов
    UINT swappedNodesOffset = SWAP_HL_UINT(0x00000020 + parameterTypes.size() * 8);
    deviceArray.append(reinterpret_cast<const char*>(&swappedNodesOffset), sizeof(UINT));

    USHORT prevOffset = 0;

    // Запись таблицы параметров узлов
    for (const NodeParameter &parameter : parameterTypes)
    {
        // Смещение параметра i в блоке параметров узла
        USHORT swappedParamOffset = SWAP_HL_SHORT(prevOffset);
        deviceArray.append(reinterpret_cast<const char*>(&swappedParamOffset), sizeof(USHORT));
        // Размер параметра i в байтах
        USHORT swappedParamSize = SWAP_HL_SHORT(parameter.size);
        deviceArray.append(reinterpret_cast<const char*>(&swappedParamSize), sizeof(USHORT));
        // Тип параметра i
        USHORT swappedParamType = SWAP_HL_SHORT(parameter.id);
        deviceArray.append(reinterpret_cast<const char*>(&swappedParamType), sizeof(USHORT));
        // Зарезервировано
        USHORT swappedParamReserved = SWAP_HL_SHORT(0);
        deviceArray.append(reinterpret_cast<const char*>(&swappedParamReserved), sizeof(USHORT));

        prevOffset += parameter.size;
    }

    // Запись блока параметров для каждого узла
    for (const Node &node : nodes)
    {
        QByteArray nodeData;
        // Идентификатор
        nodeData.append(reinterpret_cast<const char*>(&node.id), sizeof(UINT));
        // Битовая маска
        nodeData.append(reinterpret_cast<const char*>(&node.status), sizeof(UCHAR));
        // Текущий режим
        nodeData.append(reinterpret_cast<const char*>(&node.mode), sizeof(USHORT));
        // Уровень мощности в хосте
        nodeData.append(reinterpret_cast<const char*>(&node.levelHost), sizeof(UCHAR));
        // Уровень мощности
        nodeData.append(reinterpret_cast<const char*>(&node.levelNode), sizeof(UCHAR));
        // Напряжение питания
        nodeData.append(reinterpret_cast<const char*>(&node.voltage), sizeof(USHORT));
        // Ток потребления
        nodeData.append(reinterpret_cast<const char*>(&node.current), sizeof(USHORT));
        // Потребленная энергия
        nodeData.append(reinterpret_cast<const char*>(&node.energy), sizeof(UINT));
        // Время работы узла
        nodeData.append(reinterpret_cast<const char*>(&node.worktime), sizeof(UINT));

        // Добавление данных узла в общий QByteArray
        deviceArray.append(nodeData);
    }
    return true;
}
