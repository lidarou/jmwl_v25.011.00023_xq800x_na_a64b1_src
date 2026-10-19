#include <sys_mem_config.h>

MEMORY
{
    VECTOR_REGION(rx) : ORIGIN = VECTOR_REGION_BASE, LENGTH = VECTOR_REGION_SIZE
    STACK_REGION(rw) : ORIGIN = ALL_STACK_BASE, LENGTH = ALL_STACK_SIZE
    FLASH_BASIC (rx) : ORIGIN = FLASH1_AP_BASE, LENGTH = FLASH1_AP_SIZE
    FLASH_EXTEND (rx) : ORIGIN = FLASH1_AP_EXTEND_BASE, LENGTH = FLASH1_AP_EXTEND_SIZE
    PSRAM_RO (rx) : ORIGIN = CODE_REGION_BASE, LENGTH = CODE_REGION_SIZE
    PSRAM_RW (rw) : ORIGIN = RW_REGION_BASE, LENGTH = RW_REGION_SIZE
    PSRAM_HEAP (rw) : ORIGIN = HEAP_REGION_BASE, LENGTH = HEAP_REGION_SIZE
    MTB_REGION (rw) : ORIGIN = 0x54000600, LENGTH = 0x200
}

SECTIONS
{
    .basic_init :
    {
        __init_start__ = .;
        *(.init .init.*)
        __init_end__ = .;
    } > FLASH_BASIC

    .basic_text :
    {
        __basic_text_start__ = .;
        *(.glue_7t .glue_7 .gnu.linkonce.t.* .gcc_except_table .gnu.sgstubs .rel.*)
        *\libnosys.a:* (.text .text.*)
        *\libm.a:*(.text .text.*)
        *\libgcc.a:*(.text .text.*)
        *\libc_nano.a:*(.text .text.*)
        *\libc.a:*(.text .text.*)
        *\libaic_osa.a:*(.text .text.*)

        EXCLUDE_FILE(*_at.*)
        *\libaic_fs.a:*(.text .text.*)

        *\liblos_tlsf.a:*(.text .text.*)
        *\libthreadx.a:*(.text .text.*)
        *\liblevelx.a:*(.text .text.*)
        *\libfilex.a:*(.text .text.*)
        *\libnetxduo.a:*(.basic_net)

        EXCLUDE_FILE(nxd_dhcp_server.* nxd_dhcpv6_server.* nx_arp_*.* nxe_arp_*.*
                     nx_nat.* nx_ppp.* nx_bsd.*)
        *\libnetxduo.a:*(.text .text.*)

        *\libmbedcrypto.a:*(.text .text.*)
        *\libmbedx509.a:*(.text .text.*)
        *\libmbedtls.a:*(.text .text.*)
        *\liblvgl.a:*(.text .text.*)
        *\libboard.a:*(.text .text.*)

        EXCLUDE_FILE(snor.* gigadevice.* winbond.* xmc.* puya.* ux_dcd* usb*
                     aic_ppp_codec_core.* ppp_codec_reg.*)
        *\libdrivers.a:*(.text .text.*)

        EXCLUDE_FILE(qspi.* psram.* sleep_manage.* ap_slp.* wake_lock.*
                     busmon_* dump_addr.* breakpoint.* perf_*)
        *\libcs_drivers.a:*(.text .text.*)

        EXCLUDE_FILE(aic_cmd_exe.* aic_console.* imei_diag.* imei_at.* mac.*
                     sn.* aic_cali_data_cmd.* aic_factory_cmd.* aic_at_tls.*
                     aic_http_at_process.* aic_https_at_process.* aic_at_ping.*
                     aic_at_sntp.* aic_at_tcpip_main.* aic_at_tcpip_common.*)
        *\libaic_components.a:*(.text .text.*)

        EXCLUDE_FILE(aic_mcx.* alog_store_mgr.* aic_network_pcap.*)
        *\libcs_components.a:*(.text .text.*)

        /*
         * customer can add basic bsp driver code here, like below:
         * lcd_1602_driver*(.text .text.*)
         */

        __basic_text_end__ = .;
    } > FLASH_BASIC

    .basic_rodata :
    {
        __basic_rodata_start__ = .;
        *(.gnu.linkonce.r.*)
        *\libnosys.a:*(.rodata .rodata.*)
        *\libm.a:*(.rodata .rodata.*)
        *\libgcc.a:*(.rodata .rodata.*)
        *\libc_nano.a:*(.rodata .rodata.*)
        *\libc.a:*(.rodata .rodata.*)
        *\libaic_osa.a:*(.rodata .rodata.*)

        EXCLUDE_FILE(*_at.*)
        *\libaic_fs.a:*(.rodata .rodata.*)

        *\liblos_tlsf.a:*(.rodata .rodata.*)
        *\libthreadx.a:*(.rodata .rodata.*)
        *\liblevelx.a:*(.rodata .rodata.*)
        *\libfilex.a:*(.rodata .rodata.*)
        *\libnetxduo.a:*(.basic_net)

        EXCLUDE_FILE(nxd_dhcp_server.* nxd_dhcpv6_server.* nx_arp_*.* nxe_arp_*.*
                     nx_nat.* nx_ppp.* nx_bsd.*)
        *\libnetxduo.a:*(.rodata .rodata.*)

        *\libmbedcrypto.a:*(.rodata .rodata.*)
        *\libmbedx509.a:*(.rodata .rodata.*)
        *\libmbedtls.a:*(.rodata .rodata.*)
        *\liblvgl.a:*(.rodata .rodata.*)
        *\libboard.a:*(.rodata .rodata.*)

        EXCLUDE_FILE(snor.* gigadevice.* winbond.* xmc.* puya.* ux_dcd* usb*
                     aic_ppp_codec_core.* ppp_codec_reg.*)
        *\libdrivers.a:*(.rodata .rodata.*)

        EXCLUDE_FILE(qspi.* psram.* sleep_manage.* ap_slp.* wake_lock.*
                     busmon_* dump_addr.* breakpoint.* perf_*)
        *\libcs_drivers.a:*(.rodata .rodata.*)

        EXCLUDE_FILE(aic_cmd_exe.* aic_console.* imei_diag.* imei_at.* mac.*
                     sn.* aic_cali_data_cmd.* aic_factory_cmd.*  aic_at_tls.*
                     aic_http_at_process.* aic_https_at_process.* aic_at_ping.*
                     aic_at_sntp.* aic_at_tcpip_main.* aic_at_tcpip_common.*)
        *\libaic_components.a:*(.rodata .rodata.*)

        EXCLUDE_FILE(aic_mcx.* alog_store_mgr.* aic_network_pcap.*)
        *\libcs_components.a:*(.rodata .rodata.*)

        /*
         * customer can add basic bsp driver code here, like below:
         * lcd_1602_driver*(.rodata .rodata.*)
         */

        KEEP (*(SORT(.auto_init*)))
        KEEP (*(SORT(.aiccmd*)))
        __basic_rodata_end__ = .;
    } > FLASH_BASIC

    .basic_ARM.exidx :
    {
        __exidx_start = .;
        *(.ARM.exidx* .gnu.linkonce.armexidx.*)
        __exidx_end = .;
    } > FLASH_BASIC

    __vec_load_start__ = ALIGN(__exidx_end, 4);
    .basic_vectors : AT(__vec_load_start__)
    {
        __vectors_start__ = .;
        *(.vectors .vectors.*)
        __vectors_end__ = .;
    } > VECTOR_REGION
    __vec_load_end__ = __vec_load_start__ + SIZEOF(.basic_vectors);

    __idle_load_start__ = ALIGN(__vec_load_end__, 4);
    .basic_idle : AT(__idle_load_start__)
    {
        __idle_start__ = .;
        *(.idle_text .idle_text.* .idle_data .idle_data.*)
        psram.* (.text .text.* .rodata .rodata.* .data .data.*)
        * (.qspi_code .qspi_rwdata)
        __idle_end__ = .;
    } > VECTOR_REGION
    __idle_load_end__ = __idle_load_start__ + SIZEOF(.basic_idle);

    __mtb_load_start__ = ALIGN(__idle_load_end__, 4);
    .basic_mtb : AT(__mtb_load_start__)
    {
        __mtb_start__ = .;
        * (.mtb_code)
        __mtb_end__ = .;
    } > MTB_REGION
    __mtb_load_end__ = __mtb_load_start__ + SIZEOF(.basic_mtb);

    __psram_text_load_start__ = ALIGN(__mtb_load_end__ , 4);
    .basic_psram_text : AT(__psram_text_load_start__)
    {
        __psram_text_start__ = .;
        snor* (.text .text.*)
        qspi.* (.text .text.*)
        gigadevice.* (.text .text.*)
        winbond.* (.text .text.*)
        xmc.* (.text .text.*)
        puya.* (.text .text.*)
        aic_mcx.* (.text .text.*)
        * (.mpu_text)
        sleep_manage.* (.text .text.*)
        ap_slp.*  (.text .text.*)
        wake_lock.*  (.text .text.*)
        * (.timer_code)
        * (.spinlock_code)
        __psram_text_end__ = .;
    } > PSRAM_RO
    __psram_text_load_end__ = __psram_text_load_start__ + SIZEOF(.basic_psram_text);

    __psram_rodata_load_start__ = ALIGN(__psram_text_load_end__ , 4);
    .basic_psram_rodata : AT(__psram_rodata_load_start__)
    {
        __psram_rodata_start__ = .;
        snor* (.rodata .rodata.*)
        qspi.* (.rodata .rodata.*)
        gigadevice.* (.rodata .rodata.*)
        winbond.* (.rodata .rodata.*)
        xmc.* (.rodata .rodata.*)
        puya.* (.rodata .rodata.*)
        aic_mcx.* (.rodata .rodata.*)
        sleep_manage.* (.rodata .rodata.*)
        ap_slp.*  (.rodata .rodata.*)
        wake_lock.*  (.rodata .rodata.*)
        __psram_rodata_end__ = .;
    } > PSRAM_RO
    __psram_rodata_load_end__ = __psram_rodata_load_start__ + SIZEOF(.basic_psram_rodata);

    .basic_heap (NOLOAD) :
    {
        __heap_start__ = .;
        *(.sys_heap)
        __heap_end__ = ALIGN(4);
    } > PSRAM_HEAP

    __data_load_start__ = ALIGN(__psram_rodata_load_end__, 4);
    .basic_data : AT(__data_load_start__)
    {
        __data_start__ = .;
        *(.data .data.* *.data .gnu.linkonce.d.* .gnu.sgstubs)
        __data_end__ = .;
    } > PSRAM_RW
    __data_load_end__ = __data_load_start__ + SIZEOF(.basic_data);

    .basic_bss (NOLOAD) :
    {
        __bss_start__ = .;
        *(.trace_buffer .bss .bss.* .gnu.linkonce.b.*) *(COMMON)
        __bss_end__ = .;
    } > PSRAM_RW

    .basic_stack (NOLOAD) :
    {
        __stack_start__ = .;
        *(.sys_stack)
        __stack_end__ = ALIGN(4);
    } > STACK_REGION

    __RAM_segment_used_end__ = ALIGN(__bss_end__ , 4);
    __ROM_segment_used_end__ = __data_load_end__;
}

SECTIONS
{
    .extend_head :
    {
        __extend_text_start__ = .;
        KEEP (*(SORT(.extend_header)))
    } > FLASH_EXTEND

    .extend_text :
    {
        *(.text .text.* )
        __extend_text_end__ = .;
    } > FLASH_EXTEND

    .extend_rodata :
    {
        __extend_rodata_start__ = .;
       *(.rodata .rodata.*)
        __extend_rodata_end__ = .;
    } > FLASH_EXTEND
}

