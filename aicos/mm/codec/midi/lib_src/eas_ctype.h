/*----------------------------------------------------------------------------
 *
 * File: 
 * eas_ctype.h
 *
 * Contents and purpose:
 * This is a replacement for the CRT ctype.h functions. These
 * functions are currently ASCII only, but eventually, we will want
 * to support wide-characters for localization.
 *
 * Copyright (C) Sonic Network, Inc. 2004-2011
 *
 *----------------------------------------------------------------------------
 * Revision Control:
 *   $Revision: 429 $
 *   $Date: 2006-10-20 02:50:15 -0400 (Fri, 20 Oct 2006) $
 *----------------------------------------------------------------------------
*/

#ifndef _EAS_CTYPE_H
#define _EAS_CTYPE_H

EAS_INLINE EAS_I8 IsDigit (EAS_I8 c) { return ((c >= '0') && (c <= '9')); }
EAS_INLINE EAS_I8 IsSpace (EAS_I8 c) { return (((c >= 9) && (c <= 13)) || (c == ' ')); }
EAS_INLINE EAS_I8 ToUpper (EAS_I8 c) { if ((c >= 'a') && (c <= 'z')) return c & ~0x20; else return c; }
EAS_INLINE EAS_I8 ToLower (EAS_I8 c) { if ((c >= 'A') && (c <= 'Z')) return c |= 0x20; else return c; }

#endif

