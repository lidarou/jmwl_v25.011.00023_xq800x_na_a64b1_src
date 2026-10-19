SSV_HOST_DIR := $(WIFI_DIR)/host
SSV_LIB_DIR  := $(WIFI_DIR)/host/lib
SSV_LWIP_DIR := $(SSV_HOST_DIR)/tcpip/lwip-1.4.0

SSV_INC_DIR := \
	$(SSV_HOST_DIR) \
	$(SSV_HOST_DIR)/hal \
	$(SSV_HOST_DIR)/hal/ssv6020 \
	$(SSV_HOST_DIR)/hal/ssv6020/firmware \
	$(SSV_HOST_DIR)/hal/ssv6020/regs \
	$(SSV_HOST_DIR)/include \
	$(SSV_HOST_DIR)/include/priv \
	$(SSV_HOST_DIR)/include/priv/hw \
	$(SSV_HOST_DIR)/app \
	$(SSV_HOST_DIR)/app/cli \
	$(SSV_HOST_DIR)/app/cli/cmds \
	$(SSV_HOST_DIR)/app/netapp \
	$(SSV_HOST_DIR)/app/netapp/udhcp \
	$(SSV_HOST_DIR)/app/netmgr \
	$(SSV_HOST_DIR)/app/netmgr/SmartConfig \
	$(SSV_HOST_DIR)/drv \
	$(SSV_HOST_DIR)/mlme \
	$(SSV_HOST_DIR)/mlme/ap \
	$(SSV_HOST_DIR)/mlme/ap/common \
	$(SSV_HOST_DIR)/mlme/sta \
	$(SSV_HOST_DIR)/mlme/wpa_supplicant/ \
	$(SSV_HOST_DIR)/mlme/wpa_supplicant/eap_common \
	$(SSV_HOST_DIR)/mlme/wpa_supplicant/eap_peer \
	$(SSV_HOST_DIR)/mlme/wpa_supplicant/eap_server \
	$(SSV_HOST_DIR)/mlme/wpa_supplicant/eapol_auth \
	$(SSV_HOST_DIR)/mlme/wpa_supplicant/tls \
	$(SSV_HOST_DIR)/mlme/wpa_supplicant/utils \
	$(SSV_HOST_DIR)/mlme/wpa_supplicant/wps \
	$(SSV_HOST_DIR)/core

ifeq ($(TCPIP_WRAPPER),icomm_lwIP)
SSV_INC_DIR += \
	$(SSV_HOST_DIR)/netstack_wrapper/$(TCPIP_WRAPPER) \
	$(SSV_LWIP_DIR)/ports/icomm/include \
	$(SSV_LWIP_DIR)/src/include \
	$(SSV_LWIP_DIR)/src/include/lwip \
	$(SSV_LWIP_DIR)/src/include/ipv4 \
	$(SSV_LWIP_DIR)/ports/icomm/include/arch
else
SSV_INC_DIR += 	$(SSV_HOST_DIR)/netstack_wrapper/$(TCPIP_WRAPPER) 
endif


SSV_INC_DIR += $(SSV_HOST_DIR)/os_wrapper/$(OS_WRAPPER)
SSV_INC_DIR += $(SSV_HOST_DIR)/platform/$(CUSTOMER_ID)
SSV_INC_DIR += $(SSV_HOST_DIR)/drv/$(DRV_INTERFACE)

SSV_INC_DIR := $(foreach ssvincdir,$(SSV_INC_DIR),$(addprefix -I,$(ssvincdir)))

