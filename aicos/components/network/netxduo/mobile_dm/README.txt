
Steps to open DM function
1. Open MOBILE_DM config:
    Components Options → Network Configurations → Network Extension →
        Network Extended Function → Supports Mobile Terminal Management

2. Modify genesis/nvram/ap/common/config/MOBILE_DM.json, add "MOBILE_DM_CFG".
   Such as ：
{
    "file_path": "aicos/components/network/netxduo/mobile_dm/cfg/",
    "header_path": "aicos/components/network/netxduo/mobile_dm/cfg/inc",
    "custom": [
        "MOBILE_DM_CFG"
    ]
}