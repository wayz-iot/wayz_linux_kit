#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

#include "dev_client.h"
#include "wayz_pos.h"


#define  WAYZ_FAIL      -1
#define  WAYZ_OK        0

#define  DEV_NAME       "LINUX"
#define  VENDER         "ALIENTEK"
#define  PRODUCT        "STM32MP157D"
#define  SN             "1234567"
#define  TENANT         "WAYZ"

#define  POINT_FRQ      (3000 * 1000)

#define  ACCESS_KEY     "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX" // 需要申请


static void location_client_entry(void *parament)
{
    twifi_chip_info wifi_chip;
    char ret = 0;
    get_wifi_sta_mac(&wifi_chip);
    printf("station mac : %s \r\nuuid: %s \r\n", wifi_chip.sta_addr, wifi_chip.uuid);

    tdeviec_info *dev_info;

    wlan_info = wifi_param_init(WAYZ_WIFI_SSID, WAYZ_WIFI_PWD);
    dev_info = dev_para_init(DEV_NAME, VENDER, PRODUCT, SN, TENANT);

    ret = dev_register_init(dev_info, ACCESS_KEY);
    if (ret != DEV_REGISTER_OK)
    {
        printf("\033[31;22mdevice register failure. \033[0m\n");
        return ;
    }

    tlocation_info location = {0};

    ret = get_position_info(ACCESS_KEY, NULL, &location); 
    if (WAYZ_FAIL == ret)
    {
        printf("\033[31;22mthe device failed to obtain latitude and longitude information.\033[0m\n");
    }
    else
    {
        location_print(location);
    }

    while (1)
    {
        ret = get_position_info(ACCESS_KEY, NULL, &location); 
        if (WAYZ_FAIL == ret)
        {
            printf("\033[31;22mthe device failed to obtain latitude and longitude information.\033[0m\n");
        }
        else
        {
            location_print(location);
        }
        usleep(POINT_FRQ);
        memset(&location, 0, sizeof (location));
    }

}

int main(void)
{
    int ret = 0;
    pthread_t tid;

    ret = pthread_create(&tid, NULL, (void *)location_client_entry, NULL);
    if(ret != 0){
        fprintf(stderr,"Fail to pthread_create : %s\n",strerror(ret));
        exit(EXIT_FAILURE);
    }

    pthread_detach(tid);

    while (1); // 防止主线程退出

    return 0;
}

