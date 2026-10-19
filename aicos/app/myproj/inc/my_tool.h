/********************************************************************
**版权所有         深圳市几米物联有限公司
**文件名称:        my_tool.h
**文件描述:        my_tool.c头文件声明
**当前版本:        V1.0
*********************************************************************/
#ifndef __MY_TOOL_H__
#define __MY_TOOL_H__


#ifndef MAX
#define MAX(a, b)       (((a) > (b) ) ? (a) : (b))
#endif
#ifndef MIN
#define MIN(a, b)       (((a) < (b) ) ? (a) : (b))
#endif
#ifndef MID
#define MID(a,b,c)      (MAX(a,b)>c?MAX(MIN(a,b),c):MIN(MAX(a,b),c))
#endif

#ifndef ABS
#define ABS(x)  ((x) >= 0 ? (x) : -(x))
#endif

bool get_str_to_ken_ex(char *szInput, uint16 iPos,char cSplit,char *szOutBuf, uint16 iBuffLen);
void myatof_ex(char *str, double *data);
uint32 myftoa_ex(double n, uint8 a, char* str, uint32 size);

char *my_send_at_cmd(char *atcmd);
int my_atcmd_send_sync(char* cmd, char* pRsp, int rspLen, int timeout);

#endif

