//
// Created on 8/19/18.
//

#ifndef ALSA_PCM_WAVE_STRUCTURES
#define ALSA_PCM_WAVE_STRUCTURES

#include <cstring>

typedef unsigned int u32;
typedef unsigned short u16;
typedef unsigned char u8;
#pragma pack(push)
#pragma pack(1)     //1字节对齐

struct WAVEFORMAT {
    u16   	wFormatTag;     // 编码格式
    u16   	nChannels;      // 声道数
    u32 	nSamplesPerSec;  /*采样频率*/
    u32 	nAvgBytesPerSec; /*每秒所需字节数*/
    u16		nBlockAlign; /*数据块对齐单位,每个采样需要的字节数*/
    u16		wBitsPerSample;/*每个采样需要的bit数*/
};

struct WAVE_HEADER {
    u8    	RiffID [4];     //riff标志符号
    u32     RiffSize;
    u8    	WaveID[4];      //格式类型 wave
    u8    	FmtID[4];       // "fmt"
    u32     FmtSize;        //size of wave format matex
    WAVEFORMAT waveformat;
    u8		DataID[4];      //"data"
    u32 	nDataBytes;     //音频数据的大小
};

struct WAVEBUFFER{
    char*   buffer;
    size_t  buf_size;
    //WAVEBUFFER_LINK* next;
};
#pragma pack(pop) /* 恢复先前的pack设置 */
#endif //ALSA_PCM_WAVE_STRUCTURES
