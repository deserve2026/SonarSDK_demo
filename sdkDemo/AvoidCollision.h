#ifndef AVOIDCOLLISION_H
#define AVOIDCOLLISION_H

#include <QObject>
#include <QStringList>
#include <QDebug>

#pragma pack(1)
struct AvoidCollisionStruct{    // The structure of collision avoidance information
    uint8_t Type;               // Data type
    uint16_t TargetCount;       // The number of collision avoidance targets
};
struct OneTargetStruct{         // A collision avoidance message
    float X;                    // The x-position of the target
    float Y;                    // The y-position of the target
    uint8_t Intensity;          // Intensity of the target
};

#pragma pack()


class AvoidCollision : public QObject
{
    Q_OBJECT
public:
    explicit AvoidCollision(QObject *parent = nullptr);

    QString f_getTargetList(const uint8_t *inMsg);
signals:

};

#endif // AVOIDCOLLISION_H
