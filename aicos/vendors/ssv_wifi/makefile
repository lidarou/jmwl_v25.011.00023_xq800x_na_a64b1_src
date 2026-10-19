#
# Makefile for the iComm wifi host sw stack.
#
include $(TOP_DIR)/aicos/components/wifi/ssv_wifi_all/config.mk
include $(TOP_DIR)/aicos/components/wifi/ssv_wifi_all/include.mk

SSV_LIB      := lib_ssv_wifi.a

SSV_LIB_FILE := wifi_lib_pub.a
SSV_LIB_CORE := wifi_lib_core.a

ifeq ($(TCPIP_WRAPPER),icomm_lwIP)
SSV_LIB_IPERF:= wifi_lib_iperf3.a
else
SSV_LIB_IPERF:= wifi_lib_iperf3.a
endif

COMMON_OBJS = 
COMMON_LIBS =

include $(TOP_DIR)/aicos/components/wifi/ssv_wifi_all/open.mk

COMMON_OBJS += $(SSV_COBJS)
COMMON_OBJS += wifi.o
#COMMON_OBJS += novatek_wifi.o
COMMON_LIBS += $(addprefix $(WIFI_OUTDIR)/,$(SSV_LIB_FILE))

COMMON_OBJS :=$(subst $(SSV_HOST_DIR),.,$(COMMON_OBJS))
COMMON_DIRS = $(subst $(SSV_HOST_DIR),.,$(SSV_SRC_DIRS))

OUTPUT_OBJS := $(addprefix $(WIFI_OUTDIR)/,$(COMMON_OBJS))  

OUTPUT_DEPS:= $(OUTPUT_OBJS:.o=.d)
 
ALL_OUTDIR := $(WIFI_OUTDIR) 
ALL_OUTDIR += $(addprefix $(WIFI_OUTDIR)/,$(COMMON_DIRS))  

vpath %.c $(SSV_HOST_DIR):$(SSV_HOST_DIR)/..

.PHONY:all
ifeq ($(TCPIP_WRAPPER),icomm_lwIP)
all:$(COMMON_LIBS) $(SSV_LIB_DIR)/$(SSV_LIB_CORE) $(SSV_LIB_DIR)/$(SSV_LIB_IPERF)
	$(AR) -xv $(SSV_LIB_DIR)/$(SSV_LIB_CORE)
	$(AR) -xv $(SSV_LIB_DIR)/$(SSV_LIB_IPERF)
	$(AR) -xv $(COMMON_LIBS)
	$(AR) -rcs $(WIFI_OUTDIR)/$(SSV_LIB) *.o
	$(COPYTO) $(WIFI_OUTDIR)/$(SSV_LIB) $(WIFI_LIB)
	@$(RM) *.o
else
all:$(COMMON_LIBS) $(SSV_LIB_DIR)/$(SSV_LIB_CORE)
	$(AR) -xv $(SSV_LIB_DIR)/$(SSV_LIB_CORE)
#	$(AR) -xv $(SSV_LIB_DIR)/$(SSV_LIB_IPERF)
	$(AR) -xv $(COMMON_LIBS)
	$(AR) -rcs $(WIFI_OUTDIR)/$(SSV_LIB) *.o
	$(COPYTO) $(WIFI_OUTDIR)/$(SSV_LIB) $(WIFI_LIB)
	@$(RM) *.o
endif

$(COMMON_LIBS):$(OUTPUT_OBJS)
	@echo  make $(SSV_LIB_DIR)/$(SSV_LIB_FILE) 
	@$(AR) -rcs $@ $(OUTPUT_OBJS)
	
$(OUTPUT_OBJS):$(WIFI_OUTDIR)/%.o:%.c
	@$(TESTDIR) -d $(WIFI_OUTDIR)/$(dir $*) || $(MKDIR) -p $(WIFI_OUTDIR)/$(dir $*)
	@echo compile $(notdir $<)
	@echo -e "compile $(notdir $<)\r\n" >>output.txt
	@$(CC) $(CFLAGS) $(SSV_INC_DIR) $(SSV_INC_DIR_EXT) -c $< -o $@ 


ifneq "$(MAKECMDGOALS)" "clean"
#sinclude $(OUTPUT_DEPS)

$(WIFI_OUTDIR)/%.d:%.c
	@test -d $(WIFI_OUTDIR)/$(dir $*) || $(MKDIR) -p $(WIFI_OUTDIR)/$(dir $*)
	@echo "dep "$(notdir $*).d
	@set -e;$(MAKEDEPS) $(CFLAGS) $(SSV_INC_DIR) $(SSV_INC_DIR_EXT) $< > $@.temp; \
	sed 's,\($(notdir $*)\)\.o[ :]*,$(WIFI_OUTDIR)/$(dir $*)\1.o $@ : ,g' < $@.temp > $@.temp1;\
	sed /ADSv1_2/d $@.temp1 > $@;\
	$(RM) -f $@.temp*
endif		

.PHONY:clean    
clean:
	@$(RM) $(COMMON_LIBS) $(OUTPUT_OBJS) $(OUTPUT_DEPS)
	@$(RM) $(SSV_LIB_DIR)/$(SSV_LIB)
	@$(RM) $(WIFI_OUTDIR)
