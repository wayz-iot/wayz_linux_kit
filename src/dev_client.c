
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "dev_client.h"

#define  WAYZ_FAIL      -1
#define  WAYZ_OK        0


static void string_to_low(char *data)
{
    int i = 0;
    for (i = 0; i < strlen(data); i ++)
    {
        if (data[i] >= 'A' && data[i] <= 'Z')
        {
            data[i] = 'a' + (data[i] - 'A');
        }
    }
}

#if 1 // wifi chip info
char scan_file_handle(tap_info *data)
{
    FILE *fp;
	char buf[10] = {0};
    int current_index = 0; //当前读取的行
    char str_line[STR_LEN] = {0}; //每行最大读取的字符数,可根据实际情况扩大
    system("iw wlan0 scan > /tmp/wifi_scan");
    printf("wifi scan handle start.\r\n");
    if((fp = fopen("/tmp/wifi_scan", "r")) == NULL) //判断文件是否存在及可读
    {
        printf("open file %s failure!\r\n", "/tmp/wifi_scan");
        return WAYZ_FAIL;
    }

    while (NULL != fgets(str_line, STR_LEN, fp))
    {
        if (NULL != strstr(str_line, "on wlan"))
        {
            memcpy(data->tinfoAp[current_index].mac, str_line + 4, 17);
            data->tinfoAp[current_index].mac[17] = '\0';
        }

        if (NULL != strstr(str_line, "freq:"))
        {
            bzero(buf, sizeof (buf));
            sscanf(str_line, " freq: %s", buf);
            if (0 != strlen(buf))
            data->tinfoAp[current_index].freq = atoi(buf);
        }

        if (NULL != strstr(str_line, "signal:"))
        {
            bzero(buf, sizeof (buf));
            sscanf(str_line, " signal: %s dBm", buf);
            if (0 != strlen(buf))
            data->tinfoAp[current_index].rssi = atoi(buf);
        }
        
        if (NULL != strstr(str_line, "SSID:"))
        {
            memcpy(data->tinfoAp[current_index].ssid, str_line + 7, strlen(str_line) - 8);
            data->tinfoAp[current_index].ssid[strlen(str_line) - 8] = '\0';
        }

        if (NULL != strstr(str_line, "DS Parameter set: channel"))
        {
            bzero(buf, sizeof (buf));
            sscanf(str_line, " DS Parameter set: channel %s", buf);
            if (0 != strlen(buf))
            data->tinfoAp[current_index].channel = atoi(buf);
            current_index++;
        }
    }
    printf("wifi scan handle end.\r\n");
    fclose(fp); //关闭文件

	data->count = current_index;
    return current_index;
}

static char get_wifi_sta_chip_mac(char *addr)
{
    FILE *fp;
	char buf[20] = {0};
    char temp[20] = {0};
    int wlan_index = 0; //当前读取的行
    char str_line[STR_LEN] = {0}; //每行最大读取的字符数,可根据实际情况扩大
    if((fp = popen("ifconfig", "r")) == NULL) //判断文件是否存在及可读
    {
        printf("exce ifconfig failure!\r\n");
        return WAYZ_FAIL;
    }
// wlan0     Link encap:Ethernet  HWaddr 68:B9:D3:CF:A5:99
    while (NULL != fgets(str_line, STR_LEN, fp))
    {            
        if (NULL != strstr(str_line, "wlan"))
        {
            bzero(buf, sizeof (buf));
            sscanf(str_line, "wlan%d Link %s HWaddr %s", &wlan_index, temp, buf);
            if (0 != strlen(buf))
            {
                string_to_low(buf);
                memcpy(addr, buf, sizeof (buf));
            }
        }
    }
    pclose(fp); //关闭文件
    return WAYZ_OK;
}

static char get_wifi_chip_sta_mac_uuid(char *addr, char *uuid)
{
    FILE *fp;
    int ret = WAYZ_FAIL;
	char buf[20] = {0};
    char temp[40] = {0};
    char str_line[STR_LEN] = {0}; //每行最大读取的字符数,可根据实际情况扩大
    printf("get_wifi_chip_sta_mac_uuid start.\r\n");
    if((fp = popen("wpa_cli -i wlan0 status", "r")) == NULL) //判断文件是否存在及可读
    {
        printf("exce 'wpa_cli -iwlan0 status' failure!\r\n");
        return WAYZ_FAIL;
    }

    while (NULL != fgets(str_line, STR_LEN, fp))
    {
        if (NULL != strstr(str_line, "address"))
        {
            bzero(buf, sizeof (buf));
            sscanf(str_line, "address=%s", buf);
            if (0 != strlen(buf))
            {
                string_to_low(buf);
                memcpy(addr, buf, sizeof (buf));
                ret = WAYZ_OK;
            }
        }

        if (NULL != strstr(str_line, "uuid"))
        {
            bzero(temp, sizeof (temp));
            sscanf(str_line, "uuid=%s", temp);
            if (0 != strlen(temp))
            {
                memcpy(uuid, temp, sizeof (temp));
                ret = WAYZ_OK;
            }
            break;
        }
    }
    printf("get_wifi_chip_sta_mac_uuid end.\r\n");
    pclose(fp); //关闭文件
    return ret;
}

void get_wifi_sta_mac(twifi_chip_info *pchip_info)
{
    char err = 0;
    err = get_wifi_chip_sta_mac_uuid(pchip_info->sta_addr, pchip_info->uuid);
    if (WAYZ_FAIL == err)
    {
        get_wifi_sta_chip_mac(pchip_info->sta_addr);
    }
}

#endif

