/*!
* Copyright (C) Zhuhai Lanheng Technology Co., Ltd.
* All rights reserved
* @File LhForwardFrame.h
* @Author liuhai
* @Brief Forward-looking sonar data parsing library header file
* @Details Data parsing and splicing
* @LastEditors liuhai
* @LastEditTime 2020/4/17 10:17
* @Version 3.3
*/

#pragma once
#include <cstring>
#include <stdint.h>
#include <map>
#include <string>

#ifdef USE_WINDOWS
#ifdef DLL_EXPORTS
#define DLL_API _declspec(dllexport)
#else
#define DLL_API _declspec(dllimport)
#endif
#else
#define DLL_API
#endif // !DLL_API

namespace LhForwardSDK
{
const uint16_t PACK_INFO_SIZE = 256;  //Packet header information length
const uint16_t PACK_DATA_INDEX = 256; //Data index per packet
//const uint16_t PACK_SIZE = 5800;      //Total bytes per packet
const uint16_t PACK_MAX_NUM = 12000;  //Maximum packets per frame

#pragma pack(1)
struct DLL_API packageInfo
{
    uint8_t packType = 0;           //Packet type
    uint8_t dataWidth = 0;          //Data bit width
    uint32_t packTime1 = 0;         //Timestamp 1 (us)
    uint16_t packTime2 = 0;         //Timestamp 2 (us)
    uint16_t numPerRow = 0;         //Points per row
    uint16_t numPerCol = 0;         //Points per column
    uint16_t rowNumPerPack = 0;     //Rows per packet
    uint16_t rowSerialNum = 0;      //Row serial number
    uint16_t packSerialNum = 0;     //Packet serial number
    uint16_t packNumPerFrame = 0;   //Packets per frame
    uint32_t frameSerialNum = 0;    //Frame serial number
    uint8_t reserve1[8] = {0};      //Reserved
    char deviceName[15];            //Device name
    uint8_t workMode = 0;           //Work mode
    float acousticVelocity = 0;     //Sound velocity (m/s)
    uint32_t dataHeight = 0;        //Points in range direction
    float maxDist = 0;              //Range (m)
    float distReso = 0;             //Range resolution (m)
    float horAngleReso = 0;         //Horizontal angle resolution / Horizontal opening angle (degrees)
    float verAngleReso = 0;         //Vertical angle resolution
    float longitude = 0;            //Longitude
    float latitude = 0;             //Latitude
    float altitude = 0;             //Altitude
    float status = 0;               //Status
    float yaw = 0;                  //Yaw angle
    float pitch = 0;                //Pitch angle
    float roll = 0;                 //Roll angle
    float speed = 0;                //Speed
    float direction = 0;            //Heading
    uint8_t syncModel = 0;          //Synchronization mode
    float workCycle = 0;            //Work cycle (s)
    uint8_t power = 0;              //Transmit power
    uint8_t sigModel = 0;           //Signal mode
    uint8_t TVGSet = 0;             //TVG setting
    uint32_t syncDelay = 0;         //Synchronization delay (us)
    uint8_t windowFunc = 0;         //Window function
    float TVGA = 0;                 //TVG advanced setting A
    float TVGB = 0;                 //TVG advanced setting B
    float TVGC = 0;                 //TVG advanced setting C
    uint8_t brightness = 0;         //Brightness
    uint8_t dynImprove = 0;         //Dynamic improvement
    uint8_t aesaMode = 0;           //Aesa mode
    float press = 0;                //Pressure
    float temp1 = 0;                //External temperature
    float temp2 = 0;                //Internal temperature
    uint64_t utcTime = 0;           //UTC time
    uint8_t reservel[10] = {0};     //Reserved
    uint8_t versionMajor = 0;       //Protocol major version number
    uint8_t versionMinor = 0;       //Protocol minor version number
    float gamma = 0;                //Gamma coefficient
    uint8_t reserve2[4] = {0};      //Reserved
    float doaFreq = 0;              //DOA maximum frequency
    float doaFregValue = 0;         //DOA frequency peak-to-average ratio
    float doa = 0;                  //DOA angle result
    float doaValue = 0;             //DOA angle peak-to-average ratio
    float trx25DDirectionAngle;     //Trx Direction
    uint8_t rsv[60] = {0};          //Reserved
};
#pragma pack()

class DLL_API LhForwardFrame
{
public:
    enum LfFrameRet
    {
        writePackOK = 0,        //Successfully wrote one packet of data
        writePackErr = -1,      //Write error, packet header information incorrect
        oneFrameOK = 1,         //One frame of data writing completed
    };

    static uint16_t PACK_SIZE;

    LhForwardFrame(uint16_t packSize = 5800);
    ~LhForwardFrame();

    std::string getVersion();

    /**
        * @brief Write to frame buffer
        * @param [in] data Data to be written
        * @return Returns write status, enum LfFameRet
        */
    int writeOnePackData(const uint8_t* data);

    /**
        * @brief Get one frame of data
        * @param [out] packInfo Packet header information
        * @param [out] dataBuf Data information, requires allocating sufficient memory
        * @note Call immediately after writeOnePackData returns oneFrameOK to retrieve the data
        */
    void getOneFrame(struct packageInfo* packInfo, uint8_t* dataBuf);

    /**
        * @brief Initialize
        * @note
        */
    void init();

private:
    void swapPingPang();

private:
    struct packageInfo m_packInfoPing;
    struct packageInfo m_packInfoPang;
    struct packageInfo* m_curPackInfo;
    uint8_t* m_frameBufPing;
    uint8_t* m_frameBufPang;
    uint8_t* m_curFrameBuf;
    bool m_pingWritingFlag;
    std::map<int, int> m_dataWidthSize; // Data size corresponding to different bit widths
};
}
