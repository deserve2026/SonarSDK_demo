/*!
* Copyright (C) Zhuhai Lanheng Technology Co., Ltd.
* All rights reserved
* @File LhForwardImage.h
* @Author liuhai
* @Brief Forward-looking sonar imaging library header file
* @Details Data preprocessing, image generation, bilinear interpolation
* @LastEditors liuhai
* @LastEditTime 2020/3/31 11:35
* @Version 2.0
*/

#pragma once
#include <stdint.h>
#include "LhForwardFrame.h"

#ifdef USE_WINDOWS
#define DLL_API _declspec(dllexport)
#else
#define DLL_API
#endif // !DLL_API

namespace LhForwardSDK
{
class DLL_API LhForwardImage
{
public:
    LhForwardImage(int width = 800, int height = 600);
    ~LhForwardImage();

    enum DataWidth
    {
        bit8 = 0x00,
        bit16 = 0x01,
        bit32 = 0x02,
        bit64 = 0x03,
        complexBit16 = 0x10,
        complexBit32 = 0x11,
        complexBit64 = 0x12,
        complexBit128 = 0x13,
        hfType = 0x20,
        floatType = 0x21,
        doubleType = 0x22,
    };

    /**
        * @brief Data preprocessing, perform data type conversion
        * @param [in] frameInfo Data frame information
        * @param [in] dataPtr Data before conversion
        * @param [out] intensityPtr Data after conversion
        */
    void preprocessData(const packageInfo& frameInfo, const uint8_t* dataPtr, double* intensityPtr);

    /**
        * @brief Polar coordinate data image generation
        * @param [in] frameInfo Frame information
        * @param [in] dataPtr Frame data
        * @param [out] sectorMemoryPtr Interpolated memory data
        * @param [out] imageRes	Image resolution meters/pixel
        * @note The memory size for sectorMemoryPtr parameter is width*height, i.e., the image data memory size
        *       imageRes image resolution: chord length of the physical sector image / (generated image width * 0.95)
        */
    void generateForwardImage(const packageInfo& frameInfo, const uint8_t* dataPtr, uint8_t* sectorMemPtr, double& imageRes);

    /*
        * @brief Color palette data
        * @param [out] colorTablePtr Color table pointer
        * @note Internal data copy, call once during initialization is sufficient
        */
    void getColorTable(char* colorTable);

#ifdef HAVE_LIBPNG // If this macro is defined, it means the libpng library is required
    // Temporarily set to public for external use
    /**
        * @brief PNG data decoding processing
        */
    int decodePng(const packageInfo& frameInfo, const uint8_t* dataPtr, int& width, int& height, uint8_t* dataOut);
#endif

private:
    /**
        * @brief Bilinear interpolation, calculate sector image memory
        * @param [in] dataPtr Frame data before interpolation
        * @param [in] dataWidth Data bit width
        * @param [out] sectorMemoryPtr Interpolated memory data
        * @note
        */
    void generateImageMem(const uint8_t* dataPtr, uint8_t dataWidth, uint8_t* sectorMemPtr);

    /**
        * @brief Bilinear interpolation parameter calculation
        * @param [in] numPerRow Points per row
        * @param [in] numInDist Points in range direction, i.e., rows per packet x packets per frame
        * @param [in] maxRange Maximum operating range
        * @param [in] distRes Range resolution
        * @param [in] horAngleRes Horizontal angle resolution
        * @param [out] imageRes	Image resolution meters/pixel
        * @note
        */
    void generateParameter(int numPerRow, int numInDist, double maxRange, double distReso, double horAngleReso, double& imageRes);

    void bilinerInp360Cal(const uint8_t* dataPtr, uint8_t dataWidth, int numInDist, double maxDist, int numPerRow, double distReso, uint8_t* sectorMemPtr, double horAngleReso, double& imageRes);

    void bilinerInp360GenerateMap();

private:
    // Data parameters
    int m_numPerRow = 0;		//Points per row, i.e., number of imaging beams
    int m_numInDist = 0;		//Points in range direction, i.e., rows per packet x packets per frame
    double m_distReso = 0;		//Range resolution
    double m_horAngleReso = 0;	//Horizontal angle resolution / degrees
    double m_maxDist = 0;		//Maximum operating range
    // Image parameters
    int	m_width;	//Image width after imaging / pixels
    int m_height;	//Image height after imaging / pixels
    double m_imageRes;	//Image resolution meters/pixel
    int m_sectorImgSize;
    // Bilinear interpolation coefficients
    int* m_channelCount;
    int* m_sampleCount;
    double* m_dy;
    double* m_dx;
    // Color table
    char m_colorTable[256][3];
    void generateColorTable();
};

}
