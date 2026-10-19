
#include <string.h>
#include "app_log.h"

#define __inf                   APP_LOGI

/*农历转换部分*/
const unsigned int lunar68y[68] = //把获得的数据按年份存起来。年份代表的日期什么的，没有负数。

{
    0x096D46,                                       /*1970*/
    0x54AEBB,0x04AD4F,0x0A4D43,0x4D26B7,0x0D254B,0x8D52BF,0x0B5452,0x0B6A47,0x696D3C,0x095B50,/*1971-1980*/
    0x049B45,0x4A4BB9,0x0A4B4D,0xAB25C2,0x06A554,0x06D449,0x6ADA3D,0x0AB651,0x093746,0x5497BB,/*1981-1990*/
    0x04974F,0x064B44,0x36A537,0x0EA54A,0x86B2BF,0x05AC53,0x0AB647,0x5936BC,0x092E50,0x0C9645,/*1991-2000*/
    0x4D4AB8,0x0D4A4C,0x0DA541,0x25AAB6,0x056A49,0x7AADBD,0x025D52,0x092D47,0x5C95BA,0x0A954E,/*2001-2010*/
    0x0B4A43,0x4B5537,0x0AD54A,0x955ABF,0x04BA53,0x0A5B48,0x652BBC,0x052B50,0x0A9345,0x474AB9,/*2011-2020*/
    0x06AA4C,0x0AD541,0x24DAB6,0x04B64A,0x69573D,0x0A4E51,0x0D2646,0x5E933A,0x0D534D,0x05AA43,/*2021-2030*/
    0x36B537,0x096D4B,0xB4AEBF,0x04AD53,0x0A4D48,0x6D25BC,0x0D254F,/*2031-2037*/
};


typedef struct _lunar //储存农历相关信息 
{
    int             year;
    int             month;
    int             day;
    int             is_leap;
} lunar_t;


int lunarLeapMonth(int y) //判断当前年是否闰月，并得到闰的月份；不闰月就得到0。 
{
    return ((lunar68y[y - 1970] &0xf00000) >> 20); //0xf00000：1111 0000 0000 0000 0000 0000；把储存的前四位数据取出来 
}


int lunarMonthDays(int y,int m) //由于农历朔望月30或29是不确定的，此处用来判断输出。看下指定年份在指定月是多少天。 
{


    return ((lunar68y[y - 1970] & (0x80000 >> (m - 1))) ? 30: 29); /*0x80000：1000 0000 0000 0000 0000；
                经过移位，可以把第m个月存的数据调用出来*/
}


int lunarLeapMonthDays(int y) //如果闰月，得到闰月的大小，是30天还是29天。 
{
    if (lunarLeapMonth(y)) {



        return ((lunar68y[y - 1970] & (0x80000 >> (lunarLeapMonth(y) - 1))) ? 30: 29); /*0x80000：1000 0000 0000 0000
                             0000；
                                0x80000>>(lunarLeapMonth(y)-1)部分，移动1的位置 控制提取闰月在数据中的位置*/
        }
    else {
        return 0;
        }
}




int lunarSpringDate(int y,int * solar_m,int * solar_d) /*得到农历的春节，即正月初一在阳历中日期。
在函数中，把数据提取出来，利用指针变量来储存在函数中得到的结果。*/
{
    if (!solar_m || !solar_d)
        return - 1;

    *solar_m            = (lunar68y[y - 1970] &0x0060) >> 5; //0x0060：0000 0000 0110 0000；调用储存的春节月份数据 
    *solar_d            = lunar68y[y - 1970] &0x1f; //0x1f：0001 1111；调用储存的春节日期数据
    return 0;
}


int toLunar(int year,int month,int day,lunar_t * lunar) //输入公历日期，得到对应的农历日期。 
{
    int             monthTotal[13] = {
        0,31,59,90,120,151,181,212,243,273,304,334,365
        };

    //平年，365天，相邻数据间隔代表月有多长，比如31-0=31，公历下1月31天。
    int             bySpring,bySolar,daysPerMonth;

    //bySpring 记录春节离当年元旦的天数。
    //bySolar 记录阳历日离当年元旦的天数。
    int             index,flag;

    //index 记录从哪个月开始来计算。
    //flag 是用来对闰月的特殊处理。
    int             spring_solar_d;
    int             spring_solar_m;

    lunarSpringDate(year,&spring_solar_m,&spring_solar_d);

    if (spring_solar_m == 1) //春节在一月，与元旦1月1日一个月份，对日上处理就行 
        bySpring = spring_solar_d - 1;
    else 
        //春节不在一月，就是在二月，不可能跑到三月去，中间差了一个完整的公历一月，还有在公历二月多出来那几天。 
        bySpring = spring_solar_d - 1 + 31;

    bySolar             = monthTotal[month - 1] +day - 1;

    //阳历日到春节天数，利用上面之前定义的一个变量。先算差的整个月份的天数，再算上不到一个月的天数 

    if ((! (year % 4)) && (month > 2)) /*上面是按平年来说的，如果是公历闰年，并且，月份超过了2月，就得在天数上再加上一天。
	因为公历闰年是在2月上做一个处理，天数加一，由28天到29天*/
        bySolar++;

    //判断阳历日在春节前还是春节后
    if (bySolar >= bySpring) /*/阳历日在春节后（含春节那天）。*/
    {
        bySolar             -= bySpring;            //这样以元旦为基准就得到了阳历日距离春节的天数

        index               = 1; /*春节开始，从农历正月开始判断，这个是对储存的数据中，月份对应的第几位的记录。*/
        month               = 1;                    //初始化，记录月份，从正月开始。 
        flag                = 0;                    //初始化

        //闰月，月份名字是不变的。只是在月份前加个闰字。所以引进index和month进行区分。 
        daysPerMonth        = lunarMonthDays(year,month); //daysPerMonth记录大小月的天数 29 或30

        while (bySolar >= daysPerMonth) //阳历日距离春节的天数大于等于农历月得到的天数的话 
        {
            bySolar             -= daysPerMonth;    //距离天数减去已经知道的农历月的天数，不断逼近我们想要的结果 

            if (month == lunarLeapMonth(year)) //农历闰月的情况。判断当前月份是不是农历闰月闰的那个月 
            {
                flag                = ~flag;        //按位取反后由0变成-1；由-1变为0；利用这个情况， 对闰月进行了处理。 

                if (flag == 0) {
                    month++;
                    }
                }
            else //不是闰月，月份加一，继续判断 
            {
                month++;
                }

            index++;                                //在不是闰月的情况下，index和month是同时变换的。只有在闰月情况下，index会比month多了一个一。 
            daysPerMonth        = lunarMonthDays(year,index); //判断一个新的月份的天数。 
            }

        day                 = bySolar + 1;
        }
    else //阳历日在春节前 ，这样就相当于用的上一个农历年的数据。一个公历年涉及到两个农历年 
    {
        bySpring            -= bySolar;             //这样以元旦为基准就得到了阳历日距离春节的天数   
        year--;                                     //这个时间段对应的是上一个农历年，这个农历年大部分时间是在上一个公历年，年份减一。  
        month               = 12;                   //农历年最后一个月是腊月 



        if (lunarLeapMonth(year) == 0) /*上面函数确定的，不闰月的返回0；
		由于上一年农历数据是在数据末尾储存，没有闰月，农历月总共是有12个月*/
            index = 12;
        else 
            index = 13; //有闰月的存在，农历年就有13个月

        flag                = 0;                    //初始化     
        daysPerMonth        = lunarMonthDays(year,index); //daysPerMonth记录大小月的天数29或者30 

        while (bySpring > daysPerMonth) //差值大于一个月，继续循环 
        {
            bySpring            -= daysPerMonth;    //差值减掉那一个正月的天数    
            index--;                                //继续判断下一个月份天数   

            if (flag == 0) //代表不是闰月      
                month--; //月份继续减一判断        

            if (month == lunarLeapMonth(year)) //判断月份是不是闰月的那个月份 
            {
                flag                = !flag;        //对其进行处理，使得下次循环判断，月份不变。闰月，月份名字是不变的。只是在月份前加个闰字 比如，四月，闰四月 
                }

            daysPerMonth        = lunarMonthDays(year,index); //接着判断大小月 
            }

        day                 = daysPerMonth - bySpring + 1; //那个月的天数减去那个差值再加上1，就是指定日期在这个月的日 
        }

    lunar->day          = day;
    lunar->month        = month;
    lunar->year         = year;                     //赋值，为了便于后续输出 

    if (month == lunarLeapMonth(year) && index != month) //对闰月的一个处理 
        lunar->is_leap = 1; //代表这个时候有闰月的情况，后续输出，多输出一个“闰”字 
    else 
        lunar->is_leap = 0; //没闰月 

    return 0;
}


/*求某年干支生肖部分*/
void gz_sx(int year,char * nongli_year) //判断干支与生肖。 
{
    int             a,b,chazhi;

    //    char tgdz[8];   
    const char *    tiangan[] = {
        "甲","乙","丙","丁","戊","己","庚","辛","壬","癸"
        };
    const char *    dizhi[] = {
        "子","丑","寅","卯","辰","巳","午","未","申","酉","戌","亥"
        };
    const char *    shengxiao[] = {
        "鼠年","牛年","虎年","兔年","龙年","蛇年","马年","羊年","猴年","鸡年","狗年","猪年"
        };
    chazhi              = year - 2020;              //2020年农历年主要是庚子年,以其为基准，推算。  
    a                   = (chazhi + 6) % 10;        //庚是定义天干数组中第七个汉字   
    b                   = (chazhi + 0) % 12;        //子是定义地支数组中第一个汉字

    if (a < 0)
        a = a + 10;

    if (b < 0)
        b = b + 12;

    strcpy(nongli_year,tiangan[a]);
    strcat(nongli_year,dizhi[b]);
    strcat(nongli_year,shengxiao[b]);

    //    printf("%s%s年",tgdz,shengxiao[b]);
}


void nongli(int year,int month,int day,char * nongli_year,char * nongli_month) //输入想要判断的公历日期，得到农历并且输出。 
{
    const char *    ChDay[] =                       //农历月中日的叫法 

    {
        "*","初一","初二","初三","初四","初五",
        "初六","初七","初八","初九","初十",
        "十一","十二","十三","十四","十五",
        "十六","十七","十八","十九","二十",
        "廿一","廿二","廿三","廿四","廿五",
        "廿六","廿七","廿八","廿九","三十"
        };
    const char *    ChMonth[] = {
        "*","正月","二月","三月","四月","五月","六月","七月","八月","九月","十月","冬月",
        "腊月"
        };

    //农历月名称 
    //    int year,month,day;
    int             year1,month1,day1;
    lunar_t         lunar;

    //	printf("请按格式输入想要查询的公历日期:(例如2020年7月1日，输入2020-7-1) \n注意：范围为1901-2-19到2099-12-31\n");
    //    scanf("%d-%d-%d",&year,&month,&day);
    year1               = year;
    month1              = month;
    day1                = day;                      //后续求春节日期，会改变储存的，year,month,day的值，先储存下，便于后面判断。
    lunarSpringDate(year1,&month1,&day1);           //得到春节日期，为了具体干支年的判断 
    toLunar(year,month,day,&lunar);

    //	printf("对应农历为 "); 
    if (month > month1) //一个公历年有涉及到两个农历年；以春节为界限。之前为上一农历年。之后为下一农历年 
        gz_sx(year,nongli_year); //用到下一个农历年干支 
    else if ((month == month1) && day >= day1)
        gz_sx(year,nongli_year); //用到下一个农历年干支
    else 
        gz_sx(year - 1,nongli_year); //用到上一个农历年干支 

    strcpy(nongli_month,ChMonth[lunar.month]);
    strcat(nongli_month,ChDay[lunar.day]);


}


void nongli_num(int year,int month,int day,unsigned char * nongli_month) //输入想要判断的公历日期，得到农历并且输出。 
{
    const unsigned char ChDay[31] =                 //农历月中日的叫法 

    {
        0,1,2,3,4,5,
        6,7,8,9,10,
        11,12,13,14,15,
        16,17,18,19,20,
        21,22,23,24,25,
        26,27,28,29,30
        };
    const unsigned char ChMonth[] = {
        0,1,2,3,4,5,6,7,8,9,10,11,12
        };

    //农历月名称 
    //    int year,month,day;
    int             year1,month1,day1;
    lunar_t         lunar;

    //	printf("请按格式输入想要查询的公历日期:(例如2020年7月1日，输入2020-7-1) \n注意：范围为1901-2-19到2099-12-31\n");
    //    scanf("%d-%d-%d",&year,&month,&day);
    year1               = year;
    month1              = month;
    day1                = day;                      //后续求春节日期，会改变储存的，year,month,day的值，先储存下，便于后面判断。
    lunarSpringDate(year1,&month1,&day1);           //得到春节日期，为了具体干支年的判断 
    toLunar(year,month,day,&lunar);
    nongli_month[0]     = ChDay[lunar.day];
    nongli_month[1]     = ChMonth[lunar.month];

}


