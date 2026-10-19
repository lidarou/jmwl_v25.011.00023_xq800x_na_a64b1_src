/*
 * This file is part of id3v2lib library
 *
 * Copyright (c) Lars Ruiz
 *
 * For the full copyright and license information, please view the LICENSE
 * file that was distributed with this source code.
 */

#include <stdio.h>
#include <stdlib.h>

#include "modules/char_stream.private.h"
#include "modules/frame.private.h"
#include "modules/frame_list.private.h"
#include "modules/tag.private.h"
#include "modules/tag_header.private.h"
#include "modules/utils.private.h"

#include "id3v2lib.h"

ID3v2_TagHeader* ID3v2_read_tag_header(const char* file_name)
{
    FILE* fp = fopen(file_name, "rb");
    char tag_header_buffer[ID3v2_TAG_HEADER_LENGTH];

    if (fp == NULL) return NULL;

    const int bytes_read = fread(tag_header_buffer, sizeof(char), ID3v2_TAG_HEADER_LENGTH, fp);
    fclose(fp);

    if (bytes_read < ID3v2_TAG_HEADER_LENGTH) return NULL;

    return ID3v2_read_tag_header_from_buffer(tag_header_buffer);
}

ID3v2_TagHeader* ID3v2_read_tag_header_from_buffer(const char* buffer)
{
    CharStream* cs = CharStream_from_buffer(buffer, ID3v2_TAG_HEADER_LENGTH);
    ID3v2_TagHeader* header = TagHeader_parse(cs);
    CharStream_free(cs);
    return header;
}

ID3v2_Tag* ID3v2_read_tag(const char* file_name)
{
    ID3v2_TagHeader* tag_header = ID3v2_read_tag_header(file_name);

    if (tag_header == NULL) return NULL;

    FILE* file = fopen(file_name, "rb");
    int buffer_length = tag_header->tag_size + ID3v2_TAG_HEADER_LENGTH;
    char* tag_buffer = (char*) malloc((buffer_length) * sizeof(char));

    if (tag_buffer == NULL)
    {
        printf("Could not allocate buffer.");
        fclose(file);
        TagHeader_free(tag_header);
        return NULL;
    }

    fread(tag_buffer, buffer_length, 1, file);
    fclose(file);

    ID3v2_Tag* tag = ID3v2_read_tag_from_buffer(tag_buffer, buffer_length);

    free(tag_buffer);
    TagHeader_free(tag_header);

    return tag;
}

ID3v2_Tag* ID3v2_read_tag_from_buffer(const char* tag_buffer, const int buffer_length)
{
    CharStream* tag_cs = CharStream_from_buffer(tag_buffer, buffer_length);
    ID3v2_Tag* tag = Tag_parse(tag_cs);
    CharStream_free(tag_cs);
    return tag;
}