/*
 * @Author: Dawn
 * @Date: 2022-06-20 18:06:26
 * @LastEditors: Dawn
 * @description:
 * @LastEditTime: 2022-06-26 08:13:34
 * Keep On Going
 * Copyright (c) 2022 by Dawn, All Rights Reserved.
 */
#ifndef GPS_H
#define GPS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// 时间
typedef struct
{
    char utc[16];
    char day[8];
    char month[8];
    char year[8];
} gps_time_t;

typedef struct
{
    int year;
    int month;
    int day;
    int hour;
    int min;
    int sec;
} UTC_TIME;

// 经纬度
typedef struct
{
    char lat[16];
    char lon[16];
} gps_lat_lon_t;

typedef struct
{
    char UTC_time[11];
    bool is_Effective_positioning;
    char lat[16];      // 纬度
    bool is_N;         // 是北半球吗
    char lon[16];      // 经度
    bool is_E;         // 是东经吗
    char speed[7];     // 速度
    char HangXiang[7]; // 航向
} GPRMC_DATA;

/**
08:49:54  $GNGGA,084954.000,3252.04064,N,11211.73602,E,1,14,1.2,135.3,M,-23.2,M,,*68
08:49:54  $GNGLL,3252.04064,N,11211.73602,E,084954.000,A,A*41
08:49:54  $GNGSA,A,3,05,13,15,18,29,199,,,,,,,1.9,1.2,1.5,1*0E
08:49:54  $GNGSA,A,3,07,13,16,21,34,38,42,44,,,,,1.9,1.2,1.5,4*30
08:49:54  $GPGSV,4,1,13,02,54,106,,05,63,353,24,06,09,118,,07,06,049,,0*6E
08:49:54  $GPGSV,4,2,13,11,44,101,,13,67,150,30,15,42,211,23,18,12,309,30,0*67
08:49:54  $GPGSV,4,3,13,20,46,048,,29,48,293,17,30,15,076,,195,54,146,,0*5C
08:49:54  $GPGSV,4,4,13,199,49,154,20,0*69
08:49:54  $BDGSV,4,1,16,07,16,199,20,08,59,360,,10,20,210,,11,54,312,,0*75
08:49:54  $BDGSV,4,2,16,12,67,168,,13,57,340,19,16,34,172,30,21,71,006,19,0*7B
08:49:54  $BDGSV,4,3,16,22,23,047,,34,73,278,25,38,63,029,21,39,42,168,,0*7A
08:49:54  $BDGSV,4,4,16,40,10,190,,42,45,251,24,43,26,317,,44,42,157,24,0*7C

08:49:54  $GNRMC,084954.000,A,3252.04064,N,11211.73602,E,0.00,0.00,250622,,,A,V*0D
08:49:54  $GNVTG,0.00,T,,M,0.00,N,0.00,K,A*23
08:49:54  $GNZDA,084954.000,25,06,2022,00,00*4F
08:49:54  $GPTXT,01,01,01,ANTENNA OK*35
*/
/*
 *
 **$GPRMC ,021851.00,A,2233.51742,N,11356.58152,E,0.010,,260517,,,A*77

 *          $GPRMC,<1>,<2>,<3>,<4>,<5>,<6>,<7>,<8>,<9>,<10>,<11>,<12>*hh<CR><LF>
08:49:54  $GNRMC,084954.000,A,3252.04064,N,11211.73602,E,0.00,0.00,250622,,,A,V*0D
 *
 <1> UTC时间，hhmmss（时分秒）格式

 * <2> 定位状态，A=有效定位，V=无效定位

 * <3> 纬度ddmm.mmmm（度分）格式（前面的0也将被传输）

 * <4> 纬度半球N（北半球）或S（南半球）

 * <5> 经度dddmm.mmmm（度分）格式（前面的0也将被传输）

 * <6> 经度半球E（东经）或W（西经）

 * <7> 地面速率（000.0~999.9节，前面的0也将被传输）

 * <8> 地面航向（000.0~359.9度，以真北为参考基准，前面的0也将被传输）

 * <9> UTC日期，ddmmyy（日月年）格式

 * <10> 磁偏角（000.0~180.0度，前面的0也将被传输）

 * <11> 磁偏角方向，E（东）或W（西）

 * <12> 模式指示（仅NMEA0183 3.00版本输出，A=自主定位，D=差分，E=估算，N=数据无效）

 * 我们所关心的是GPRMC这条信息，因为其中包括当前格林威治时间、经度、纬度、日期等。

 * **/
GPRMC_DATA get_gps_moreData(char *shuju);
gps_time_t get_gps_time(char *shuju);
gps_lat_lon_t get_gps_lat_lon(char *shuju);
char *cut_substr(char *dest, char *src, char start, int n);

#endif
