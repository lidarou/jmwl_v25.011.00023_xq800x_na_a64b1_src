#ifndef __RTP_H__
#define __RTP_H__
#ifdef __cplusplus
extern "C" {
#endif
/** ??RTP?????? */
int rtp_header_length(const void* rtp, int rtpLen);
unsigned short rtp_unpack_uint16(const void* buf);
unsigned rtp_unpack_uint24(const void* buf);
unsigned rtp_unpack_uint32(const void* buf);
void rtp_pack_uint16(void* buf, unsigned short val);
void rtp_pack_uint24(void* buf, unsigned val);
void rtp_pack_uint32(void* buf, unsigned val);
struct jrtc_photo_t;
enum RTP_PT
{
    RTP_IDR     = 1, /*< video key frame */
    RTP_FIRST   = 2, /*< first packet of the frame */
    RTP_ANNEXB  = 4, /*< AnnexB format stream */
    RTP_PARAM   = 8, /*< param packet */
    RTP_NRI     = 0xF0, /*< nal_ref_idc */
};
/* ???????RTP? */
struct rtp_buf_t
{
    void* buf;
    unsigned len[48];
    unsigned size;
    unsigned types;/* ?????,RTP_PT ??? */
    void (*effect)(struct jrtc_photo_t* image);/* ???????? */
};

struct rtp_file_t;
typedef unsigned (*rtp_file_fn)(void* buf, unsigned len, struct rtp_file_t* rf);
struct rtp_file_t
{
    void* stream; /*< ?????? */
    rtp_file_fn filefn;/**< IO ?? */
    unsigned startMs;
};
/** ???RTP?????
 * @param[in] fn ????
 * @return ???? 0
 * ??:
 * @code
 * static unsigned write_rtp(void* buf, unsigned len, struct rtp_file_t* rf)
 * {
 *  if (!rf->stream) return 0;
 *  return fwrite(buf, 1, len, (FILE*)rf->stream);
 * }
 * struct rtp_file_t rf;
 * rf.stream = fopen('xx.rtp', 'wb+')
 * rtp_dump_init(&rf, write_rtp, nowMs);
 * @endcode
 */
int rtp_dump_init(struct rtp_file_t* rf, rtp_file_fn fn, unsigned nowMs);
/** ??????RTP?
 * @param[in] nowMs ??RTP????????
 */
int rtp_dump_packet(struct rtp_file_t* rf, void* rtp, int rtpLen, unsigned nowMs);

/** ???RTP?????
 * @param[in] fn ????
 * @return ???? 0
 * ??:
 * @code
 * static unsigned read_rtp(void* buf, unsigned len, struct rtp_file_t* rf)
 * {
 *  if (!rf->stream) return 0;
 *  return fread(buf, 1, len, (FILE*)rf->stream);
 * }
 * struct rtp_file_t rf;
 * rf.stream = fopen('xx.rtp', 'rb')
 * rtp_read_init(&rf, read_rtp, nowMs);
 * @endcode
 */
int rtp_read_init(struct rtp_file_t* rf, rtp_file_fn fn, unsigned nowMs);
/** ??????RTP?
 * @param[out] nowMs ??RTP???????
 */
int rtp_read_packet(struct rtp_file_t* rf, void* rtp, int rtpBufferSize, unsigned* nowMs);

/** ?AMR???RTP?
 * @param[in] bAlignment ??????
 * @param[in] iFrmCnt    AMR???
 * @code
 *  int MME_ReadAudioRTP(void* rtp, int rtpBufferSize, int rtpHeaderLength, unsigned timeoutMs) {
 *      uint8_t *amr[]; int len[];//????AMR??
 *      int rtpLen = rtp_pack_amr_init(rtp, rtpHeaderLength, true, true, iFrmCnt);
 *      assert(rtpLen == 0);
 *      for (int i=0; i<iFrmCnt; ++i) {
 *          rtpLen = rtp_pack_amr(rtp, rtpBufferSize, amr[i], len[i]);
 *      }
 *      return rtpLen;//RTP?????
 *  }
 * @endcode
 */
int rtp_pack_amr_init(void* rtp, int rtpHeaderLength, char isAMRWB, char bAlignment, char iFrmCnt);

/** ??????RTP???, ??-1 */
int rtp_pack_amr(void* rtp, int rtpBufferSize, const void* amr, int armLen);

/** ?RTP??AMR
 * @param[in] bAlignment ??????,??????AMR????
 * @return ??iFrmCnt ??
 * @code
 *  const int iFrmCnt = rtp_unpack_amr_init(rtp, rtpBufferSize, true, true);
 *  for (int i=0; i<iFrmCnt; ++i) {
 *      char amr[64];
 *      int len = rtp_unpack_amr(rtp, rtpBufferSize, amr);
 *  }
 * @endcode
 */
int rtp_unpack_amr_init(void* rtp, int rtpBufferSize, char isAMRWB, char bAlignment);
int rtp_unpack_amr(void* rtp, int rtpBufferSize, char amr[64]);

/** NetEQ ?????
 * @param[in] codecName ?????,?????PCMU,PCMA,AMR
 * @param[in] sampleRate ????
 * @param[in] payloadType
 */
struct rtp_neteq_t* rtp_neteq_alloc(const char* codecName, int sampleRate, unsigned char payloadType);
void                rtp_neteq_free(struct rtp_neteq_t* neteq);

/** ??RTP?,??????
 * @param[in] ms ??RTP???????
 */
int                 rtp_neteq_write(struct rtp_neteq_t* neteq, void* rtp, int rtpLen, unsigned ms);

/** ??PCM????????
 * @param[out]  sampleNum ????. ???????
 */
int                 rtp_neteq_read(struct rtp_neteq_t* neteq, void* pcm_uint16, int* sampleNum);

/** ???????????????RTP?
 *
 * - ???: rtp.buf ? rtp.len[0] ?????, rtp.size ??????,
 *           [rtp.buf - rtpHeaderLength, rtp.buf + rtp.size) ???????
 *
 * - ???: rtp.buf ????RTP?????, rtp.size ????RTP?????
 *           rtp.len[] ???RTP??, ?????RTP????? rtpPacketLength
 *
 * @param[in,out] rtp ????,????????RTP?
 * @param[in] rtpPacketLength ????????
 * @param[in] rtpHeaderLength ????????
 *
 * @return ????RTP???
 *
 * @code
 * int MME_ReadVideoRTP(struct rtp_buf_t* rtp, int rtpBufferSize, int rtpHeaderLength, MMEImage*image, unsigned timeoutMs) {
 *      assert(rtp->size == 0);
 *      rtp->buf = h264;//????
 *      rtp->len[0] = h264_len;///????
 *      rtp->size = buflen;//??????[h264 - rtpHeaderLength, h264 + buflen)
 *      int num = rtp_split_frame(rtp, rtpBufferSize, rtpHeaderLength);
 *      ....
 *      return num;
 * }
 * @endcode
 */
int rtp_split_frame(struct rtp_buf_t* rtp, int rtpPacketLength, int rtpHeaderLength);

#ifdef __cplusplus
}
#endif

#endif // __RTP_H__
