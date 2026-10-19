# General configuration options
#
# Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.

aic_library_sources_ifdef(
	CONFIG_DCXO_DIAG_CALI_AP
        dcxo_cali_ap.c
)

aic_library_sources_ifdef(
	CONFIG_DCXO_DIAG_CALI_CP
        dcxo_cali_cp.c
)

aic_library_sources_ifdef(
        CONFIG_RCO32K_CALI
        rco32k_cali.c
)
