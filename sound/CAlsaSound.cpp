//
// Created by stephen on 18-6-27.
//

#include <sys/ioctl.h>
#include <iostream>
#include "CAlsaSound.h"
#include "waveformat.h"

void CAlsaSound::ShowPCMDetail() {
    int val;

    printf("ALSA library version: %s\n",
           SND_LIB_VERSION_STR);

    printf("\nPCM stream types:\n");
    for (val = 0; val <= SND_PCM_STREAM_LAST; val++)
        printf("  %s\n",
               snd_pcm_stream_name((snd_pcm_stream_t)val));

    printf("\nPCM access types:\n");
    for (val = 0; val <= SND_PCM_ACCESS_LAST; val++)
        printf("  %s\n",
               snd_pcm_access_name((snd_pcm_access_t)val));

    printf("\nPCM formats:\n");
    for (val = 0; val <= SND_PCM_FORMAT_LAST; val++)
        if (snd_pcm_format_name((snd_pcm_format_t)val)
            != NULL)
            printf("  %s (%s)\n",
                   snd_pcm_format_name((snd_pcm_format_t)val),
                   snd_pcm_format_description(
                           (snd_pcm_format_t)val));

    printf("\nPCM subformats:\n");
    for (val = 0; val <= SND_PCM_SUBFORMAT_LAST;
         val++)
        printf("  %s (%s)\n",
               snd_pcm_subformat_name((
                                              snd_pcm_subformat_t)val),
               snd_pcm_subformat_description((
                                                     snd_pcm_subformat_t)val));

    printf("\nPCM states:\n");
    for (val = 0; val <= SND_PCM_STATE_LAST; val++)
        printf("  %s\n",
               snd_pcm_state_name((snd_pcm_state_t)val));
}

void CAlsaSound::InitPlayDevice() {
    int ret;
    isOpened4Play = false;
    switch (waveformat.wBitsPerSample) {
        case 8:
            format = SND_PCM_FORMAT_S8;//SND_PCM_FORMAT_U8;
            break;
        case 16:
            format = SND_PCM_FORMAT_S16_LE;//SND_PCM_FORMAT_S16_BE,SND_PCM_FORMAT_U16_BE,SND_PCM_FORMAT_U16_LE
            break;
        case 24:
            format = SND_PCM_FORMAT_S24_LE;
            break;
        case 32:
            format = SND_PCM_FORMAT_S32_LE;
            break;
        default:
            format = SND_PCM_FORMAT_UNKNOWN;
            break;
    }
    snd_pcm_hw_params_t *pt_params, *p_params;
    snd_pcm_sw_params_t *p_swparams;
    //1. 打开PCM，最后一个参数为0意味着标准配置  Open PCM device for playback.
    //              returned PCM handle    ASCII identifier of the PCM handle
    //                                              Wanted stream           block_mode(SND_PCM_NONBLOCK, SND_PCM_ASYNC)
    ret = snd_pcm_open(&playback_handle, device, SND_PCM_STREAM_PLAYBACK, block_mode);
    if (ret < 0) {
        std::cerr<<"Playback open error: " << snd_strerror(ret) <<std::endl;
        return;
    }

    //2. 分配snd_pcm_hw_params_t结构体   Allocate a hardware parameters object.
    snd_pcm_hw_params_alloca(&pt_params);
    snd_pcm_hw_params_alloca(&p_params);
    snd_pcm_sw_params_alloca(&p_swparams);

    if ((ret = setparams_stream(playback_handle, pt_params, "playback")) < 0) {
        std::cerr << "Unable to set parameters for playback stream: " << snd_strerror(ret) << std::endl;
        return;
    }

    /* Set period size to 32 frames. */
    // A frame is equivalent of one sample being played
    // 1 frame = (num_channels) * (1 sample in bytes) = (2 channels) * (2 bytes (16 bits) per sample) = 4 bytes (32 bits)
    //ret = snd_pcm_hw_params_get_buffer_size(hw_params, &periodsize);
    // We can control when this PCM interrupt is generated, by setting a period size, which is set in frames.
    //periodsize = frame_bytes * 2;
    //snd_pcm_uframes_t bufsize = sizePlayBuff;
    //approximate target buffer duration in 10 ms / returned chosen approximate target buffer duration
    if ((ret = setparams_bufsize(playback_handle, p_params, pt_params, latency, "playback")) < 0) {
        std::cerr << "Unable to set bufsize for playback stream: " << snd_strerror(ret) << std::endl;
        exit(EXIT_FAILURE);
    }

    if ((ret = setparams_set(playback_handle, p_params, p_swparams, "playback")) < 0) {
        std::cerr << "Unable to set sw parameters for playback stream: " << snd_strerror(ret) << std::endl;
        exit(EXIT_FAILURE);
    }

    /* Use a buffer large enough to hold one period */
    //snd_pcm_hw_params_get_period_size(play_hw_params, &periodsize, &dir);
    isOpened4Play = true;
    return;
}

void CAlsaSound::SetWavParameters(u16 nChannels, u32 sampling_rate, u16 wBitsPerSample) {
    waveformat.wBitsPerSample = wBitsPerSample;
    waveformat.nChannels = nChannels;
    //CAlsaSound::format = format;
    waveformat.nSamplesPerSec = sampling_rate;
    waveformat.wFormatTag = 1;//WAVE_FORMAT_PCM;
    waveformat.nBlockAlign = wBitsPerSample*nChannels/(u16)8;
}

CAlsaSound::~CAlsaSound() {
    if (isOpened4Play || playback_handle != NULL || record_handle != NULL) {
        CloseDevice();
    }
    /*if (pWaveHeader != NULL){
        delete(pWaveHeader);
        pWaveHeader = NULL;
    }*/
}

void CAlsaSound::CloseDevice() {
    snd_pcm_drain(playback_handle);
    snd_pcm_close(playback_handle);
    playback_handle = NULL;
    snd_pcm_drain(record_handle);
    snd_pcm_close(record_handle);
    record_handle = NULL;
    isOpened4Play = false;
    if (playBuffer != NULL) {
        delete playBuffer;
        sizePlayBuff = 0;
        playBuffer = NULL;
    }
    if (recordBuffer != NULL){
        delete(recordBuffer);
        sizeRecordBuff = 0;
        recordBuffer = NULL;
    }
}

void CAlsaSound::SetWaveFormat(WAVEFORMAT wavfmt) {
    memset(&waveformat, 0, sizeof(WAVEFORMAT));
    memcpy(&waveformat, &wavfmt, sizeof(WAVEFORMAT));
}

CAlsaSound::CAlsaSound()
        : isOpened4Play(false)
        , isOpened4Record(false)
        , playBuffer(NULL)
        , playback_handle(NULL)
        , recordBuffer(NULL)
        , record_handle(NULL)
        , record_hw_params(NULL)
        , record_sw_params(NULL)
        , format(SND_PCM_FORMAT_S16_LE)
        , streamPlay(SND_PCM_STREAM_PLAYBACK)
        , streamRecord(SND_PCM_STREAM_CAPTURE)
        , block_mode(0)
        , dir(0)
        , output(NULL)
        , latency(882)
        , sizePlayBuff(65536){
    //set default parameters
    //frame_bytes = pWaveHeader->wBitsPerSample * pWaveHeader->nChannels / 8;
    char devicename[] = "default";
    memcpy(device, devicename, sizeof(devicename));
    int err = snd_output_stdio_attach(&output, stdout, 0);
    if (err < 0) {
        std::cerr << "Output failed: " << snd_strerror(err) << std::endl;
    }
}

int CAlsaSound::setparams_stream(snd_pcm_t *handle, snd_pcm_hw_params_t *params, const char *id) {
    int err;
    unsigned int rrate;

    /*if (err < 0) {
        std::cerr << "snd_pcm_hw_params_alloca error: " <<snd_strerror(err) << std::endl;
        return false;
    }*/

    //3. 初始化hw_params   Fill it in with default values. OR Set the desired hardware parameters.
    err = snd_pcm_hw_params_any(handle, params);
    if (err < 0) {
        std::cerr << "Broken configuration for " << snd_strerror(err) <<
                  " PCM: no configurations available: " << id << std::endl;
        return err;
    }
    err = snd_pcm_hw_params_set_rate_resample(handle, params, 1);
    if (err < 0) {
        std::cerr << "Resample setup failed for " << id << " (val 1): " << snd_strerror(err) << std::endl;
        return err;
    }
    //4. 初始化访问权限    Interleaved block_mode
    //Interleaving means that we alternate samples for the left and right channel (LRLRLR).
    err = snd_pcm_hw_params_set_access(handle, params, SND_PCM_ACCESS_RW_INTERLEAVED);
    if (err < 0) {
        std::cerr << "Access type not available for " << id << ": " << snd_strerror(err) << std::endl;
        return err;
    }
    //5. 初始化采样格式SND_PCM_FORMAT_S16_LE, 16位    Signed 16-bit little-endian format
    err = snd_pcm_hw_params_set_format(handle, params, format);
    if (err < 0) {
        std::cerr << "Sample format not available for " << id << ": " << snd_strerror(err) << std::endl;
        return err;
    }
    //7. 设置通道数量     Two channels (stereo)
    err = snd_pcm_hw_params_set_channels(handle, params, waveformat.nChannels);
    if (err < 0) {
        std::cerr << "Channels count (" << waveformat.nChannels << ") not available for " << id << ": " << snd_strerror(err) << std::endl;
        return err;
    }
    //6. 设置采样率，如果硬件不支持我们设置的采样率，将使用最接近的
    //val = 44100,有些录音采样频率固定为8KHz
    //44100 bits/second sampling rate (CD quality)
    rrate = waveformat.nSamplesPerSec;
    err = snd_pcm_hw_params_set_rate_near(handle, params, &rrate, 0);
    if (err < 0) {
        std::cerr << "Rate " << rrate << "Hz not available for " << id << ": " << snd_strerror(err) << std::endl;
        return err;
    }
    if ((int)rrate != waveformat.nSamplesPerSec) {
        std::cerr << "Rate doesn't match (requested " << waveformat.nSamplesPerSec << "iHz, get " << err << "Hz)" << std::endl;
        return -EINVAL;
    }
    return 0;
}

int CAlsaSound::setparams_bufsize(snd_pcm_t *handle, snd_pcm_hw_params_t *params, snd_pcm_hw_params_t *tparams,
                                  snd_pcm_uframes_t bufsize, const char *id) {
    int err;
    snd_pcm_uframes_t periodsize;

    snd_pcm_hw_params_copy(params, tparams);
    periodsize = bufsize*2;
    err = snd_pcm_hw_params_set_buffer_size_near(handle, params, &periodsize);
    if (err < 0) {
        std::cerr << "Unable to set buffer size "<< bufsize * 2 << " for " << id << ": " << snd_strerror(err) << std::endl;
        return err;
    }
    /*if (period_size > 0)
        periodsize = period_size;
    else*/
    periodsize /= 2;    //periodsize auto
    err = snd_pcm_hw_params_set_period_size_near(handle, params, &periodsize, 0);
    /*snd_pcm_hw_params_get_period_time(params, &playLatency, 0);*/
    if (err < 0) {
        std::cerr << "Unable to set period size " << periodsize << " for "<< id <<": %s\n" << snd_strerror(err) << std::endl;
        return err;
    }

    return 0;
}

int CAlsaSound::setparams_set(snd_pcm_t *handle, snd_pcm_hw_params_t *params, snd_pcm_sw_params_t *swparams,
                              const char *id) {
    int err;
    snd_pcm_uframes_t val;
    //8. 设置hw_params    Write the parameters to the driver
    err = snd_pcm_hw_params(handle, params);
    if (err < 0) {
        std::cerr << "Unable to set hw params for " << id << ": " << snd_strerror(err) << std::endl;
        return err;
    }
    err = snd_pcm_sw_params_current(handle, swparams);
    if (err < 0) {
        std::cerr << "Unable to determine current swparams for " << id << ": " << snd_strerror(err) << std::endl;
        return err;
    }
    err = snd_pcm_sw_params_set_start_threshold(handle, swparams, 0x7fffffff);
    if (err < 0) {
        std::cerr << "Unable to set start threshold mode for " << id << ": " << snd_strerror(err) << std::endl;
        return err;
    }
    /*if (!block_mode)
        val = 4;
    else
        snd_pcm_hw_params_get_period_size(params, &val, NULL);
    err = snd_pcm_sw_params_set_avail_min(handle, swparams, val);
    if (err < 0) {
        std::cerr << "Unable to set avail min for " << id << ": " << snd_strerror(err) << std::endl;
        return err;
    }
    err = snd_pcm_sw_params(handle, swparams);
    if (err < 0) {
        std::cerr << "Unable to set sw params for " << id << ": " << snd_strerror(err) << std::endl;
        return err;
    }*/
    return 0;
}

void CAlsaSound::InitRecordDevice() {
    int ret;
    isOpened4Record = false;
    switch (waveformat.wBitsPerSample) {
        case 8:
            format = SND_PCM_FORMAT_S8;//SND_PCM_FORMAT_U8;
            break;
        case 16:
            format = SND_PCM_FORMAT_S16_LE;//SND_PCM_FORMAT_S16_BE,SND_PCM_FORMAT_U16_BE,SND_PCM_FORMAT_U16_LE
            break;
        case 24:
            format = SND_PCM_FORMAT_S24_LE;
            break;
        case 32:
            format = SND_PCM_FORMAT_S32_LE;
            break;
        default:
            format = SND_PCM_FORMAT_UNKNOWN;
            break;
    }
    snd_pcm_hw_params_t *pt_params, *p_params;
    snd_pcm_sw_params_t *p_swparams;
    snd_pcm_hw_params_t *ct_params, *c_params;
    snd_pcm_sw_params_t *c_swparams;

    ret = snd_pcm_open(&record_handle, device, SND_PCM_STREAM_CAPTURE, block_mode);
    if (ret < 0) {
        std::cerr <<  "Record open error: " << snd_strerror(ret) << std::endl;
        exit(EXIT_FAILURE);
    }

    //2. 分配snd_pcm_hw_params_t结构体   Allocate a hardware parameters object.
    snd_pcm_hw_params_alloca(&pt_params);
    snd_pcm_hw_params_alloca(&p_params);
    snd_pcm_sw_params_alloca(&p_swparams);
    snd_pcm_hw_params_alloca(&ct_params);
    snd_pcm_hw_params_alloca(&c_params);
    snd_pcm_sw_params_alloca(&c_swparams);

    if ((ret = setparams_stream(record_handle, ct_params, "record")) < 0) {
        std::cerr << "Unable to set parameters for record stream: " << snd_strerror(ret) << std::endl;
        exit(EXIT_FAILURE);
    }

    if ((ret = setparams_bufsize(record_handle, c_params, ct_params, latency, "record")) < 0) {
        std::cerr << "Unable to set bufsize for record stream: " << snd_strerror(ret) << std::endl;
        exit(EXIT_FAILURE);
    }

    if ((ret = setparams_set(record_handle, c_params, c_swparams, "record")) < 0) {
        std::cerr << "Unable to set sw parameters for record stream: " << snd_strerror(ret) << std::endl;
        exit(EXIT_FAILURE);
    }

    isOpened4Record = true;
}

void CAlsaSound::SetLatency(snd_pcm_uframes_t latency) {
    CAlsaSound::latency = latency;
}

void CAlsaSound::PlayBuffer(WAVEBUFFER* pWaveBuffer) {
    if (!isOpened4Play) {
        std::cerr << "音频设备未准备好" << std::endl;
        return;
    }
    //9. 写音频数据到PCM设备
    if(pWaveBuffer->buf_size > 0)
    {
        //std::cout << "本次语音长度：" << pWaveBuffer->buf_size << "字节" << std::endl;
        snd_pcm_sframes_t ret = snd_pcm_writei(playback_handle, pWaveBuffer->buffer, pWaveBuffer->buf_size / waveformat.nBlockAlign);//
        if (ret == -EPIPE)
        {
            /* EPIPE means underrun */
            std::cerr << "underrun occurred" << std::endl;
            //完成硬件参数设置，使设备准备好
            snd_pcm_prepare(playback_handle);
        }
        else if (ret < 0)
        {
            std::cerr << "error from writei: " << snd_strerror(ret) << std::endl;
        }
    }

}

snd_pcm_sframes_t CAlsaSound::RecordToBuffer(WAVEBUFFER* pWaveBuffer, size_t size) {
    snd_pcm_uframes_t ret = snd_pcm_readi(record_handle, pWaveBuffer->buffer, size);//
    if(ret == -EPIPE) {//EPIPE means overrun
        std::cerr<<"-EPIPE"<<std::endl;
        snd_pcm_prepare(record_handle);
    } else if(ret <0 )
        std::cerr << "read error: " << snd_strerror((ret)) << std::endl;
    else if (ret != size)
        std::cerr<<"short write: "<<ret<<" bytes."<<std::endl;
    pWaveBuffer->buf_size = ret * waveformat.nBlockAlign;
    return ret;
}
