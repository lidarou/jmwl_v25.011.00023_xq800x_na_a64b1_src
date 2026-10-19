/*****************************************************************************/
/*                                                                           */
/*      Copyright 2021 by AIC Corporation. All rights reserved.              */
/*                                                                           */
/*****************************************************************************/

#ifndef __GNSS_SERVICE_H__
#define __GNSS_SERVICE_H__

#include "icoe_gps_types.h"

#ifdef CONFIG_ICOE_GPS_SRC
void gnss_service_enable_agnss(int enable);
void gnss_service_cb_register(GpsCallbacks* callbacks);
int gnss_service_get_local(GpsLocation* location);
int gnss_service_get_sv_status(GnssSvStatus* sv);
int gnss_service_get_sv_info(int *sv_num, int *snr);

int gnss_service_start(void);
int gnss_service_stop(void);
#else
inline void gnss_service_enable_agnss(int enable) {}
inline void gnss_service_cb_register(GpsCallbacks* callbacks) {}
inline int gnss_service_get_local(GpsLocation* location) {}
inline int gnss_service_get_sv_status(GnssSvStatus* sv) {}
inline int gnss_service_get_sv_info(int *sv_num, int *snr) {}

inline int gnss_service_start(void)  { return 0; }
inline int gnss_service_stop(void) { return 0; }

#endif /* CONFIG_ICOE_GPS_SRC */

#endif /* __GNSS_SERVICE_H__ */
