/*
 * @Author: Dawn
 * @Date: 2022-06-20 18:08:01
 * @LastEditors: Dawn
 * @description:
 * @LastEditTime: 2022-06-25 23:29:29
 * Keep On Going
 * Copyright (c) 2022 by Dawn, All Rights Reserved.
 */
#include "gps.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

gps_time_t get_gps_time(char *raw)
{
    char *row;
    char *pos1;
    char *pos2;

    gps_time_t gps_time;
    //清理数据
    bzero(&gps_time, sizeof(gps_time));

    row = strstr(raw, "$GNZDA");
    pos1 = strchr(row, ',');                          // UTC时间...
    pos2 = strchr(pos1 + 1, ',');                     // 日...
    cut_substr(gps_time.utc, pos1, 1, pos2 - pos1);   // UTC时间
    pos1 = strchr(pos2 + 1, ',');                     // 月...
    cut_substr(gps_time.day, pos2, 1, pos1 - pos2);   // 日
    pos2 = strchr(pos1 + 1, ',');                     // 年...
    cut_substr(gps_time.month, pos1, 1, pos2 - pos1); // 月
    pos1 = strchr(pos2 + 1, ',');                     // 本时区小时...
    cut_substr(gps_time.year, pos2, 1, pos1 - pos2);  // 年
    //printf("utc=%s day=%s month=%s year=%s\n", gps_time.utc, gps_time.day, gps_time.month, gps_time.year);
    return gps_time;
}

GPRMC_DATA get_gps_moreData(char *shuju)
{
    char *row;
    char *pos1;
    char *pos2;


//  *$GPRMC,<1>,<2>,<3>,<4>,<5>,<6>,<7>,<8>,<9>,<10>,<11>,<12>*hh<CR><LF>
// 08:49:54  $GNRMC,084954.000,A,3252.04064,N,11211.73602,E,0.00,0.00,250622,,,A,V*0D

    GPRMC_DATA gps_data;
    //清理数据
    bzero(&gps_data, sizeof(gps_data));
    //返回出现GPRMC的位置
    row = strstr(shuju, "$GNRMC");
    pos1 = strchr(row, ',');
    pos2 = strchr(pos1 + 1, ',');
    cut_substr(gps_data.UTC_time, pos1, 1, pos2 - pos1 - 1); // UTC时间
    pos1 = strchr(pos2 + 1, ',');
    if (*(pos2 + 1) == 'A')
    {
        gps_data.is_Effective_positioning = true;
    }
    else
    {
        gps_data.is_Effective_positioning = false;
    }
    pos2 = strchr(pos1 + 1, ',');
    cut_substr(gps_data.lat, pos1, 1, pos2 - pos1 - 1); //纬度

    pos1 = strchr(pos2 + 1, ','); //是北半球
    if (*(pos2 + 1) == 'N')
    {
        gps_data.is_N = true;
    }
    else
    {
        gps_data.is_N = false;
    }

    pos2 = strchr(pos1 + 1, ',');
    cut_substr(gps_data.lon, pos1, 1, pos2 - pos1-1); //经度

    pos1 = strchr(pos2 + 1, ',');
    if (*(pos2 + 1) == 'E')
    {
        gps_data.is_E = true;
    }
    else
    {
        gps_data.is_E = false;
    }

    pos2=strchr(pos1+1,',');
    cut_substr(gps_data.speed, pos1, 1, pos2 - pos1-1); //速度

    pos1 = strchr(pos2 + 1, ',');
    cut_substr(gps_data.HangXiang,pos2,1,pos1-pos2-1); //航向

    return gps_data;
}

gps_lat_lon_t get_gps_lat_lon(char *raw)
{
    char *row;
    char *pos1;
    char *pos2;

    gps_lat_lon_t gps_lat_lon;
    //清理数据
    bzero(&gps_lat_lon, sizeof(gps_lat_lon));
    row = strstr(raw, "$GNGGA");                           //返回第一次出现 "$GNGGA"的位置指针
    pos1 = strchr(row, ',');                               // UTC时间...
    pos2 = strchr(pos1 + 1, ',');                          // 纬度...
    pos1 = strchr(pos2 + 1, ',');                          // 纬度方向...
    cut_substr(gps_lat_lon.lat, pos2, 1, pos1 - pos2 - 1); // 纬度
    pos2 = strchr(pos1 + 1, ',');                          // 经度...
    pos1 = strchr(pos2 + 1, ',');                          // 经度方向...
    cut_substr(gps_lat_lon.lon, pos2, 1, pos1 - pos2 - 1); // 经度

    printf("lat=%s lon=%s\n", gps_lat_lon.lat, gps_lat_lon.lon);
    return gps_lat_lon;
}

//从src的start位置起取n个字符并赋值给dest且返回dest
char *cut_substr(char *dest, char *src, char start, int n)
{

    char *p = dest;
    char *q = src;

    q += start;
    while (n--)
        *(p++) = *(q++);
    *(p++) = '\0';
    return dest;
}
