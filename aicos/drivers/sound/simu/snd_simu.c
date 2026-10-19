/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file is used in simulate envioment.
 */

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x16d7d7
#define LOG_MODULE    AMOD_SOUND

extern int audio_clt_init(void);
extern int stream_pcm_init(void);
extern int iis_init(void);
extern int es8374_codec_init(void);
extern int snd_dai_init(void);
extern int snd_console_simu_init(void);
extern int stream_compr_init(void);

int snd_simu_init(void)
{
    es8374_codec_init();
    snd_dai_init();
    stream_pcm_init();
    stream_compr_init();
    /* audio_clt_init(); */
    iis_init();
    snd_console_simu_init();

    return 0;
}

