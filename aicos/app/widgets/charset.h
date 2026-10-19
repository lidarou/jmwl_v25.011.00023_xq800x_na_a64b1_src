

#ifndef __CHARSET_H__
#define __CHARSET_H__

#ifdef __cplusplus
extern "C"
{
#endif

    /* Enum of all supported charsets*/
    typedef enum {
        CHSET_BASE,
        CHSET_ASCII,
        CHSET_ARABIC_ISO,                           // ISO 8859-6 (Arabic)
        CHSET_ARABIC_WIN,                           // Windows-1256 (Arabic)
        CHSET_PERSIAN_CP1097,                       // CP1097  (Persian) 
        CHSET_PERSIAN_CP1098,                       // CP1098  (Persian)
        CHSET_BALTIC_ISO,                           // ISO-8859-4 (Baltic language group)
        CHSET_BALTIC_WIN,                           // Windows-1257 (Baltic language group)
        CHSET_CEURO_ISO,                            // ISO-8859-2 (Central european language group)
        CHSET_CEURO_WIN,                            // Windows-1250 (Central european language group)
        CHSET_CYRILLIC_ISO,                         // ISO-8859-5 (Cyrillic script based languages)
        CHSET_CYRILLIC_WIN,                         // Windows-1251 (Cyrillic script)
        CHSET_GREEK_ISO,                            // ISO-8859-7 (Greek)
        CHSET_GREEK_WIN,                            // Windows-1253 (Greek)
        CHSET_HEBREW_ISO,                           // ISO-8859-8 (Hebrew)
        CHSET_HEBREW_WIN,                           // Windows-1255 (Hebrew)
        CHSET_LATIN_ISO,                            // ISO-8859-4 (North european language group)
        CHSET_NORDIC_ISO,                           // ISO-8859-10 (Nordic language group)
        CHSET_SEURO_ISO,                            // ISO-8859-3 (South european language group)
        CHSET_TURKISH_ISO,                          // ISO-8859-9 (Turkish)
        CHSET_TURKISH_WIN,                          // Windows-1254 (Turkish)
        CHSET_WESTERN_ISO,                          // ISO-8859-1 (Latin script based languages)
        CHSET_ARMENIAN_ISO,                         // ISO-10585 ARMSCII (Armenian)
        CHSET_THAI_WIN,                             // Windows-874 (Thai)
        CHSET_VIETNAMESE_WIN,                       // Windows-1258 (Vietnamese)
        CHSET_KOI8_R,                               // KOI8-russian 
        CHSET_TIS_620,                              // Thai Industrial Standard 620
        CHSET_WESTERN_WIN,                          // Windows-1252 (Latin script based languages)

        /* Add new (8-bit) encodings above this line */
        CHSET_BIG5,                                 // Big5 (Traditional chinese)
        CHSET_GB2312,                               // GB2312 (Simplified chinese)
        CHSET_HKSCS,                                // HKSCS 2004 (Hong Kong chinese)
        CHSET_SJIS,                                 // SJIS (Japanese)
        CHSET_GB18030,                              // GB18030 (Simplified chinese-extended)
        CHSET_UTF7,                                 // UTF-7 
        CHSET_EUCKR,                                // Extended Unix Code - korean

        /* Place all CJK encodings above this one */
        CHSET_UTF16LE,                              // UTF-16LE
        CHSET_UTF16BE,                              // UTF-16BE
        CHSET_UTF8,                                 // UTF-8
        CHSET_UCS2,                                 // UCS2
        CHSET_TOTAL,
        } chset_enum;


    /* do not change the order */

    /* Enum of all supporte charset pairs */
    typedef enum _ENCODING_SCHEMES_ENUM {
        BASE_ENCODING_SCHEME,
        ASCII_TO_UCS2,
        UCS2_TO_ASCII,
        ARABIC_ISO_TO_UCS2,
        UCS2_TO_ARABIC_ISO,
        ARABIC_WINDOWS_TO_UCS2,
        UCS2_TO_ARABIC_WINDOWS,
        PERSIAN_CP1097_TO_UCS2,
        UCS2_TO_PERSIAN_CP1097,
        PERSIAN_CP1098_TO_UCS2,
        UCS2_TO_PERSIAN_CP1098,
        BALTIC_ISO_TO_UCS2,
        UCS2_TO_BALTIC_ISO,
        BALTIC_WINDOWS_TO_UCS2,
        UCS2_TO_BALTIC_WINDOWS,
        CENTRAL_EUROPEAN_ISO_TO_UCS2,
        UCS2_TO_CENTRAL_EUROPEAN_ISO,
        CENTRAL_EUROPEAN_WINDOWS_TO_UCS2,
        UCS2_TO_CENTRAL_EUROPEAN_WINDOWS,
        CYRILLIC_ISO_TO_UCS2,
        UCS2_TO_CYRILLIC_ISO,
        CYRILLIC_WINDOWS_TO_UCS2,
        UCS2_TO_CYRILLIC_WINDOWS,
        GREEK_ISO_TO_UCS2,
        UCS2_TO_GREEK_ISO,
        GREEK_WINDOWS_TO_UCS2,
        UCS2_TO_GREEK_WINDOWS,
        HEBREW_ISO_TO_UCS2,
        UCS2_TO_HEBREW_ISO,
        HEBREW_WINDOWS_TO_UCS2,
        UCS2_TO_HEBREW_WINDOWS,
        LATIN_ISO_TO_UCS2,
        UCS2_TO_LATIN_ISO,
        NORDIC_ISO_TO_UCS2,
        UCS2_TO_NORDIC_ISO,
        SOUTH_EUROPEAN_ISO_TO_UCS2,
        UCS2_TO_SOUTH_EUROPEAN_ISO,
        TURKISH_ISO_TO_UCS2,
        UCS2_TO_TURKISH_ISO,
        TURKISH_WINDOWS_TO_UCS2,
        UCS2_TO_TURKISH_WINDOWS,
        WESTERN_ISO_TO_UCS2,
        UCS2_TO_WESTERN_ISO,
        ARMENIAN_ISO_TO_UCS2,
        UCS2_TO_ARMENIAN_ISO,
        THAI_WINDOWS_TO_UCS2,
        UCS2_TO_THAI_WINDOWS,
        VIETNAMESE_WINDOWS_TO_UCS2,
        UCS2_TO_VIETNAMESE_WINDOWS,
        KOI8_R_TO_UCS2,
        UCS2_TO_KOI8_R,
        TIS_620_TO_UCS2,
        UCS2_TO_TIS_620,
        WESTERN_WINDOWS_TO_UCS2,
        UCS2_TO_WESTERN_WINDOWS,

        /* Add new (8-bit) encodings above this line */
        BIG5_TO_UCS2,
        UCS2_TO_BIG5,
        GB2312_TO_UCS2,
        UCS2_TO_GB2312,
        HKSCS_TO_UCS2,
        UCS2_TO_HKSCS,
        SJIS_TO_UCS2,
        UCS2_TO_SJIS,
        GB18030_TO_UCS2,
        UCS2_TO_GB18030,
        UTF7_TO_UCS2,
        UCS2_TO_UTF7,
        EUCKR_WINDOWS_TO_UCS2,
        UCS2_TO_EUCKR_WINDOWS,

        /* Place all CJK encodings above this one */
        UTF16LE_TO_UCS2,
        UCS2_TO_UTF16LE,
        UTF16BE_TO_UCS2,
        UCS2_TO_UTF16BE,
        UTF8_TO_UCS2,
        UCS2_TO_UTF8,
        CHSET_PAIR_TOTAL
        } chset_pair_enum;


    typedef enum {
        BIG_5_ENCODING_TYPE,
        GB2312_ENCODING_TYPE,
        HKSCS_ENCODING_TYPE,
        NO_OF_TEXT_ENCODING
        } TextEncodingEnum;


    typedef struct {
        short           start;
        short           end;
        } key_index_t;


    /* 
     * dual 8bits string operations
     */
    int dchar_strlen(const char * wstr);
    int dchar_strcpy(char * des,int des_len,const char * src); // des_len 
    int dchar_strncpy(char * des,int des_len,const char * src,int size);
    int dchar_strcat(char * des,int des_len,const char * src); // des_len 
    int dchar_nmemcpy(char * des,int des_len,const char * src,int src_len); // len: byte len
    int dchar_strcmp(const char * des,const char * src);
    int dchar_strncmp(const char * des,const char * src,int size);
    int unicode_to_ucs2encoding(unsigned short unicode,unsigned char * charLength,unsigned char * arrOut);
    int ucs2encoding_to_unicode(unsigned short * pUnicode,unsigned char * arr);

    /* 
     * UTF-8
     */
    int chset_utf8_strlen(const unsigned char * s);
    int chset_utf8_to_ucs2(unsigned char * dest,unsigned char * utf8);
    int chset_ucs2_to_utf8(unsigned char * utf8,unsigned short ucs2);
    int chset_utf8_to_ucs2_string_ex(unsigned char * dest,int dest_size,unsigned char * src,
        unsigned int * src_end_pos);
    int chset_utf8_to_ucs2_string(unsigned char * dest,int dest_size,unsigned char * src);
    int chset_ucs2_to_utf8_string_ex(unsigned char * dest,int dest_size,unsigned char * src,
        unsigned int * src_end_pos);
    int chset_ucs2_to_utf8_string(unsigned char * dest,int dest_size,unsigned char * src);
    int chset_ucs2_to_utf8_length_in_bytes(const unsigned char * raw);
    int chset_utf8_to_ucs2_length_in_bytes(const unsigned char * raw);

    /* 
     * UTF-7 or BASE64
     */
    void chset_utf_ucs2_chars(unsigned char * output_chars,unsigned char * current_input_chars,
        unsigned char prev_input_char,int combination_no,int * output_len);
    int chset_utf7_to_ucs2_str(unsigned char * g_unicode_decoded_buf,unsigned char * g_encoded_buf,int data_size,
        unsigned int * src_end_pos);
    void chset_ucs2_utf_chars(unsigned char * output_chars,int * output_len,unsigned short current_char,
        unsigned short prev_char,int no_of_chars,int encode_flag,int data_size);
    int chset_ucs2_to_utf7_str(unsigned char * g_encoded_buf,unsigned char * g_data_buf,int data_size,
        unsigned int * src_end_pos);

    /* 
     * Ascii
     */
    int chset_ucs2_to_ascii(unsigned char * pOutBuffer,unsigned char * pInBuffer,int dest_size,
        unsigned int * src_end_pos);
    int chset_ascii_to_ucs2(unsigned char * pOutBuffer,unsigned char * pInBuffer,int dest_size,
        unsigned int * src_end_pos);

    /* 
     * convert
     */
    // init
    void chset_init(void);

    // ucs2 => dest_type
    int chset_convert_from_native_ucs2(chset_enum dest_type,unsigned char * src_buff,unsigned char * dest_buff,
        int         dest_size,unsigned int * src_end_pos);

    // src_type => ucs2
    int chset_convert_to_native_ucs2(chset_enum src_type,unsigned char * src_buff,unsigned char * dest_buff,
        int         dest_size,unsigned int * src_end_pos);

    int mmi_asc_to_ucs2(signed char * pOutBuffer,signed char * pInBuffer);

    void url_decode(char buf[]);

    unsigned short chset_ucs2_to_text_str_ex(unsigned char * dest,int dest_size,unsigned char * src,
        TextEncodingEnum encoding,unsigned int * src_end_pos);


#ifdef __cplusplus
}


#endif

#endif


