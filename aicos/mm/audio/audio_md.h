
#include "aic_md_id.h"

#define AUD_MD_SIZE_MUSIC           0x298//0x156
#define AUD_MD_SIZE_REC             0x298//0x156
#define AUD_MD_SIZE_VOLTE           0x298//0x156
#define AUD_MD_SIZE_VOIP            0x298//0x156

#define AUD_MD_NUM_MUSIC            15
#define AUD_MD_NUM_REC              15
#define AUD_MD_NUM_VOLTE            20
#define AUD_MD_NUM_VOIP             20

#define AUD_MD_BASE_MUSIC           MD_ID_AUDIO_BASE
#define AUD_MD_BASE_REC             (AUD_MD_BASE_MUSIC + AUD_MD_NUM_MUSIC)
#define AUD_MD_BASE_VOLTE           (AUD_MD_BASE_REC + AUD_MD_NUM_REC)
#define AUD_MD_BASE_VOIP            (AUD_MD_BASE_VOLTE + AUD_MD_NUM_VOLTE)

enum {
    AUD_MD_MUSIC_START = AUD_MD_BASE_MUSIC,  /* 100 */
    AUD_MD_MUSIC_RCV = AUD_MD_MUSIC_START,   /* 100 */
    AUD_MD_MUSIC_SPK,                        /* 101 */
    AUD_MD_MUSIC_EP,                         /* 102 */
    AUD_MD_MUSIC_SPK_EP,                     /* 103 */
    AUD_MD_MUSIC_END = AUD_MD_BASE_MUSIC + AUD_MD_NUM_MUSIC - 1,
};

enum {
    AUD_MD_REC_START = AUD_MD_BASE_REC,     /* 115 */
    AUD_MD_REC_MAIN_MIC = AUD_MD_REC_START, /* 115 */
    AUD_MD_REC_EP_MIC,                      /* 116 */
    AUD_MD_REC_END = AUD_MD_BASE_REC + AUD_MD_NUM_REC - 1,
};

enum {
    AUD_MD_VOLTE_START = AUD_MD_BASE_VOLTE, /* 130 */
    AUD_MD_VOLTE_RCV = AUD_MD_VOLTE_START,  /* 130 */
    AUD_MD_VOLTE_SPK,                       /* 131 */
    AUD_MD_VOLTE_EP,                        /* 132 */
    AUD_MD_VOLTE_END = AUD_MD_BASE_VOLTE + AUD_MD_NUM_VOLTE - 1,
};

enum {
    AUD_MD_VOIP_START = AUD_MD_BASE_VOIP,   /* 150 */
    AUD_MD_VOIP_RCV = AUD_MD_VOIP_START,    /* 150 */
    AUD_MD_VOIP_SPK,                        /* 151 */
    AUD_MD_VOIP_EP,                         /* 152 */
    AUD_MD_VOIP_END = AUD_MD_BASE_VOIP + AUD_MD_NUM_VOIP - 1,
};

#define AUD_MD_SIZE_MAX     (2048 * 2)

/* Do not change */
enum audio_scen_e {
    AUD_SCEN_MUSIC = 0, /* 0 */
    AUD_SCEN_REC,       /* 1 */
    AUD_SCEN_VOLTE,     /* 2 */
    AUD_SCEN_VOIP,      /* 3 */
    AUD_SCEN_MAX
};

u32 audio_md_get_cur(void);
int audio_md_read(u32 id, u8 *buf);
int audio_md_write(u32 id, u8 *buf, u32 size);
int audio_md_set(u32 id);
int audio_md_set_buf(u32 id, u8 *buf, u32 size);
bool audio_md_check(u16 id);
u32 audio_md_get_count(u16 scen_id);
u32 audio_md_get_base(u16 scen_id);

