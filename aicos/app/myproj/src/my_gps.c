/********************************************************************
**版权所有         深圳市几米物联有限公司
**文件名称:        my_gps.c
**当前版本:        V1.0
**作    者:        Nestor Zhang (zhangqingcan@jimiiot.com)
**完成日期:        2020年4月20日
*********************************************************************/
#include "my_common.h"
#include <math.h>

/*
 * 连续位置点求平均值的做法，对于正常行驶轨迹具有平滑
 * 经纬度或轨迹点有好处。但是会导致在急转弯或者大拐弯
 * 情况下行驶轨迹和真实轨迹偏差较大。
*/
#define POINT_N     1


/*
 * 从GPS开机以来，是否有收到过有效的GSV消息，
 * 7代的UBLOX 芯片在搜索不到卫星的情况下，会默认不打印GSV语句
 */
bool g_bReciveGsv = FALSE;
bool g_bReciveBDGsv = FALSE;
bool g_bReciveGpsData = FALSE;

bool g_bGpsDebug = FALSE;

// 实时的GPS是否定位标志位. g_gps有延时
bool g_bPosValid = FALSE;
bool g_bPosUp = FALSE;

// 全局变量，存储当前最新收到的一次GPS数据
T_GPSInfo  g_gps = {0};

// 全局变量，存储当前最新收到的一次BDS数据
T_GPSInfo  g_bds = {0};

bool g_bGPSPower = FALSE;


/*************************************************************
**函数名称:  OpenGPSIO
**入口参数:  无
**出口参数:  无
**函数功能:  打开GPS的底层IO操作
*************************************************************/
void OpenGPSIO(void)
{
    if (g_bGPSPower)
    {
        my_log_printf(1, "GPS POWER已经打开，不再重复打开");

        return ;
    }

    // Make GPS UART ready!
    my_gps_uart_init();

    //清除内存中的GPS数据控制结构
    memset(&g_gps, 0, sizeof(g_gps));
    g_bPosValid = FALSE;

    my_gps_power_io_on_off(1);

    // GPS电源开
    g_bGPSPower = TRUE;
}


/*************************************************************
**函数名称:  CloseGPSPower
**入口参数:  无
**出口参数:  无
**函数功能:  关闭GPS IO的底层操作函数
*************************************************************/
void CloseGPSPower(void)
{
    my_log_printf(1, "关闭GPS电源");

    my_gps_power_io_on_off(0);

    // MAIN UART不关闭的话系统无法进入低功耗休眠状态!
    // Nestor 2023-02-15
    if (g_bGPSPower)
    {
        // NOTE: 未打开GPS串口时调用关闭接口系统会复位!
        my_uart_close(MY_GPS_PORT);
        g_bGPSPower = FALSE;
    }
}


static void open_gps_power(void)
{
    // 打开GPS主供电
    OpenGPSIO();
}


/*************************************************************
**函数名称:  my_gps_uart_init
**入口参数:  无
**出口参数:  无
**函数功能:  GPS 所使用的串口的初始化操作
*************************************************************/
void my_gps_uart_init(void)
{
    MY_UART_ST_STRUCT  set;

    //my_gps_uart_gpio_init();

    set.baud = MY_UART_BAUD_115200;
    set.port = MY_GPS_PORT;

    my_uart_init(&set);
}

void my_gps_uart_deinit(void)
{
    //my_gps_uart_gpio_deinit();
}


/*************************************************************
**函数名称:  my_gps_power_io_on_off
**入口参数:  on   ---  开启或者是关闭GPS电源
             1    ---  打开GPS电源
             0    ---  关闭GPS电源
**出口参数:  无
**函数功能:  GPS  电源端口的打开与关闭操作
*************************************************************/
void my_gps_power_io_on_off(bool on)
{
    if (on)
    {
        my_log_printf(1, "###OPEN GPS POWER IO");
        JM_GpioSet(MY_GPS_POWER, 1);
    }
    else
    {
        my_log_printf(1, "###CLOSE GPS POWER IO");
        JM_GpioSet(MY_GPS_POWER, 0);
    }
}


/*************************************************************
**函数名称:  GetArgv
**入口参数:  szLine   ---  待解析的字符串
             iPos      ---  提取字段的位置
             szBuf    ---  存储提取出的字符串
             iBuffLen ---  存储数据最长的字符串
**出口参数:  无
**函数功能:  GPS数据字段提取函数
*************************************************************/
void GetArgv(char *szLine, uint16 iPos, char *szBuf, uint16 iBuffLen)
{
    uint16 i = 0;
    char   *c = szLine;
    char   *p = szBuf;

    if( szLine == NULL )
        return;

    while( *c != '\0' )
    {
        if(i == iPos &&  *c!=',')
        {
            *p++ = *c;

            // 防止溢出，后面还要往这个 szBuf[iBuffLen-1] 放入结尾符
            if( p >= (szBuf + iBuffLen - 1) )
                break;
        }
        else if(i > iPos)
        {
            break;
        }

        if( *c == ',')
        {
            i++;
        }

        c++;
    }

    *p = '\0';
}


/*************************************************************
**函数名称:  ConvertGPS
**入口参数:  szInGPS   ---  输入的GPS数据
             szOutGPS  ---  转换之后的GPS数据
**出口参数:  无
**函数功能:  把度分结构的位置转换成度结构
*************************************************************/
void ConvertGPS(char *szInGPS, char *szOutGPS, uint32 outSize)
{
    char szDu[10] = {0};
    char szMin[3] = {0};
    char szSec[10] = {0};
    double fSec;

    char szInt[10];
    char szFra[10];

    uint8 iLen;

    // 先初始化输出的数据
    szOutGPS[0] = '\0';

    get_str_to_ken_ex(szInGPS, 0, '.', szInt, 10 );   // 取整数
    get_str_to_ken_ex(szInGPS, 1, '.', szFra, 10 );   // 取小数

    // 取度,根据整数部分的长度有所区别
    iLen = strlen( szInt );
    if( iLen > 2 )   // 如果长度大于2，则说明有度部分，取多出的部分
        strncpy(szDu, szInt, iLen - 2);
    else             // 小于等于2时，度是为0
        strcpy( szDu, "0" );

    // 取分
    if( iLen > 2 )
        strncpy(szMin, &szInt[iLen - 2], 2);  // 整数部分后两位是分
    else
        strncpy(szMin, szInt, 2);             // 否则，只取现有值

    // 取秒
    strncpy(szSec,szFra, 4);   // 小数部分全是秒

    // 将分秒转化为度
    fSec = (atoi(szMin) * 10000 + atoi(szSec) ) / 600000.0;
    fSec = atoi(szDu) + fSec;

    //sprintf(szOutGPS, "%.6f", fSec);
    myftoa_ex(fSec, 6, szOutGPS, outSize);
}


// 算校验和
static uint8 getCheckSum(char *szLine, uint16 iLen)
{
    uint16 iRet = 0;

    while( iLen > 0 )
        iRet ^= szLine[--iLen];

    return iRet;
}


// $GNRMC,093546.000,A,2234.59408,N,11356.58489,E,0.00,0.00,130524,,,A,V*09
bool check_line(char *szLine)
{
    uint8 iSum, iOldSum;
    char *pEnd;
    char *pStart = &szLine[1];   // 跳过美元附号，这个前面验证过

    // 找星号所在位，不能没有
    pEnd = strstr(pStart, "*");
    if (pEnd == NULL)
        return FALSE;

    iSum = getCheckSum(pStart, pEnd - pStart);     // 美元起到星号，算出校验和
    iOldSum = strtol(pEnd+1, NULL, 16);      // *号后面的字符串转为十六进制

    if (iSum == iOldSum)                     // 校验和相等
        return TRUE;
    else
        return FALSE;
}


//这个函数修改了更新gps全局变量的条件
// 传入一行GPS数据，提取并显示中间的关键数据
void GPS_Parse(uint8 * szLine)
{
    static T_GPSInfo GpsInfo = {0};
    char *p = (char *)szLine;
    char szBuf[25] = { 0 };

    if (p == NULL)
    {
        return;
    }

    if (strlen(p) == 0)
    {
        return;
    }

    if (g_bGpsDebug)
    {
        my_log_printf_internal(0, "%s\r\n", szLine);
    }

    // 取出第0 个字段
    GetArgv(p, 0, szBuf, 25);

    if (strncmp(szBuf, "$GPGSV", strlen("$GPGSV")) == 0)
    {
        char szTmp[20] = { 0 };
        char szNum[10] = { 0 };
        char szValue[10] = { 0 };
        int numMsg = 0;
        int msgNum = 0;
        int numSV = 0;
        int i = 0, iCount = 0;
        static char gpsStrong[12][8];
        static int iPos= 0;

        g_bReciveGpsData = TRUE;

        GetArgv(p, 1, szNum, 10); // Number of messages
        numMsg = atoi(szNum);
        GetArgv(p, 2, szNum, 10); // Number of this message
        msgNum = atoi(szNum);
        GetArgv(p, 3, szNum, 10); // Number of satellites in view
        numSV = atoi(szNum);

        // 如果没有搜到任何卫星
        if (numSV == 0)
        {
            memset(GpsInfo.szStrong, 0, sizeof(GpsInfo.szStrong));
            g_bReciveGsv = FALSE;
            GpsInfo.szValid[0] = '0';
            g_gps.szValid[0] = '0';
            return;
        }
        g_bReciveGsv = TRUE;
        GetArgv(p, 3, GpsInfo.szSee, sizeof(GpsInfo.szSee)); //获取GPGSV 可视卫星总数

        if (msgNum == 1) // GSV 第一条语句
        {
            memset(gpsStrong, 0, sizeof(gpsStrong));
            iPos = 0;
        }

        if (numMsg > msgNum) {
            iCount = 4;
        } else { // numMsg == msgNum
            iCount = numSV - (numMsg - 1) * 4;
        }

        for (i = 0; i < iCount; i++)
        {
            GetArgv(p, i*4 + 4, szNum, 10);
            GetArgv(p, i*4 + 7, szTmp, 10);
            // U-Blox 7020 最后一个以*分隔
            get_str_to_ken_ex(szTmp, 0, '*', szValue, 10);

            // 不记录无信号卫星
            if (szValue[0] == '\0' || strcmp(szValue, "0") == 0) continue;

            sprintf(szTmp, "%s%s", szNum ,szValue);
            strcpy(gpsStrong[iPos++], szTmp);

            //将iPos >= 11 改为 iPos > 11, 解决了gpsStrong[11]无法被正常赋值的问题
            if (iPos > 11)
                iPos = 0;
        }

        // GSV收取完毕，更新szStrong.
        if (numMsg == msgNum) {
            memcpy(GpsInfo.szStrong, gpsStrong, sizeof(GpsInfo.szStrong));
        }
    }
    else if (strncmp(szBuf, "$GNGSA", strlen("$GNGSA")) == 0 || // 双系统或多系统混合定位：GPS/BDS/GAL/GLO/QZSS
             strncmp(szBuf, "$GPGSA", strlen("$GPGSA")) == 0 || // GPS单模
             strncmp(szBuf, "$GBGSA", strlen("$GBGSA")) == 0) // BDS单模
    {
        g_bReciveGpsData = TRUE;

        // 取定位状态标志：1-未定位 2-2D定位 3-3D定位
        GetArgv(p, 2, GpsInfo.szPosState, sizeof(GpsInfo.szPosState));

        // 取位置精度因子
        GetArgv(p, 15, GpsInfo.szPdop, sizeof(GpsInfo.szPdop));

        // 取水平精度因子
        GetArgv(p, 16, GpsInfo.szHdop, sizeof(GpsInfo.szHdop));
    }
    else if (strncmp(szBuf, "$BDGSV", strlen("$BDGSV")) == 0 ||
             strncmp(szBuf, "$GBGSV", strlen("$GBGSV")) == 0 ) // UC6228 V4.1
    {
        char szTmp[20] = {0};
        char szNum[10];
        char szValue[10] = {0};
        int numMsg = 0;
        int msgNum = 0;
        int numSV = 0;
        int i = 0, iCount = 0;
        static char bdsStrong[12][8];
        static int iBDPos = 0;

        g_bReciveGpsData = TRUE;

        GetArgv(p, 1, szNum, 10); // Number of messages
        numMsg = atoi(szNum);
        GetArgv(p, 2, szNum, 10); // Number of this message
        msgNum = atoi(szNum);
        GetArgv(p, 3, szNum, 10); // Number of satellites in view
        numSV = atoi(szNum);

        // 如果没有搜到任何卫星
        if (numSV == 0)
        {
            memset(g_bds.szStrong, 0, sizeof(g_bds.szStrong));
            g_bReciveBDGsv = FALSE;
            return;
        }
        g_bReciveBDGsv = TRUE;
        GetArgv(p, 3, g_bds.szSee, sizeof(g_bds.szSee)); //获取BDGSV 可视卫星总数


        if (msgNum == 1) // GSV 第一条语句
        {
            memset(bdsStrong, 0, sizeof(bdsStrong));
            iBDPos = 0;
        }

        if (numMsg > msgNum) {
            iCount = 4;
        } else { // numMsg == msgNum
            iCount = numSV - (numMsg - 1) * 4;
        }

        for (i = 0; i < iCount; i++)
        {
            GetArgv(p, i*4 + 4, szNum, 10);
            GetArgv(p, i*4 + 7, szValue, 10);

            // 不记录无信号卫星
            if (szValue[0] == '\0' || strcmp(szValue, "0") == 0) continue;

            sprintf(szTmp, "%s%s", szNum ,szValue);
            strcpy(bdsStrong[iBDPos++], szTmp);

            if (iBDPos >= 11)
                iBDPos = 0;
        }

        // GSV收取完毕，更新szStrong.
        if (numMsg == msgNum) {
            memcpy(g_bds.szStrong, bdsStrong, sizeof(g_bds.szStrong));
        }
    }

    // GPGGA
    else if(strncmp(szBuf, "$GPGGA", strlen("$GPGGA")) == 0 || // GPS单模
            strncmp(szBuf, "$BDGGA", strlen("$BDGGA")) == 0 || // BDS单模
            strncmp(szBuf, "$GBGGA", strlen("$GBGGA")) == 0 || // UC6228 BDS单模
            strncmp(szBuf, "$GNGGA", strlen("$GNGGA")) == 0) // TD1030 GPS&BDS
    {
        g_bReciveGpsData = TRUE;

        // 刷新锁定星数
        GetArgv(p, 7, GpsInfo.szLock, 3);
        // 海拔
        GetArgv(p, 9, GpsInfo.szAltitude, 20);
    }

    // 验证是否是GPRMC数据
    else if(strncmp(szBuf, "$GPRMC", strlen("$GPRMC")) == 0 || // GPS单模
            strncmp(szBuf, "$BDRMC", strlen("$BDRMC")) == 0 || // BDS单模
            strncmp(szBuf, "$GBRMC", strlen("$GBRMC")) == 0 || // UC6228 BDS单模
            strncmp(szBuf, "$GNRMC", strlen("$GNRMC")) == 0) // TD1030 GPS&BDS
    {
        if (g_bGpsDebug)
        {
            my_log_printf_internal(1, "%s\r\n", szLine);
        }

        /*
         * 连续位置点求平均值的做法，对于正常行驶轨迹具有平滑
         * 经纬度或轨迹点有好处。但是会导致在急转弯或者大拐弯
         * 情况下行驶轨迹和真实轨迹偏差较大。
         */
        static double lngArr[POINT_N]; // 存储连续N个经度
        static double latArr[POINT_N]; // 存储连续N个纬度
        static double spdArr[POINT_N]; // 存储连续N个纬度
        static double aglArr[POINT_N]; // 存储连续N个纬度
        static uint8 index = 0;
        static uint8 debugMsg = 0;
        char szLati[20], szLong[20];
        char szNewLati[20], szNewLong[20];
        char szValid[3] = { 0 };
        char szPosMode[2] = { 0 }; //定位模式标志
        double fSpeed = 0.0;
        double fAngle = 0.0;
        bool bPosValid = FALSE;
        bool bAddPos = FALSE;

        g_bReciveGpsData = TRUE;

        if (FALSE == check_line((char*)szLine))            //校验GPRMC是否有乱码
        {
            my_log_printf(1, "GPRMC/GNRMC 校验错误!");
            return ;                                          //不做任何的处理，直接返回
        }

        // $GNRMC,085926.000,A,2234.59649,N,11356.58520,E,0.000,0.000,281025,,,A,S*09
        ///////////////////////////////////////////////////////
        // 刷新时间
        GetArgv(p, 1, GpsInfo.szUTC, 20);
        GetArgv(p, 9, GpsInfo.szDate, 20);

        // 东西经和速度，方位角等
        GetArgv(p, 4, GpsInfo.szNS, 2);
        GetArgv(p, 6, GpsInfo.szEW, 2);
        GetArgv(p, 7, GpsInfo.szSpeed, 10);
        GetArgv(p, 8, GpsInfo.szAngle, 10);

        // 取经纬度值
        GetArgv(p, 3, szLati, 20);
        GetArgv(p, 5, szLong, 20);

        // 取出有效位标记
        GetArgv(p, 2, szValid, 2);

        /**
         * 定位模式标志
         * 中科微AT6558R规格书定义：
         * A=自主模式
         * E=估算模式（航位推算）
         * N=数据无效
         * D=差分模式
         * M=未定位，但存在外部输入或历史保存的位置
         *
         * 芯与物CC1161W规格书定义：
         * N=未定位
         * A=单点定位
         * D=差分定位
         */
        GetArgv(p, 12, szPosMode, sizeof(szPosMode));

        // 取导航状态标志 注：仅NMEA 4.1及以上版本有效
        // S– 位置可靠
        // C– 位置谨慎使用
        // U– 位置不可靠
        // V– 设备不提供导航状态信息
        GetArgv(p, 13, GpsInfo.szNavState, sizeof(GpsInfo.szNavState));

        if (szValid[0] == 'A')
        {
            g_bPosValid = TRUE;

            //my_log_printf(1, "GPS已定位!");

            // 转换当前经纬度
            ConvertGPS(szLati, szNewLati, sizeof(szNewLati));  // 转换成度结构
            ConvertGPS(szLong, szNewLong, sizeof(szNewLong));  // 转换成度结构

            // 在使用index检查index是否合法，防止逻辑出错！
            if (index >= POINT_N) index = 0;

            // 经纬度字符串转浮点
            myatof_ex(szNewLong, lngArr + index);
            myatof_ex(szNewLati, latArr + index);

            // 取速度
            myatof_ex(GpsInfo.szSpeed, &fSpeed);
            // 取方位角
            myatof_ex(GpsInfo.szAngle, &fAngle);
            spdArr[index] = fSpeed;
            aglArr[index] = fAngle;
            index++;

            if (g_bGpsDebug)
            {
                my_log_printf(1, "当前GPS经度/纬度/速度/角度: %s|%s|%.1f|%.2f",
                                    szNewLong, szNewLati, fSpeed, fAngle);
            }

            // 之前的数据无效，首次卫星定位
            // 根据实际测试，国产芯片首次卫星定位的位置漂移比较大。
            // 需要将前面三个位置点过滤掉
            if( GpsInfo.szValid[0] != 'A' )
            {
                // 使用最新数据前先清理之前缓存的定位数据！！！
                index = 0;
            }
            // 如果之前有数据，则检查经纬度
            else
            {
                double dLng = 0;
                double dLat = 0;
                double fLastAngle = 0;
                int i = 0;

                // 重新初始化fSpeed和fAngle，前面已被赋值过！
                fSpeed = 0.0;
                fAngle = 0.0;

                // 平滑经纬度
                if (index == POINT_N)
                {
                    for (i = 0; i < POINT_N; i++)
                    {
                        dLng += lngArr[i];
                        dLat += latArr[i];
                        fSpeed += spdArr[i];
                        fAngle += aglArr[i];
                    }
                    dLng = dLng / (double) POINT_N;
                    dLat = dLat / (double) POINT_N;
                    fSpeed = fSpeed / (double) POINT_N;
                    fAngle = fAngle / (double) POINT_N;

                    // 校正低速漂移点
                    if (fSpeed < 1)
                    {
                        fSpeed = 0;
                    }

                    myftoa_ex(dLng, 6, GpsInfo.szLongitude, sizeof(GpsInfo.szLongitude));
                    myftoa_ex(dLat, 6, GpsInfo.szLatitude, sizeof(GpsInfo.szLatitude));
                    myftoa_ex(fSpeed / 1.852, 3, GpsInfo.szSpeed, sizeof(GpsInfo.szSpeed)); //GPS速度单位转换：1节=1海里/小时=1.852公里/小时
                    myftoa_ex(fAngle, 2, GpsInfo.szAngle, sizeof(GpsInfo.szAngle));

                    bPosValid = TRUE;

                    index = 0; // 重新计算POINT_N个点
#if 0
                    my_log_printf(1, "Average Lng/Lat/Spd/Angle: %s|%s|%s|%s",
                                  GpsInfo.szLongitude, GpsInfo.szLatitude,
                                  GpsInfo.szSpeed, GpsInfo.szAngle);
#endif
                }
                else
                {
                    return; // index < POINT_N 不刷新最后数据
                }
            } // szValid = 'A'

            // 保存下数据有效性
            GpsInfo.szValid[0] = szValid[0];
        }
        else // 如果当前数据无效
        {
            g_bPosValid = FALSE;

            // 设备首次失去定位，立即上报最近一次有效定位!
            if (GpsInfo.szValid[0] == 'A')
            {
                // 记录当前位置无效，但暂时不改写g_gps
                GpsInfo.szValid[0] = 0;
#if 0
                my_log_printf(1, "失去定位，立即上报最后一次有效位置！");
                bAddPos = TRUE;
#endif
            }
            else
            {
                GpsInfo.szValid[0] = 0;
                // Save whole GPS info.
                memcpy(&g_gps, &GpsInfo, sizeof(GpsInfo));
            }
        }
     } // END OF GPRMC
 }


// 将GPS数据按行进行处理
void GPS_LineCheck(uint8 *szBuf)
{
    static uint16 index = 0;
    static uint8 szGPS[MAX_GPS_LENGTH + 2];          // 做为行缓弃区，多留一个字节
    char *p = (char *)szBuf;

    // 先验证输入不为空
    if (p == NULL)
        return;

    while (*p != '\0')  //处理所有字符
    {
        if ((*p == '\r') || (*p =='\n'))  // 遇到换行符了
        {
            szGPS[index] = '\0';  // 构造一个字符串
            GPS_Parse(szGPS);

            index = 0;  // 将索引指针指禹行首

            p++;   // 跳过\r
            if (*p == '\n')   // 如果下一个字符是\n，则跳过
            {
                p++;   // 跳过\n
            }
        }
        else
        {
            szGPS[index++] = *p++;

            if (index >= MAX_GPS_LENGTH)
            {
                my_log_printf(1, "GPS_LineCheck() Error: line is too long!");
                index = 0;
            }
        }
    }
}


// 分多段写入，每次最多写入32B. Uart FIFO size is 64B.
uint16 Gps_puthex_ex(uint8 *pData, uint16 iLen)
{
    uint32 write_len = 0;
    uint32 write_len_ret = 0;
    uint32 out_cnt = 0;
    int fail_cnt = 0;

    do
    {
        if (iLen - out_cnt > 1024)
        {
            write_len = 1024;
        }
        else
        {
            write_len = (iLen - out_cnt);
        }

        write_len_ret = my_uart_write_data(MY_GPS_PORT, pData + out_cnt, write_len);
        out_cnt += write_len;
        JM_Sleep(50); // 115200波特率

    } while (out_cnt < iLen);

    my_log_printf(1, "\r\nData length %d/%d is written.\r\n", out_cnt, iLen);

    return out_cnt;
}



// 115200波特率要求readBuf至少2KB，否则数据会被覆盖丢失
// 9600波特率要求readBuf至少1KB，否则数据会被覆盖丢失
#define GPS_READ_SIZE       2048

/*************************************************************
**函数名称:  my_gps_task
**入口参数:  argc -- 入口参数的个数
**           argv -- 参数的具体内容
**出口参数:  无
**函数功能:  GPRS 任务处理函数
*************************************************************/
void my_gpsrecv_task(void *argv)
{
    size_t readLen = 0;
    static unsigned char __attribute__ ((aligned(32))) readBuf[GPS_READ_SIZE];

    //my_log_printf(1, "############# my_gps_task is runing ##################");

    //打开GPS供电函数内部会做GPS UART初始化
    //my_gps_uart_init();

    //开机首次开启GPS，获取GPS固件版本后再关闭GPS
    open_gps_power();

    while (1)
    {
        /* readLen data. */
        memset(readBuf, 0, GPS_READ_SIZE);
        readLen = my_gps_uart_read_data(readBuf, GPS_READ_SIZE - 1);
        if (readLen == 0 || readLen >= GPS_READ_SIZE)
        {
            //NOTE: 当GPS UART口被关闭是uart_read会直接返回0
            //my_log_printf(0, "gps_uart_read: err size = (%d)!\n", readLen);
            JM_Sleep(500);
            continue;
        }

        readBuf[readLen] = 0x00;
        GPS_LineCheck((uint8 *)readBuf);
    }
}


