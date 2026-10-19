/**
 *  DESCRIPTION
 *
 *    This file defines romfs configurations.
 */

#ifndef __ROMFS_CONFIG_H__
#define __ROMFS_CONFIG_H__

enum FOTA_ROMFS_CONFIG_TYPE{
    FOTA_CONFIG_NONE,
    FOTA_CONFIG_ROMFS,
    FOTA_CONFIG_ROMFS_PART,
};

inline u32 romfs_config() {
    return FOTA_CONFIG_NONE;
}

#endif /* __ROMFS_CONFIG_H__ */

