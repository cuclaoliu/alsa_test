//
// Created by stephen on 18-6-27.
//

#include <iostream>
#include "CAlsaSound.h"
#define play_file "../data_files/sample.wav"//
//#define play_file "../data_files/young_and_beautiful.wav"//
#define record_file "../data_files/record_sound.wav"

CAlsaSound snd;

void ReadWavHeader(FILE *pFile, WAVEFORMAT& waveformat) {
    WAVE_HEADER* pWaveHeader = new WAVE_HEADER;
    memset(pWaveHeader, 0, sizeof(WAVE_HEADER));
    rewind(pFile);
    int rt = fread(pWaveHeader, 1, sizeof(WAVE_HEADER), pFile);
    if(rt != sizeof(WAVE_HEADER)){
        std::cout<<"read wave file ERROR."<<std::endl;
        return;
    }
    memcpy(&waveformat, &pWaveHeader->waveformat, sizeof(WAVEFORMAT));
}

void PlayWave(const char *wavFile)
{
    int len, ret;

    FILE *pFile = fopen(wavFile, "r");
    if (pFile == NULL){
        std::cout<<"can NOT open wave file."<<std::endl;
        return;
    }

    WAVEFORMAT waveformat;
    ReadWavHeader(pFile, waveformat);
    snd.SetWaveFormat(waveformat);

    if (!snd.IsOpened4Record()) {
        snd.SetLatency(waveformat.nSamplesPerSec);
        //setWavParameters(pWaveHeader->nChannels, pWaveHeader->nSamplesPerSec, pWaveHeader->wBitsPerSample);
        snd.InitPlayDevice();
    }
    int sizePlayBuff = 1024; //latency * waveformat.nBlockAlign;
    char* playBuffer = new char[sizePlayBuff];
    std::cout << "size = " << sizePlayBuff << std::endl;

    //fseek(pFile, 44, SEEK_SET);
    int index = 0;

    while (1)
    {
        len = fread(playBuffer, 1, sizePlayBuff, pFile);
        if(len == 0)
        {
            fprintf(stderr, "end of file on input\n");
            break;
        }
        else if (len != sizePlayBuff)
        {
        }
        //9. 写音频数据到PCM设备
        if(len > 0)
        {
            WAVEBUFFER wavebuffer;
            wavebuffer.buf_size = len;
            wavebuffer.buffer = playBuffer;
            snd.PlayBuffer(&wavebuffer);
        }
        //sleep(1);   //测试播1秒停1秒
    }
    fclose(pFile);
    delete playBuffer;
}

void RecordWave(const char *recordFile) {
    int sampleRate = 44100;
    int wBitsPerSample = 16;
    int nChannel = 2;
    int nBlockAlign = wBitsPerSample * nChannel / 8;
    if (!snd.IsOpened4Record()) {
        snd.SetWavParameters(2, sampleRate, 16);
        snd.SetLatency(882);
        snd.InitRecordDevice();
    }
    FILE *pFile = fopen(recordFile, "w");
    if (pFile == NULL){
        std::cout<<"can NOT create wave file."<<std::endl;
        return;
    }

    //periodsize = sampleRate / 100;
    WAVEBUFFER wavebuffer;
    wavebuffer.buf_size = 8192; //latency*waveformat.nBlockAlign; /* 2 bytes/sample, 2 channels */
    wavebuffer.buffer = new char[wavebuffer.buf_size];
    std::cout << "size = " << wavebuffer.buf_size << std::endl;
    int loops = 6* sampleRate /(wavebuffer.buf_size/nBlockAlign);
    while(loops-->0){
        snd_pcm_sframes_t ret = snd.RecordToBuffer(&wavebuffer, 882);
        if (ret > 0){
            size_t wr_ret = fwrite(wavebuffer.buffer, 1, (wavebuffer.buf_size), pFile);
            if (wr_ret != ret*nBlockAlign){
                std::cerr<<"fwrite"<<std::endl;
            }
        }

    }
    fclose(pFile);
    delete wavebuffer.buffer;
}

int main(int argc, char* argv[])
{
    //CAlsaSound snd;
    //snd.ShowPCMDetail();
    std::cout<<"playing " << play_file <<"..."<<std::endl;
    PlayWave(play_file);//argv[1]
    sleep(2);
    std::cout<<"play done."<<std::endl;

    std::cout<<"recording..."<<std::endl;
    RecordWave(record_file);
    std::cout<<"record done."<<std::endl;
    sleep(2);
    exit(0);
}