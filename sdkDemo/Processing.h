/*!
* @file Processing.h
* @brief Data processing thread
* @details 1. Data stitching, 2. Image generation, bilinear interpolation
* @author liuhai
* @date 2019/11/27 14:54
*/
#pragma once

#include <QObject>
#include <QImage>
#include <iostream>
#include <fstream>
#include <time.h>
#include <QtMath>
#include <mutex>

#include "common/CircleBuffer.h"
#include "common/WhileThread.h"
#include "common/sonarCommon.h"
#include "LhForwardFrame.h"
#include "LhForwardImage.h"
#include "AvoidCollision.h"


using namespace LhForwardSDK;
class Processing :  public QObject, public WhileThread
{
	Q_OBJECT

public:
	/**
    * @brief Processing class construction function
    * @param [in] circleBuffer Loop buffer, retrieve data from here and concatenate it
	*/
    explicit Processing(CircleBuffer*& circleBuffer, QObject* parent = nullptr);
	~Processing();
    QString device_name;
    QString l_avoidCollisionMsgs;

signals:
    void sigImageOK(QImage img);    // Sonar image to be displayed
    void sigDeviceName();
    void sigPackageInfo(struct packageInfo inPackageInfo);

private:
    uint16_t PACK_SIZE = LhForwardSDK::LhForwardFrame::PACK_SIZE;
    void threadFunc();

    AvoidCollision m_avoidCollision;
	CircleBuffer*& m_circleBuffer;
	LhForwardFrame* m_lhForwardFrame;
	LhForwardImage* m_lhForwardImage;
    struct packageInfo curFrameInfo;
    uint8_t* m_frameDataBuf;
    double* m_intensity;    // intensity data
    uchar* m_panSectorMemory;
	QImage m_sectorImage;
};

