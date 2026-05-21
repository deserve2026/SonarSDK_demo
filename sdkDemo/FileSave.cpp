#include "FileSave.h"

#define MAX_FILE_SIZE (524288000)   // 500M

FileSave::FileSave(CircleBuffer*& circleBuffer, QObject *parent)
    :QObject(parent)
    , m_circleBuffer(circleBuffer)
    , m_curFileData(new char[PACK_SIZE * 5000])     // A frame of data can contain up to 5000 UDP packets
{
    memset(&m_curPackInfo, 0, sizeof(packageInfo));
}

FileSave::~FileSave()
{
    delete[] m_curFileData;
}

bool FileSave::f_saveFile(QString fileName)
{
    bool ret = false;
    setlocale(LC_ALL, ".UTF-8");
    m_fileName = fileName.toStdString();
    m_fileSize = 0;
    m_ofs.open(m_fileName, ios::binary | ios::out);
    ret = m_ofs.is_open();
    if(false == ret){
        qDebug() << "Failed to save the file";
    }
    else{
        qDebug() << "Save file";
    }
    return ret;
}

void FileSave::threadFunc()
{
    int count = 0;
    m_fileNum = 0;
    m_fileSize = 0;
    m_curFilePackNum = 0;

    memset(&m_curPackInfo, 0, sizeof(packageInfo));
    m_circleBuffer->setRead2Pos();
    char *data = new char[PACK_SIZE];
    while(m_runFlag) {
        if (m_circleBuffer->isAnyData2()) {
            memcpy(data, m_circleBuffer->readDataAddr2(), PACK_SIZE);
            packageInfo* head = (packageInfo*)data;
            if (head->frameSerialNum != m_curPackInfo.frameSerialNum ) {
                // A new frame of data has arrived, save it
                m_ofs.write(m_curFileData, PACK_SIZE * m_curFilePackNum);
                // Update the size of the written file
                m_fileSize += PACK_SIZE * m_curFilePackNum;
                if (m_fileSize >= MAX_FILE_SIZE) {
                    // The file exceeds the maximum size. Create a new file
                    m_ofs.close();
                    m_fileNum++;
                    if (m_fileNum == 1) {
                        m_fileName = m_fileName.substr(0, m_fileName.length() - 3) + "_1.DB";
                    }
                    else {
                        size_t p = m_fileName.find_last_of('_');
                        m_fileName = m_fileName.substr(0, p + 1) + ::to_string(m_fileNum) + ".DB";
                    }
                    m_fileSize = 0;
                    setlocale(LC_ALL, ".UTF-8");
                    m_ofs.open(m_fileName, ios::binary | ios::out);
                    if(!m_ofs.is_open()){
                        qDebug() << "Failed to save the file";
                    }
                }
                // Reset buffer data size to zero
                m_curFilePackNum = 0;
                memcpy(&m_curPackInfo, head, sizeof(packageInfo));
            }
            // New data is written into the cache
            memcpy(&m_curFileData[m_curFilePackNum * PACK_SIZE], data, PACK_SIZE);
            m_curFilePackNum++;
        }

        count++;
        if (count >= 10) {
            count = 0;
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }

    delete [] data;
    m_ofs.close();
}



