/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file audio engine resource file, it contains music data array for
 *    debug.
 */

__ALIGNED(2) const char music_file[] = {
#ifdef CONFIG_ADSP_EDA_MP3
    #include "20s_fengyi.h"
#elif defined (CONFIG_MP3_8K_MONO)
    #include "fengyi_8k_mono.h"
#elif defined (CONFIG_MP3_NORMAL)
    #include "tone_441.h"
#elif defined (CONFIG_PCM_16K_MONO)
    #include "1k_16000.h"
#elif defined (CONFIG_PCM_8K_MONO)
    #include "1k_8000.h"
#elif defined (CONFIG_PCM_16K_DUAL)
    #include "1k_16k_2ch.h"
#elif defined (CONFIG_PCM_8K_DUAL)
    #include "1k_8k_2ch.h"
#elif defined (CONFIG_PCM_48K_MONO)
    #include "1k_48000.h"
#elif defined (CONFIG_PCM_48K_DUAL)
    #include "1k_48k_2ch.h"
#elif defined (CONFIG_CHIRP_48K_MP3)
    #include "chirp_48k_mp3.h"
#elif defined (CONFIG_CHIRP_48K_WAV)
    #include "chirp_48k_wav.h"
#elif defined (CONFIG_MIDI_FILE)
    #include "midi_file.h"
#else
    /* none */
#endif
};

