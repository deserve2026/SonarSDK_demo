#ifndef DEVICEINFO_H
#define DEVICEINFO_H

#include <QObject>
#include <QDebug>
#include <QStringList>

#pragma pack(1)
struct deviceDetailData
{
    float frequency;
    uint8_t winFunc[8] = {0};
    uint8_t tvg[8] = {0};
    uint8_t sigMode[8] = {0};
    float minDistReso;
    float maxDistReso;
    short horBeam;
    float minHorAngleReso;
    float maxHorAngleReso;
    short verBeam;
    float minVerAngleReso;
    float maxVerAngleReso;
    float maxRange;
    float horAngle;
    uint8_t reserve[32] = { 0 };
};

struct deviceTotalInfo
{
    uint8_t flag;
    char deviceName[15];
    uint64_t chipId;
    uint32_t licStatus;
    uint8_t workModeNum;
    uint8_t brightnessBase;
    uint8_t modeFlag[8];
    uint8_t reverse[90];
    deviceDetailData dataDetail[8];
};
#pragma pack()
#endif // DEVICEINFO_H
