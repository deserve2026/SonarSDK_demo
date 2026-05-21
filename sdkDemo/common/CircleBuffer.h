/*!
* @file CircleBuffer.h
* @brief Circular buffer implementation
* @details
* @author liuhai
* @date 2019/11/27 12:38
*/
#pragma once

#include <cstdint>
#include <string>
#include <string.h>
#include <iostream>
#include <mutex>

class CircleBuffer
{
public:
	/**
    * @brief Circular buffer class constructor
    * @param [in] bufSize Size of each buffer / bytes
    * @param [in] bufNum Buffer count
	*/
	CircleBuffer(uint16_t bufSize, uint16_t bufNum);
	~CircleBuffer();

	/**
    * @brief Write into the circular buffer
    * @param [in] writeDataPtr Data pointer to be written
    * @param [in] dataSize Write data size / bytes
	*/
	void writeData(const char* writeDataPtr, uint16_t dataSize);
	
	/**
    * @brief Read-out circular buffer
    * @param [in] dataSize Read data size/byte
    * @param [out] readDataPtr Read out the position pointer
	*/
	void readData(uint16_t dataSize, char* readDataPtr);

	/**
    * @brief Read out circular buffer
    * @return Return the address of the data read this time
	*/
	char* readDataAddr();
	
    /**
     * @brief File save and read buffer
     * @return Return to read address
     */
    char* readDataAddr2();
	/**
    * @brief Determine whether there is unread data in the current circular buffer
    * @return true:There is unread data false:No unread data
	*/   
	bool isAnyData();
    bool isAnyData2();
    /**
    * @brief Set the current reading position
    */
    void setRead2Pos();

private:
	char* m_bufHeadPtr;
	char* m_bufEndPtr;
	char* m_writePtr;
	char* m_readPtr;
    char* m_readPtr2;
	const uint16_t m_bufSize;
	const uint16_t m_bufNum;
	std::mutex m_mutex;
	std::mutex m_mutex2;
};
