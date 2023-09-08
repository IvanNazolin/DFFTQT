#ifndef _CUSTOM_ALGO_H_
#define _CUSTOM_ALGO_H_

#ifdef __cplusplus
extern "C" {
#endif 

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

// chIndexList index define
#define AI0         0
#define AI1         1
#define AI2         2
#define AI3         3
#define TACHOMETER  100

// InputRange
#define B10		1
#define B1d25	4

typedef struct _Sensor{
    char* type;
    double sensitivity;
}Sensor;

typedef struct _ChannelInfo{
	uint32_t channelNum;
	uint16_t* channelList;
}ChannelInfo;

typedef struct _DeviceInfo{
    double rate;
    uint32_t dataCount;
	uint32_t inputRange;
	ChannelInfo channelInfo;
    Sensor sensor;
}DeviceInfo;


#ifdef __linux__ 
double* CustomAlgo(uint16_t chIndex, void *rawData, DeviceInfo devInfo, char* customParams, uint32_t* outCount);
#endif 

#ifdef _WIN32 
double* CustomAlgo(uint16_t chIndex, double* raw, DeviceInfo devInfo, char* customParams, uint32_t* outCount);
#endif 

#ifdef __cplusplus
}
#endif

#endif // _CUSTOM_ALGO_H_
