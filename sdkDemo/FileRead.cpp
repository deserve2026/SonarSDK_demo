#include "FileRead.h"

FileRead::FileRead(CircleBuffer*& circleBuffer, QObject* parent)
    : QObject(parent)
    , m_circleBuffer(circleBuffer)
    , m_curFrameData(new char[PACK_SIZE * PACK_MAX_NUM])
{

}

FileRead::~FileRead()
{
    delete[] m_curFrameData;
}

bool FileRead::f_readFile(QString inFileName)
{
    bool ret = false;

    setlocale(LC_ALL, ".UTF-8");

    m_ifs.open(inFileName.toStdString(), ifstream::binary | ifstream::in );
    ret = m_ifs.is_open();

    return ret;
}

void FileRead::threadFunc()
{
    char *data = new char[PACK_SIZE];
    memset(data, 0, PACK_SIZE);

    // Pre-read to obtain all frame positions
    packageInfo* head;
    packageInfo curHead;
    memset(&curHead, 0, sizeof(packageInfo));
    while (!m_ifs.eof()) {
        m_ifs.read(data, PACK_SIZE);
        head = (packageInfo*)data;
        if (head->frameSerialNum != curHead.frameSerialNum) {
            // A new frame, recording position
            m_framePos.push_back((unsigned int)m_ifs.tellg() - PACK_SIZE);
            memcpy(&curHead, head, sizeof(packageInfo));
        }
    }
    m_ifs.clear();
    m_ifs.seekg(0, ios::beg);
    m_curFramePosInd = 0;

    while (m_runFlag)
    {
        if (m_startFlag) {
            // ==================== 斩断无限循环的外科手术 ====================
            if (m_curFramePosInd >= m_framePos.size()) {
                // m_curFramePosInd = 0;  // 把原厂这行流氓代码直接删掉或注释掉！

                m_startFlag = false;      // 强行将播放状态改为“停止”，拉起手刹！

                // 在输出窗口疯狂报警，提醒你拔管
                // 在输出窗口疯狂报警，提醒你拔管 (纯英文防乱码)
                qDebug() << "\n=======================================================";
                qDebug() << ">>> WARNING: DB File Playback Completed!";
                qDebug() << ">>> Data stream stopped. NO MORE duplicate images will be saved.";
                qDebug() << ">>> Please close the software NOW and load the next file.";
                qDebug() << "=======================================================\n";

                continue; // 忽略下面的动作，直接进入静默挂机状态
            }
            // ================================================================
            getOneFrameIntoBuf();
            m_curFramePosInd++;
            std::this_thread::sleep_for(std::chrono::milliseconds(int(100)));
        }
    }

    if(m_ifs.is_open()){
        m_ifs.close();
    }
}

void FileRead::getOneFrameIntoBuf()
{
    unsigned int startPos = m_framePos.at(m_curFramePosInd);
    unsigned int stopPos;
    if (m_curFramePosInd == m_framePos.size() - 1) {
        m_ifs.seekg(0, ios::end);
        stopPos = m_ifs.tellg();
    }
    else {
        stopPos = m_framePos.at(m_curFramePosInd + 1);
    }
    int frameSize = stopPos - startPos;
    // Read a file
    memset(m_curFrameData, 0, PACK_SIZE * PACK_MAX_NUM);
    m_ifs.seekg(startPos, ios::beg);
    m_ifs.read(m_curFrameData, frameSize);
    // write to buffer
    for (int i = 0; i < frameSize / PACK_SIZE; i++) {
        int off = i * PACK_SIZE;
        m_circleBuffer->writeData(&m_curFrameData[off], PACK_SIZE);
    }
}



