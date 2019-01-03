//
// Created by stephen on 18-6-27.
//

#ifndef CALSASOUND_H
#define CALSASOUND_H

#include <stdio.h>
#include <stdlib.h>
#include <alsa/asoundlib.h>

#include "waveformat.h"

/*
 * A Typical Sound Application
Programs that use the PCM interface generally follow this pseudo-code:

open interface for capture or playback
        set hardware parameters
        (access block_mode, data format, channels, rate, etc.)
while there is data to be processed:
read PCM data (capture)
or write PCM data (playback)
close interface
 */

class CAlsaSound {
public:
    CAlsaSound();

    void ShowPCMDetail();

    virtual ~CAlsaSound();

    void SetWaveFormat(WAVEFORMAT);

    void InitPlayDevice();
    void InitRecordDevice();
    void SetWavParameters(u16 nChannels, u32 sampling_rate, u16 wBitsPerSample);

    void PlayBuffer(WAVEBUFFER*);
    void SetLatency(snd_pcm_uframes_t latency);
    void CloseDevice();
    bool IsOpened4Play(){return isOpened4Play;}
    bool IsOpened4Record(){return isOpened4Record;}
    snd_pcm_sframes_t RecordToBuffer(WAVEBUFFER*, size_t);

private:
    char *playBuffer;
    char *recordBuffer;
    int sizePlayBuff;
    int sizeRecordBuff;
    snd_pcm_uframes_t latency;
    int dir;    //sub unit direction
    bool isOpened4Play;
    bool isOpened4Record;
    snd_pcm_t *playback_handle;//PCM设备句柄pcm.h
    snd_pcm_t *record_handle;//PCM设备句柄pcm.h
/*PCM hardware configuration space container
snd_pcm_hw_params_t is an opaque structure which contains a set of possible PCM hardware configurations.
 For example, a given instance might include a range of buffer sizes, a range of period sizes,
 and a set of several sample formats. Some subset of all possible combinations these sets may be valid,
 but not necessarily any combination will be valid.
When a parameter is set or restricted using a snd_pcm_hw_params_set* function, all of the other ranges
 will be updated to exclude as many impossible configurations as possible. Attempting to set a parameter
 outside of its acceptable range will result in the function failing and an error code being returned.
 */
    snd_pcm_hw_params_t *record_hw_params;//硬件信息和PCM流配置
    snd_pcm_sw_params_t *record_sw_params;//
    snd_pcm_stream_t streamPlay, streamRecord;
    char device[64];
    int block_mode;

    WAVEFORMAT waveformat;

    snd_pcm_format_t format;
    snd_output_t *output;

    int setparams_stream(snd_pcm_t *handle, snd_pcm_hw_params_t *pParams, const char *id);

    int setparams_bufsize(snd_pcm_t *handle, snd_pcm_hw_params_t *params, snd_pcm_hw_params_t *pt_params,
                          snd_pcm_uframes_t buffsize, const char *id);

    int setparams_set(snd_pcm_t *handle, snd_pcm_hw_params_t *params, snd_pcm_sw_params_t *sw_params, const char *id);
};


#endif //CALSASOUND_H
