#ifndef __MY_GPS_H__
#define __MY_GPS_H__

// PIN19 G3 function6 for GPS_PWR_EN, GPIO26
#define MY_GPS_POWER           GPIO_26  //GPS POWER控制引脚
#define MAX_GPS_LENGTH         256          // GPS信息最长的一行长度
#define GPS_RECV_BUF_MAX_LEN   1024

// 115200波特率要求readBuf至少2KB，否则数据会被覆盖丢失
// 9600波特率要求readBuf至少1KB，否则数据会被覆盖丢失
#define GPS_READ_SIZE       2048

// AT6558R
#define AT6558R_GBS         "$PCAS04,3*1A\r\n" //双模
#define AT6558R_GPS         "$PCAS04,1*18\r\n" //单GPS
#define AT6558R_BDS         "$PCAS04,2*1B\r\n" //单BDS
#define AT6558R_HOTBOOT     "$PCAS10,0*1C\r\n" //热启动
#define AT6558R_COLDBOOT    "$PCAS10,2*1E\r\n" //冷启动

// CC1161W
#define GPS_TYPE_NAME       "CC1161W" //芯与物GPS定位芯片

#define CC1161W_HOTBOOT     "$RESET,0,h00\r\n" //热启动
#define CC1161W_WARMBOOT    "$RESET,0,h01\r\n" //温启动
#define CC1161W_COLDBOOT    "$RESET,0,hFF\r\n" //冷启动

// 与定位相关的全局变量
typedef struct{
    char szUTC[20];
    char szDate[20];

    char szValid[3];
    char szNavState[3];     // 导航状态标志
    char szPosState[3];     // 定位状态标志

    char szLatitude[20];    // 纬度
    char szNS[3];           // 北纬或南纬
    char szLongitude[20];   // 经度
    char szEW[3];           // 东经或西经

    char szAltitude[20];     // 海拔
    char szSpeed[10];        // 地面速度
    char szAngle[10];        // 地面航向

    char szLock[3];          // 锁定位星个数
    char szSee[3];           // 可视位星个数

    char szStrong[12][8];    // 信号强度

    char szPdop[10];        //位置精度因子
    char szHdop[10];        //水平精度因子
}T_GPSInfo;


void my_gps_task(void *argv);
void my_gpsrecv_task(void *argv);

void my_gps_uart_gpio_init(void);
void my_gps_uart_gpio_deinit(void);

void my_gps_uart_init(void);
void my_gps_uart_deinit(void);

void OpenGPSIO(void);
void CloseGPSPower( void );
void my_gps_power_io_on_off(bool on);


uint16 Gps_puthex_ex(uint8 *pData, uint16 iLen);

extern bool              g_bGpsDebug;
extern bool              g_bPosValid;
extern T_GPSInfo         g_gps;
extern T_GPSInfo         g_bds;

extern bool              g_bGpsDebug;
extern bool              g_bReciveGpsData;
extern bool              g_bReciveGsv;
extern bool              g_bGPSPower;

#endif

