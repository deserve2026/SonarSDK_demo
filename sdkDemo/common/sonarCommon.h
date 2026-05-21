/*
* @file sonarCommon.h
* @brief General definition of equipment
* @details
* @author liuhai
* @date 2019/10/15 11:05
*/

#pragma once
#include <cstdint>
#include <string>

const uint16_t UDP_SOFT_ASK_PORT = 1009;    //Software UDP port
const uint16_t UDP_DATA_PORT = 5001;        //Sonar equipment upd data transmission port
const uint16_t TCP_CMD_PORT = 5007;         //Sonar equipment TCP command receiving port
const uint16_t UDP_REC_PORT = 5008;         //Sonar equipment UDP receiving port
const uint16_t UDP_REC_SDK_PORT = 5009;     //Sonar equipment UDP receiving port（SDK）
const uint16_t UDP_DEV_ASK_PORT = 9001;     //Device UDP query port

const int IMAGE_WIDTH = 968;
const int IMAGE_HEIGHT = 512;
