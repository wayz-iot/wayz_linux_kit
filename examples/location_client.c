#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

#include "dev_client.h"
#include "wayz_pos.h"
#include "wayz_log.h"


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
    WAYZ_LOGD("station mac : %s \r\nuuid: %s ", wifi_chip.sta_addr, wifi_chip.uuid);

    tdeviec_info *dev_info;

    dev_info = dev_para_init(DEV_NAME, VENDER, PRODUCT, SN, TENANT);

    ret = dev_register_init(dev_info, ACCESS_KEY);
    if (ret != DEV_REGISTER_OK)
    {
        WAYZ_LOGE("device register failure.");
        return ;
    }

    tlocation_info location = {0};

    ret = get_position_info(ACCESS_KEY, NULL, &location); 
    if (WAYZ_FAIL == ret)
    {
        WAYZ_LOGE("the device failed to obtain latitude and longitude information.");
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
            WAYZ_LOGE("the device failed to obtain latitude and longitude information.");
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

