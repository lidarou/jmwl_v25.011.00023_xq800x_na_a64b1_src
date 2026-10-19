/*
 * Copyright (C) 2014 The Android Open Source Project
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x0dbc7c
#define LOG_MODULE    AMOD_AUDIO

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <aic_cmd_api.h>
#include <alog_api.h>
#include "timer_inc.h"
#include "audio_util.h"
#include "pvmp3decoder_api.h"
#include "mp3reader.h"
//#include <audio_utils/sndfile.h>

enum {
    kInputBufferSize = 10 * 1024,
    kOutputBufferSize = 4608 * 2,
};

int mp3_test(int argc, const char **argv) {

#if 0
    if (argc != 3) {
        ALOGE("Usage %s <input file> <output file>\n", argv[0]);
        return EXIT_FAILURE;
    }
#endif
    ALOGI("yzl ad mp3_test argv 0:%s argv 1:%s\n", argv[0], argv[1]);

    // Initialize the config.
    tPVMP3DecoderExternal config;
    config.equalizerType = flat;
    config.crcEnabled = false;

    // Allocate the decoder memory.
    uint32_t memRequirements = pvmp3_decoderMemRequirements();
    void *decoderBuf = malloc(memRequirements);
    assert(decoderBuf != NULL);
    ALOGI("yzl ad mp3_test 0");

    // Initialize the decoder.
    pvmp3_InitDecoder(&config, decoderBuf);
    ALOGI("yzl ad mp3_test 1");

    // Open the input file.
    struct Mp3Reader mp3Reader;
    bool success = ad_mp3dec_init(&mp3Reader, argv[0]);
    if (!success) {
        ALOGE("Encountered error reading %s\n", argv[0]);
        free(decoderBuf);
        return EXIT_FAILURE;
    }
    ALOGI("yzl ad mp3_test 2");

    // Open the output file.

    FILE *handle = fopen(argv[1], "wb");
    if (handle == NULL) {
        ALOGE("Encountered error writing %s\n", argv[1]);
        ad_mp3dec_deinit(&mp3Reader);
        free(decoderBuf);
        return EXIT_FAILURE;
    }
    ALOGI("yzl ad mp3_test 3");

    // Allocate input buffer.
    uint8_t *inputBuf = (uint8_t*)(malloc(kInputBufferSize));
    assert(inputBuf != NULL);

    // Allocate output buffer.
    int16_t *outputBuf = (int16_t*)(malloc(kOutputBufferSize));
    assert(outputBuf != NULL);

    // Decode loop.
    int retVal = EXIT_SUCCESS;
    int count = 0;
    while (1) {
        // Read input from the file.
        uint32_t bytesRead;
        u32 time0 = aic_get_timestamp();
        bool success = getFrame(&mp3Reader, inputBuf, &bytesRead);
        if (!success) break;
        if(count == 500)
            break;
        count ++;
        // Set the input config.
        config.inputBufferCurrentLength = bytesRead;
        config.inputBufferMaxLength = 0;
        config.inputBufferUsedLength = 0;
        config.pInputBuffer = inputBuf;
        config.pOutputBuffer = outputBuf;
        config.outputFrameSize = kOutputBufferSize / sizeof(int16_t);

        ERROR_CODE decoderErr;

        ALOGI("yzl ad mp3_test 4");
        u32 time = aic_get_timestamp();
        decoderErr = pvmp3_framedecoder(&config, decoderBuf);
        if (decoderErr != NO_DECODING_ERROR) {
            ALOGE("Decoder encountered error\n");
            retVal = EXIT_FAILURE;
            break;
        }
        u32 time1 = aic_get_timestamp();
        ALOGI("yzl ad mp3_test 5 cost time:%d ms , byread:%d ms", (time1 - time) / 1000, (time1 - time0) / 1000);
        fwrite(outputBuf,getNumChannels(&mp3Reader), config.outputFrameSize, handle);

    ALOGI("yzl ad mp3_test 6");
    }

    // Close input reader and output writer.
    ad_mp3dec_deinit(&mp3Reader);
    fclose(handle);

    // Free allocated memory.
    free(inputBuf);
    free(outputBuf);
    free(decoderBuf);

    return retVal;
}

CONSOLE_EXPORT_CMD(mp3_test, mp3_test , mp3_test);

