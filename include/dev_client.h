#ifndef  __DEV_CLIENT_H_
#define  __DEV_CLIENT_H_

#define  AP_MAX_NUMBER       100
#define  STR_LEN             256
#define  MAC_LEN             (18)
#define  UUID_LEN            (40)


#pragma pack(push)
#pragma pack(1)


typedef struct _ap_struct_Info_
{
    char mac[MAC_LEN]; // 48:3F:DA:59:15:80
    int rssi;
    int freq;
    char channel;
    char ssid[33];
}tap_struct_info;

typedef struct _ap_info_
{
    tap_struct_info tinfoAp[AP_MAX_NUMBER];
    int count;
    int length;
}tap_info;

typedef struct _wifi_chip_info_
{
    char sta_addr[MAC_LEN];
    char uuid[UUID_LEN];
}twifi_chip_info;

#pragma pack(pop)

char scan_file_handle(tap_info *data);
void get_wifi_sta_mac(twifi_chip_info *pchip_info);

#endif