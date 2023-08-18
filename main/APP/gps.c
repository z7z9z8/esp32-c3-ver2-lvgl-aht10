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
    //��������
    bzero(&gps_time, sizeof(gps_time));

    row = strstr(raw, "$GNZDA");
    pos1 = strchr(row, ',');                          // UTCʱ��...
    pos2 = strchr(pos1 + 1, ',');                     // ��...
    cut_substr(gps_time.utc, pos1, 1, pos2 - pos1);   // UTCʱ��
    pos1 = strchr(pos2 + 1, ',');                     // ��...
    cut_substr(gps_time.day, pos2, 1, pos1 - pos2);   // ��
    pos2 = strchr(pos1 + 1, ',');                     // ��...
    cut_substr(gps_time.month, pos1, 1, pos2 - pos1); // ��
    pos1 = strchr(pos2 + 1, ',');                     // ��ʱ��Сʱ...
    cut_substr(gps_time.year, pos2, 1, pos1 - pos2);  // ��
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
    //��������
    bzero(&gps_data, sizeof(gps_data));
    //���س���GPRMC��λ��
    row = strstr(shuju, "$GNRMC");
    pos1 = strchr(row, ',');
    pos2 = strchr(pos1 + 1, ',');
    cut_substr(gps_data.UTC_time, pos1, 1, pos2 - pos1 - 1); // UTCʱ��
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
    cut_substr(gps_data.lat, pos1, 1, pos2 - pos1 - 1); //γ��

    pos1 = strchr(pos2 + 1, ','); //�Ǳ�����
    if (*(pos2 + 1) == 'N')
    {
        gps_data.is_N = true;
    }
    else
    {
        gps_data.is_N = false;
    }

    pos2 = strchr(pos1 + 1, ',');
    cut_substr(gps_data.lon, pos1, 1, pos2 - pos1-1); //����

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
    cut_substr(gps_data.speed, pos1, 1, pos2 - pos1-1); //�ٶ�

    pos1 = strchr(pos2 + 1, ',');
    cut_substr(gps_data.HangXiang,pos2,1,pos1-pos2-1); //����

    return gps_data;
}

gps_lat_lon_t get_gps_lat_lon(char *raw)
{
    char *row;
    char *pos1;
    char *pos2;

    gps_lat_lon_t gps_lat_lon;
    //��������
    bzero(&gps_lat_lon, sizeof(gps_lat_lon));
    row = strstr(raw, "$GNGGA");                           //���ص�һ�γ��� "$GNGGA"��λ��ָ��
    pos1 = strchr(row, ',');                               // UTCʱ��...
    pos2 = strchr(pos1 + 1, ',');                          // γ��...
    pos1 = strchr(pos2 + 1, ',');                          // γ�ȷ���...
    cut_substr(gps_lat_lon.lat, pos2, 1, pos1 - pos2 - 1); // γ��
    pos2 = strchr(pos1 + 1, ',');                          // ����...
    pos1 = strchr(pos2 + 1, ',');                          // ���ȷ���...
    cut_substr(gps_lat_lon.lon, pos2, 1, pos1 - pos2 - 1); // ����

    printf("lat=%s lon=%s\n", gps_lat_lon.lat, gps_lat_lon.lon);
    return gps_lat_lon;
}

//��src��startλ����ȡn���ַ�����ֵ��dest�ҷ���dest
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
