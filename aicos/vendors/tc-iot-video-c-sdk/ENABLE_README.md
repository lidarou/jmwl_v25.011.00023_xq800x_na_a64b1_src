# TC_IOT enable 指导手册


## 一、设备信息设置
### 1. 设备信息获取和修改
    设备信息是 tc_iot 正常运行的必要前提
   - 咨询腾讯获取设备三元组信息(产品ID，设备名称，设备密钥)
   - 咨询腾讯获取 VoIP 功能需要的 device_id

    信息获取后，打开文件夹 aicos\vendors\tc-iot-video-c-sdk\samples\iot_video_ipc\device_info 填入对应信息
   - 产品ID：     device_info.json -> productId
   - 设备名称： device_info.json -> deviceName
   - 设备密钥： device_info.json -> deviceSecret
   - device_id:   voip\device_key  -> device_id
   - 设备密钥： voip\device_key  -> device_key

### 2. VoIP 调试参数获取
    通过腾讯 VoIP 测试小程序 P2P player，获取 Model ID，wxa_appid, sn ticket 和 open id，具体方法可参考腾讯提供的小程序使用文档
    对应信息需要在调用 iv_avt_voip_init 和 iv_avt_voip_call 时以参数形式填入。


## 二、enable 配置 -- 当前使用 gx316h_h2 工程
### 1. enable TC_IOT_SDK
    运行 ./make ap kuconfig
   - 打开 Vendor Compnent -> TC IOT VIDEO

### 2. enable tc_iot_sample
    编辑 aicos\aicos\vendors\tc-iot-video-c-sdk  修改  set(QCLOUD_VIDEO_COMPILE_SAMPLE "ON" )


## 三、enable 配置 -- 当前使用其他工程
### 1. enable TC_IOT_SDK
   运行 ./make ap kuconfig
   - 打开 Vendor Compnent -> TC IOT VIDEO
   - 打开 File System Config -> File System (FS) support -> FAT File System support -> Generate FATFS image at build time
   - 打开 File System Config -> File System Config->Romfs File System
   - 打开 File System Config -> File System Config->File System Device Select->flash device fs support

### 2. enable tc_iot_sample
    打开 aicos\aicos\vendors\tc-iot-video-c-sdk  设置  set(QCLOUD_VIDEO_COMPILE_SAMPLE "ON" )

### 3. 对应 project 配置修改
   - 对比修改当前工程与 aicos\projects\genesis\ap\gx316h_h2\config\mbedtls_config.h 和 fs_config.h
   - 对比修改当前工程与 aicos\targets\genesis\gx316h_h2 中  pkg_cfg.json 和 fmd_partition_table.json