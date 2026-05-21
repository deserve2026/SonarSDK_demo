/*!
* Copyright (C) Zhuhai Lanheng Technology Co., Ltd.
* All rights reserved
* @File LhForwardCMD.h
* @Author liuhai
* @Brief Forward-looking sonar data parsing library header file
* @Details Data parsing and splicing
* @LastEditors liuhai
* @LastEditTime 2020/4/13 11:16
* @Version 3.3
*/


#pragma once

#ifdef USE_WINDOWS
#define DLL_API _declspec(dllexport)
#else
#define DLL_API
#endif // !DLL_API

#ifdef USE_WINDOWS
#include <WinSock2.h>
#elif USE_LINUX
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#define SOCKADDR_IN sockaddr_in
#define SOCKADDR sockaddr
#define closesocket close
#endif
#include <string>
#include <cstdint>
using std::string;

namespace LhForwardSDK
{
#ifdef USE_WINDOWS
typedef SOCKET TYPE_SOCKET;
#elif USE_LINUX
typedef int TYPE_SOCKET;
#define INVALID_SOCKET -1
#endif

#define VERSION_BUFFER 4096         // Define the buffer size for receiving version number

class DLL_API LhForwardCMD
{
public:
    enum LfCMDRet
    {
        ok = 0,
        connectErr = 1,		//Device connection error, only returned by connectDevice function, indicates connection failure, please check network connection
        noConnect = 2,		//Device not connected error, returned by sendCMD function, indicates command sent without device connection
        cmdArgErr = 3,		//Command argument error, returned by sendCMD function, indicates argument mismatch for the command. Note: Only some argument errors can be detected.
        sendErr = 4,		//Command send error, returned by sendCMD function, indicates possible device connection issue, try sending again after reconnecting
        receiveErr = 5,		//Command reply error, returned by sendCMD function, indicates the sonar device received the sent command but the command was incorrect, possibly due to argument error
        noReceive = 6,		//No command reply, returned by sendCMD function, indicates possible connection issue, please reconnect and try sending again
    };

    enum LfCMDInfo
    {
        udpData = 0x00,             //UDP data receiver information configuration
        sync = 0x01,                //Synchronization
        perid = 0x02,               //Period
        workMode = 0x05,            //Work mode
        startStop = 0x06,           //Start Stop
        maxDist = 0x09,             //Maximum distance
//        doa = 0x0B,                 //DOA mode
//        doaFc = 0x0C,               //DOA mode fc setting
        power = 0x10,               //Transmit power
        signal = 0x11,              //Signal mode
//        tvg = 0x12,                 //Built-in TVG
//        delay = 0x13,               //Synchronization delay
        acousticVelocity = 0x14,    //Sound velocity
        dataSource = 0x15,          //Upload data source
        winfunc = 0x16,             //Built-in window function
        tvgAdv = 0x17,              //Advanced TVG settings
        brightness = 0x18,          //Brightness
//        compassCalibration = 0x19,	//Compass calibration
//        distRes = 0x1A,             //Distance resolution
//        angleRes = 0x1B,            //Angle resolution
        timeService = 0x1C,         //Downward time service
//        dynImprove = 0x1D,          //Dynamic improvement
        pressZero = 0x1E,           //Pressure zeroing
        udpSend = 0x1F,             //UDP send control
        saveConfigInfo = 0x20,      //Save current configuration
//        restoreFacSet = 0x21,       //Restore factory settings
//        workTimeSet = 0x23,         //Auto stop time after startup
        setDevDefaultIp = 0x24,     //Set device default IP
//        rmDevLog = 0x25,            //Clear device log
        gamma = 0x26,               //Gamma coefficient
        dhcpFlay = 0x27,
//        threshold = 0x28,           //Detection threshold
        verAngle = 0x29,            //Vertical opening angle
        trx25DDirectionAngle = 0x2a,//Trx direction
        aesaMode = 0x30,    		//Vertical opening angle mode
//        medianFilter = 0x31,        //Median filter
        paramInfo = 0x40,           //Device current parameters
//        tcpKeepAlive = 0x4d,        //Heartbeat detection
        versionInfo = 0x4E,         //Version information

        //AESA
//        AESAstartFrequency = 0x51,      //Start frequency
//        AESAscanWidth = 0x52,           //Sweep width
//        AESAoriginalPhase = 0x53,       //Initial phase
//        AESApulseWidth = 0x55,          //Transmission pulse width
//        AESAtransmittedPower = 0x56,    //Transmission power

//        firN = 0x70,
    };

#pragma pack(1)
    struct s_sonarParamBymode {
        float range;
        uint8_t signalMode;
        uint8_t tvgMode;
        uint8_t windowMode;
        uint8_t trxPower;
        float tvgA, tvgB, tvgC;
        float rangeRes;
        float angleHor;
        float angleResV;
        uint8_t rsv[32];
    };

    struct s_sonarParam {
        uint8_t validFlag;
        uint32_t hostUdpAddr;
        uint16_t hostUdpPort;
        float prt;
        float sv;
        uint8_t dataSource;
        uint8_t syncSource;
        uint8_t sonarMode;
        uint8_t brightness;
        uint8_t compressLevel;
        uint8_t aesaMode;
        float aesaAngle;
        float syncDelay;
        uint8_t runStatus;
        uint8_t udpSendFlag;
        uint8_t rsv[225];
        s_sonarParamBymode paramBymode[8];
    };
#pragma pack()

    /**
        * @brief Constructor
        * @param [in] sonarIP Sonar device IP
        * @param [in] sonarPort Sonar device command port number
        */
    LhForwardCMD(string sonarIP, uint16_t sonarPort);
    ~LhForwardCMD();

    std::string getVersion();

    /**
        * @brief Connect device function
        */
    LhForwardCMD::LfCMDRet connectDevice();
    LhForwardCMD::LfCMDRet connectDevice(string inHostIp); // Function to bind specific computer IP and establish TCP connection

    /**
        * @brief Disconnect device function
        */
    void disconnectDevice();

    /**
        * @brief Send command function
        * @param [in] info Command number
        * @param [in] arg Command argument
        * @param [out] reply Reply information
        */
    LhForwardCMD::LfCMDRet sendCMD(LfCMDInfo info);                 //Send command without arguments
    LhForwardCMD::LfCMDRet sendCMD(LfCMDInfo info, uint8_t arg);    //Send command with uint8 argument
    LhForwardCMD::LfCMDRet sendCMD(LfCMDInfo info, uint16_t arg);	//Send command with uint16 argument
    LhForwardCMD::LfCMDRet sendCMD(LfCMDInfo info, float arg);      //Send command with float argument

    LhForwardCMD::LfCMDRet sendCMD(LfCMDInfo info, uint8_t arg, float arg1); //Send phased array command

    LhForwardCMD::LfCMDRet sendCMD(LfCMDInfo info, float arg1, float arg2, float arg3);	//Send advanced TVG command
    LhForwardCMD::LfCMDRet sendCMD(LfCMDInfo info, uint32_t arg);	//Send command with uint32_t argument
    LhForwardCMD::LfCMDRet sendCMD(LfCMDInfo info, uint64_t arg);   //Downward time service
    LhForwardCMD::LfCMDRet sendCMD(LfCMDInfo info, void* reply);	//Send command with reply information, currently only version information

    LhForwardCMD::LfCMDRet AESAsendCMD(LfCMDInfo info, uint8_t arg, float arg1);    //AESA
    LhForwardCMD::LfCMDRet AESAsendCMD(LfCMDInfo info, uint8_t arg, uint8_t arg1);  //AESA transmission power

    /**
        * @brief Send UDP data receiver information configuration
        * @param [in] info Command number
        * @param [in] ip IP address to receive sonar device data e.g., ip[0]=192,ip[1]=168,ip[2]=1,ip[3]=83
        * @param [in] port UDP port number to receive sonar device data
        */
    LhForwardCMD::LfCMDRet sendCMD(LfCMDInfo info, uint8_t ip[4], uint16_t port);			//Send udpdata command

    /**
        * @brief Get the command socket
        */
    TYPE_SOCKET getCMDSocket() { return m_cmdSocket; }

private:
    LfCMDRet sendAndRecvActive(char* cmd, int cmdSize);

    TYPE_SOCKET m_cmdSocket;
    string m_sonarIP;
    uint16_t m_sonarPort;
};

}
