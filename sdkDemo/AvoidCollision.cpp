#include "AvoidCollision.h"

AvoidCollision::AvoidCollision(QObject *parent)
    : QObject{parent}
{

}

QString AvoidCollision::f_getTargetList(const uint8_t *inMsg)
{
    QString retTargetList;  // List of collision avoidance information
    retTargetList.clear();

    AvoidCollisionStruct l_avoidCollisionStruct;
    OneTargetStruct l_oneTargetStruct;
    memcpy(&l_avoidCollisionStruct, inMsg, sizeof(AvoidCollisionStruct));
    for(int i = sizeof(AvoidCollisionStruct); i < l_avoidCollisionStruct.TargetCount*sizeof(OneTargetStruct); i += sizeof(OneTargetStruct)){
        memcpy(&l_oneTargetStruct, &inMsg[i], sizeof(OneTargetStruct));
        retTargetList += QString::number(l_oneTargetStruct.X, 'f', 2) +QString(",")+
                         QString::number(l_oneTargetStruct.Y, 'f', 2) +QString(",")+
                         QString::number(l_oneTargetStruct.Intensity) +QString(";");
    }

    if( true == retTargetList.endsWith(";")){   // If the last character is a semicolon (;), remove it
        retTargetList = retTargetList.left(retTargetList.length()-1);
    }

//    qDebug() << "Collision avoidance target information:" << retTargetList;

    return retTargetList;
}
