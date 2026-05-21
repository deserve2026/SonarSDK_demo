#include "Processing.h"
#include <QDebug>

Processing::Processing(CircleBuffer*& circleBuffer, QObject* parent)
    : QObject(parent)
    , m_circleBuffer(circleBuffer)
    , m_lhForwardFrame(new LhForwardFrame)
    , m_lhForwardImage(new LhForwardImage(IMAGE_WIDTH, IMAGE_HEIGHT))
    , m_frameDataBuf(new uint8_t[PACK_SIZE * PACK_MAX_NUM])
    , m_intensity(new double[PACK_MAX_NUM * PACK_SIZE])
    , m_panSectorMemory(new uchar[(uint64_t)PACK_MAX_NUM * PACK_SIZE])
{
}

Processing::~Processing()
{
    delete m_lhForwardFrame;
    delete m_lhForwardImage;
    delete[] m_frameDataBuf;
    delete[] m_intensity;
    delete[] m_panSectorMemory;
}

void Processing::threadFunc()
{
    int count = 0;
    uint8_t *data = new uint8_t[PACK_SIZE];

    // Get the image
    QImage tempImg;

    while (m_runFlag) {
        if (m_circleBuffer->isAnyData()) {
            memset(data, 0, PACK_SIZE);
            // There is unextracted data in the first level buffer, which is passed into the second level buffer
            memcpy(data, m_circleBuffer->readDataAddr(), PACK_SIZE);
            if (m_lhForwardFrame->writeOnePackData(data) == LhForwardFrame::oneFrameOK) {
                // One frame of data has been received
                struct packageInfo curFrameInfo;
                m_lhForwardFrame->getOneFrame(&curFrameInfo, m_frameDataBuf); // Obtain data information
                // qDebug() << curFrameInfo.frameSerialNum;
                char l_devName[16] = {0};
                memcpy(l_devName, curFrameInfo.deviceName, 15);
                device_name = l_devName;
                emit sigDeviceName();
                emit sigPackageInfo(curFrameInfo);

                if (curFrameInfo.packType == 0)    // Imaging data (Beam)
                {
                    // Currently, images are scaled according to height
                    double imageRes = 0;

                    int panSize = IMAGE_WIDTH > IMAGE_HEIGHT ? IMAGE_WIDTH : IMAGE_HEIGHT;
                    int lwidth = IMAGE_WIDTH;
                    int lheight = IMAGE_HEIGHT;
                    if (360 == curFrameInfo.horAngleReso) {
                        lwidth = panSize;
                        lheight = panSize;
                    }
                    memset(m_panSectorMemory, 0, (size_t)(panSize * panSize));

                    m_lhForwardImage->generateForwardImage(curFrameInfo,m_frameDataBuf, m_panSectorMemory, imageRes);

                    tempImg = QImage(m_panSectorMemory, lwidth, lheight, QImage::Format_Grayscale8);

                    emit sigImageOK(tempImg);
                }

                else if(curFrameInfo.packType == 2) // Uncompressed image data (Uncompressed)
                {
                    m_lhForwardImage->preprocessData(curFrameInfo, m_frameDataBuf, m_intensity);

                    int coef = 256;
                    if (curFrameInfo.dataWidth == 0x00)
                        coef = 1;

                    for (size_t i = 0; i < curFrameInfo.numPerRow * curFrameInfo.dataHeight; i++)
                    {
                        m_frameDataBuf[i] = m_intensity[i] / coef;
                    }

                    tempImg = QImage(m_frameDataBuf, curFrameInfo.numPerRow, curFrameInfo.dataHeight, QImage::Format_Grayscale8);

                    emit sigImageOK(tempImg);
                }
                else if (curFrameInfo.packType == 3)    // Compressed image data (PNG)
                {
                    if(tempImg.loadFromData(m_frameDataBuf, curFrameInfo.dataHeight, "png")){

                        // collision avoidance
                        // Because each packet has a header of 256, even if 10 floats are not passed, it will not cause memory overflow
                        uint32_t l_pngDataSize = curFrameInfo.dataHeight;   // Later, the protocol for PNG was modified to store the size of the entire PNG image
                        l_avoidCollisionMsgs = m_avoidCollision.f_getTargetList(&m_frameDataBuf[l_pngDataSize]);

                        emit sigImageOK(tempImg);
                    }

                }
                else if (curFrameInfo.packType == 4)    // Compressed image data (JPG)
                {
                    if(tempImg.loadFromData(m_frameDataBuf, curFrameInfo.dataHeight, "jpg")){

                        uint32_t l_pngDataSize = curFrameInfo.dataHeight;
                        l_avoidCollisionMsgs = m_avoidCollision.f_getTargetList(&m_frameDataBuf[l_pngDataSize]);

                        emit sigImageOK(tempImg);
                    }

                }
            }


        }
        else {
            count++;
            if (count >= 10) {
                count = 0;
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
        }

    }
}

