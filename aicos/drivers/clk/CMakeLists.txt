# General configuration options
#
# Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.

add_subdirectory_ifdef(CONFIG_AIC_IOT_CLK iot_clk)
add_subdirectory_ifdef(CONFIG_AIC_STANDARD_CLK std_clk)
