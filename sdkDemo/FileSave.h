#pragma once

#include <QObject>
#include <fstream>
#include <iostream>
#include <chrono>
#include <QPixmap>
#include <queue>
#include <mutex>
#include <QImage>
#include <QDebug>
#include <QMutex>

#include "common/sonarCommon.h"
#include "common/CircleBuffer.h"
#include "common/WhileThread.h"
#include "LhForwardFrame.h"


using namespace std;
using namespace LhForwardSDK;
// using LhForwardSDK::PACK_SIZE;

class FileSave :public QObject, public WhileThread
{
    Q_OBJECT
public:
	/**
    * @brief Processing class constructor function
    * @param [in] circleBuffer Circular buffering, obtain data from here and concatenate it
	*/
    explicit FileSave(CircleBuffer*& circleBuffer, QObject* parent = nullptr);
	~FileSave();

	/**
    * @brief Save file function
    */
    bool f_saveFile(QString fileName);

private:
    uint16_t PACK_SIZE = LhForwardSDK::LhForwardFrame::PACK_SIZE;
	void threadFunc();
    void init();
	CircleBuffer*& m_circleBuffer;

	std::string m_fileName;
	int m_fileNum;
	ofstream m_ofs;
	packageInfo m_curPackInfo;
	unsigned int m_fileSize;
	char* m_curFileData;
	int m_curFilePackNum;
};


