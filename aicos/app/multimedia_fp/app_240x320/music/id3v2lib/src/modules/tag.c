/*
 * This file is part of id3v2lib library
 *
 * Copyright (c) Lars Ruiz
 *
 * For the full copyright and license information, please view the LICENSE
 * file that was distributed with this source code.
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "frames/apic_frame.private.h"
#include "frames/comment_frame.private.h"
#include "frames/text_frame.private.h"
#include "modules/char_stream.private.h"
#include "modules/frame.private.h"
#include "modules/frame_header.private.h"
#include "modules/frame_ids.h"
#include "modules/frame_list.private.h"
#include "modules/picture_types.h"
#include "modules/tag_header.private.h"
#include "modules/utils.private.h"

#include "tag.private.h"

ID3v2_Tag* ID3v2_Tag_new(ID3v2_TagHeader* header, const int padding_size)
{
    ID3v2_Tag* tag = (ID3v2_Tag*) malloc(sizeof(ID3v2_Tag));
    tag->header = header == NULL ? TagHeader_new_empty() : header;
    tag->frames = FrameList_new();
    tag->padding_size = 0;

    return tag;
}

ID3v2_Tag* ID3v2_Tag_new_empty()
{
    return ID3v2_Tag_new(NULL, 0);
}

ID3v2_Tag* Tag_parse(CharStream* tag_cs)
{
    ID3v2_TagHeader* header = TagHeader_parse(tag_cs);
    if (header == NULL) return NULL;

    ID3v2_Tag* tag = ID3v2_Tag_new(header, 0);

    if (header->extended_header_size > 0)
    {
        // An extended header exists, skip it
        CharStream_seek(tag_cs, header->extended_header_size, SEEK_SET);
    }

    ID3v2_Frame* current_frame;
    while (tag_cs->cursor < tag->header->tag_size)
    {
        current_frame = Frame_parse(tag_cs, tag->header->major_version);
        if (current_frame == NULL) break;
        FrameList_add_frame(tag->frames, current_frame);
    }

    tag->padding_size = tag_cs->size - tag_cs->cursor;

    return tag;
}

/**
 * Getter functions
 */
ID3v2_Frame* ID3v2_Tag_get_frame(ID3v2_Tag* tag, char* frame_id)
{
    if (tag == NULL) return NULL;
    return FrameList_get_frame_by_id(tag->frames, frame_id);
}

ID3v2_FrameList* ID3v2_Tag_get_frames(ID3v2_Tag* tag, char* frame_id)
{
    if (tag == NULL) return NULL;
    return FrameList_get_frames_by_id(tag->frames, frame_id);
}

ID3v2_TextFrame* ID3v2_Tag_get_artist_frame(ID3v2_Tag* tag)
{
    return (ID3v2_TextFrame*) ID3v2_Tag_get_frame(tag, ID3v2_ARTIST_FRAME_ID);
}

ID3v2_TextFrame* ID3v2_Tag_get_album_frame(ID3v2_Tag* tag)
{
    return (ID3v2_TextFrame*) ID3v2_Tag_get_frame(tag, ID3v2_ALBUM_FRAME_ID);
}

ID3v2_TextFrame* ID3v2_Tag_get_title_frame(ID3v2_Tag* tag)
{
    return (ID3v2_TextFrame*) ID3v2_Tag_get_frame(tag, ID3v2_TITLE_FRAME_ID);
}

ID3v2_TextFrame* ID3v2_Tag_get_track_frame(ID3v2_Tag* tag)
{
    return (ID3v2_TextFrame*) ID3v2_Tag_get_frame(tag, ID3v2_TRACK_FRAME_ID);
}

ID3v2_TextFrame* ID3v2_Tag_get_album_artist_frame(ID3v2_Tag* tag)
{
    return (ID3v2_TextFrame*) ID3v2_Tag_get_frame(tag, ID3v2_ALBUM_ARTIST_FRAME_ID);
}

ID3v2_TextFrame* ID3v2_Tag_get_genre_frame(ID3v2_Tag* tag)
{
    return (ID3v2_TextFrame*) ID3v2_Tag_get_frame(tag, ID3v2_GENRE_FRAME_ID);
}

ID3v2_TextFrame* ID3v2_Tag_get_year_frame(ID3v2_Tag* tag)
{
    return (ID3v2_TextFrame*) ID3v2_Tag_get_frame(tag, ID3v2_YEAR_FRAME_ID);
}

ID3v2_TextFrame* ID3v2_Tag_get_disc_number_frame(ID3v2_Tag* tag)
{
    return (ID3v2_TextFrame*) ID3v2_Tag_get_frame(tag, ID3v2_DISC_NUMBER_FRAME_ID);
}

ID3v2_TextFrame* ID3v2_Tag_get_composer_frame(ID3v2_Tag* tag)
{
    return (ID3v2_TextFrame*) ID3v2_Tag_get_frame(tag, ID3v2_COMPOSER_FRAME_ID);
}

/**
 * Since the ID3v2 spec allows defining multiple COMM frames in a
 * single tag, this gets the first COMM frame found. To retrieve every
 * COMM frame found use ID3v2_Tag_get_comment_frames(tag) instead.
 */
ID3v2_CommentFrame* ID3v2_Tag_get_comment_frame(ID3v2_Tag* tag)
{
    return (ID3v2_CommentFrame*) ID3v2_Tag_get_frame(tag, ID3v2_COMMENT_FRAME_ID);
}

/**
 * Since the ID3v2 spec allows defining multiple COMM frames in a
 * single tag, this gets every COMM frame found. To retrieve only the
 * first COMM frame found use ID3v2_Tag_get_comment_frame(tag) instead.
 */
ID3v2_FrameList* ID3v2_Tag_get_comment_frames(ID3v2_Tag* tag)
{
    return ID3v2_Tag_get_frames(tag, ID3v2_COMMENT_FRAME_ID);
}

ID3v2_ApicFrame* ID3v2_Tag_get_album_cover_frame(ID3v2_Tag* tag)
{
    return (ID3v2_ApicFrame*) ID3v2_Tag_get_frame(tag, ID3v2_ALBUM_COVER_FRAME_ID);
}

ID3v2_FrameList* ID3v2_Tag_get_apic_frames(ID3v2_Tag* tag)
{
    return ID3v2_Tag_get_frames(tag, ID3v2_ALBUM_COVER_FRAME_ID);
}


void ID3v2_Tag_free(ID3v2_Tag* tag)
{
    TagHeader_free(tag->header);
    ID3v2_FrameList_free(tag->frames);
    free(tag);
}
