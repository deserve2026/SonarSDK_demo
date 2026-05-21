/*!
* @file FileRead.h
* @brief Simple data reading
* @details
* @author liuhai
* @date 2019/11/27 14:40
*/
#pragma once

#ifdef USE_WINDOWS
#define WIN32_LEAN_AND_MEAN
#include "Windows.h"
#elif USE_LINUX
#include "unistd.h"
#endif

#include <QObject>
#include <fstream>
#include <iostream>
#include <chrono>
#include <QThread>
#include <QMessageBox>
#include <QImage>
#include <vector>
#include "common/CircleBuffer.h"
#include "LhForwardFrame.h"
#include "common/WhileThread.h"
#include <QDebug>

using namespace LhForwardSDK;
using namespace std;

class FileRead : public QObject , public WhileThread
{
    Q_OBJECT

public:
    explicit FileRead(CircleBuffer*& circleBuffer, QObject* parent = nullptr);
    ~FileRead();

    bool m_startFlag;
    QString fileName;

    bool f_readFile(QString inFileName);
    void setGotoFrame(int inFrame);

signals:
    void sigImageOK(QImage img);
private:
    uint16_t PACK_SIZE = LhForwardSDK::LhForwardFrame::PACK_SIZE;
    void threadFunc();
    void init();

    CircleBuffer*& m_circleBuffer;
    std::ifstream m_ifs;

    vector<unsigned int> m_framePos;
    int m_curFramePosInd;
    char* m_curFrameData;
    void getOneFrameIntoBuf();
};
