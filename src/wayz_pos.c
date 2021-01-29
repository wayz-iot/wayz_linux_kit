
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <math.h>
#include "cJSON.h"
#include "http_client.h"
#include "dev_client.h"
#include "wayz_pos.h"


twifi_chip_info tchip_info = {0};
static char access_key[KEY_LENGTH] = {0};

#if 1 // 经纬度转换
const double a = 6378245.0;
const double ee = 0.00669342162296594323;
const double pi = 3.14159265358979324;

#define  FLOAT_EPS      1e-6
#define  DOUBLE_EPS     1e-15

static char check_double_equal_zero(double f)
{
    if(fabs(f) <= DOUBLE_EPS)
    {
        return WAYZ_OK;
    }
    else
    {
        return WAYZ_FAIL;
    }
}

static char check_float_equal_zero(float f)
{
    if(fabs(f) <= FLOAT_EPS)
    {
        return WAYZ_OK;
    }
    else
    {
        return WAYZ_FAIL;
    }
}

static unsigned char _outOfChina(double lat, double lon)
{
    if (lon < 72.004 || lon > 137.8347)
        return 1;
    if (lat < 0.8293 || lat > 55.8271)
        return 1;
    return 0;
}

static double _transformLat(double x, double y)
{
    double ret = -100.0 + 2.0 * x + 3.0 * y + 0.2 * y * y + 0.1 * x * y + 0.2 * sqrt(abs(x));
    ret += (20.0 * sin(6.0 * x * pi) + 20.0 * sin(2.0 * x * pi)) * 2.0 / 3.0;
    ret += (20.0 * sin(y * pi) + 40.0 * sin(y / 3.0 * pi)) * 2.0 / 3.0;
    ret += (160.0 * sin(y / 12.0 * pi) + 320 * sin(y * pi / 30.0)) * 2.0 / 3.0;
    return ret;
}

static double _transformLon(double x, double y)
{
    double ret = 300.0 + x + 2.0 * y + 0.1 * x * x + 0.1 * x * y + 0.1 * sqrt(abs(x));
    ret += (20.0 * sin(6.0 * x * pi) + 20.0 * sin(2.0 * x * pi)) * 2.0 / 3.0;
    ret += (20.0 * sin(x * pi) + 40.0 * sin(x / 3.0 * pi)) * 2.0 / 3.0;
    ret += (150.0 * sin(x / 12.0 * pi) + 300.0 * sin(x / 30.0 * pi)) * 2.0 / 3.0;
    return ret;
}

static void gps_transform( double wgLat, double wgLon, double * mgLat, double* mgLon)
{
    if (_outOfChina(wgLat, wgLon))
    {
        *mgLat = wgLat;
        *mgLon = wgLon;
        return;
    }
    double dLat = _transformLat(wgLon - 105.0, wgLat - 35.0);
    double dLon = _transformLon(wgLon - 105.0, wgLat - 35.0);
    double radLat = wgLat / 180.0 * pi;
    double magic = sin(radLat);
    magic = 1 - ee * magic * magic;
    double sqrtMagic = sqrt(magic);
    dLat = (dLat * 180.0) / ((a * (1 - ee)) / (magic * sqrtMagic) * pi);
    dLon = (dLon * 180.0) / (a / sqrtMagic * cos(radLat) * pi);
    *mgLat = wgLat + dLat;
    *mgLon = wgLon + dLon;
};

// static void wgs84togcj02(double wgLat, double wgLon, double * mgLat, double* mgLon)
// {
//     gps_transform(wgLat, wgLon, mgLat, mgLon);
// }

static void gcj02towgs84(double wgLat, double wgLon, double * mgLat, double* mgLon)
{
    gps_transform(wgLat, wgLon, mgLat, mgLon);
    *mgLat = wgLat * 2 - *mgLat;
    *mgLon = wgLon * 2 - *mgLon;
}

// static void gcj02tobd09(double wgLat, double wgLon, double * mgLat, double* mgLon)
// {
//     double x = wgLon, y = wgLat;
//     double z = sqrt(x * x + y * y) + 0.00002 * sin(y * pi);
//     double theta = atan2(y, x) + 0.000003 * cos(x * pi);
//     *mgLon = z * cos(theta) + 0.0065;
//     *mgLat = z * sin(theta) + 0.006;
// }
#endif


static void wayz_error(char* msg)
{
    printf("\033[31;22m[E/wayz]: ERROR %s\033[0m\r\n", msg); // Print the error message to stderr.
}

twifi_info *wifi_param_init(const char *ssid, const char *passwd)
{
    twifi_info *wlan_info = (twifi_info *) calloc(1, sizeof (twifi_info));
    if (NULL == wlan_info)
    {
        wayz_error("wifi param malloc fail.");
        return NULL;
    }

    wlan_info->ssid = strdup(ssid);
    wlan_info->passwd = strdup(passwd);

    return wlan_info;
}

tdeviec_info * dev_para_init(const char *dev_name, const char *manufacturer, const char *product, \
                const char *SN, const char *tenant)
{
    tdeviec_info *dev_info = (tdeviec_info *) calloc(1, sizeof (tdeviec_info));
    if (NULL == dev_info)
    {
        wayz_error("wifi param malloc fail.");
        return NULL;
    }

    dev_info->dev_name = strdup(dev_name);
    dev_info->manufacturer = strdup(manufacturer);
    dev_info->product = strdup(product);
    dev_info->SN = strdup(SN);
    dev_info->tenant = strdup(tenant);
    return dev_info;
}

// static char wifi_init(const char *ssid, const char *password)
// {
//     return WAYZ_FAIL;
// }

// static rt_uint16_t chnTofreq(uint8_t channel)
// {
//     return (FREQ_START + (channel - 1) * SEGMEMTATION);
// }

static char *point_cJson_handler(tpost_data *post_data, tap_info ap_info)
{
 /* declare a few. */
    cJSON *root = NULL;
    cJSON *fmt = NULL;
    cJSON *img = NULL;
    cJSON *thm = NULL;
    cJSON *fld = NULL;
    int i = 0, j = 0;
    char macBuf[MAC_LEN] = {0};
    char *cJsonBuffer = NULL;
    char *buffer = NULL;
    unsigned char state = 0;
    // char time_buf[25] = {0};
    char buftemp[40] = {0};
    char time_buf[20] = {0};

    root = cJSON_CreateObject();

    // time_t cur_time = 0;
    // uint64_t time = 0;
    struct timeval start;

    // cur_time = -1;//http_get_time(time_buf);
    // if (-1 == cur_time)
    // {
    //     // cur_time = wayz_get_time_by_ntp(NULL);
    // }
    
    gettimeofday(&start, NULL);
    // if (cur_time)
    // {
        // time = start.tv_sec * 1000 + (start.tv_usec / 1000);
        // time = start.tv_sec;
        sprintf(time_buf, "%ld%ld", start.tv_sec, start.tv_usec / 1000);
        // printf("cur_time: %lld, %lld \r\n", cur_time, time);
    //     pre_time = time;
    // }
    // else
    // {
    //     time = (pre_time == 0 ? TIMESTAMP_DEL : (pre_time + 5000 + (start.tv_usec / 1000)));
    // }

    if (0 == strlen(tchip_info.uuid))
    {
        sprintf(buftemp, "38efe26e-bcd8-%s", tchip_info.sta_addr);
    }
    else
    {
        sprintf(buftemp, "%s", tchip_info.uuid);
    }

    cJSON_AddNumberToObject(root, "timestamp", atoll(time_buf));
    cJSON_AddItemToObject(root, "id", cJSON_CreateString(buftemp));
    cJSON_AddItemToObject(root, "asset", fmt = cJSON_CreateObject());
    sprintf(macBuf, "%s", tchip_info.sta_addr);
    cJSON_AddItemToObject(fmt, "id", cJSON_CreateString(macBuf));
    cJSON_AddItemToObject(root, "location", img = cJSON_CreateObject());
    cJSON_AddNumberToObject(img, "timestamp", atoll(time_buf));
    // add wifis
    if (ap_info.count >= 4)
    {
        cJSON_AddItemToObject(img, "wifis", thm = cJSON_CreateArray());
        for (i = 0, j = 0; i < ap_info.count; i++)
        {
            sprintf(macBuf, "%s", ap_info.tinfoAp[i].mac);
            cJSON_AddItemToArray(thm, fld = cJSON_CreateObject());
            cJSON_AddStringToObject(fld, "macAddress", macBuf);
            // cJSON_AddStringToObject(fld, "ssid", "");
            // cJSON_AddNumberToObject(fld, "frequency", chnTofreq(ap_info.tinfoAp[i].channel));
            cJSON_AddNumberToObject(fld, "signalStrength", abs(ap_info.tinfoAp[i].rssi));
            j ++;
            if (j > 10)
            {
                //break;
            }
        }
        state = 1;
    }

    if (NULL != post_data)
    {
        // add gnss
        if (WAYZ_OK != check_double_equal_zero(post_data->gnss.lng) && 
            WAYZ_OK != check_double_equal_zero(post_data->gnss.lat) &&
            WAYZ_OK != check_float_equal_zero(post_data->gnss.accuracy))
        {
            cJSON_AddItemToObject(img, "gnss", thm = cJSON_CreateObject());
            cJSON_AddNumberToObject(thm, "timestamp", post_data->gnss.timestamp);
            cJSON_AddItemToObject(thm, "point", fld = cJSON_CreateObject());
            cJSON_AddNumberToObject(fld, "longitude", post_data->gnss.lng);
            cJSON_AddNumberToObject(fld, "latitude", post_data->gnss.lat);
            cJSON_AddNumberToObject(thm, "accuracy", post_data->gnss.accuracy);
            state = 1;
        }
        // add cellulars
        if (0 != post_data->cellulars.count)
        {
            cJSON_AddItemToObject(img, "cellulars", thm = cJSON_CreateArray());

            for (i = 0; i < post_data->cellulars.count; i ++)
            {
                cJSON_AddItemToArray(thm, fld = cJSON_CreateObject());
                cJSON_AddNumberToObject(fld, "timestamp", post_data->cellulars.cell[i].timestamp);
                cJSON_AddNumberToObject(fld, "cellId", post_data->cellulars.cell[i].cell_id);
                cJSON_AddStringToObject(fld, "radioType", post_data->cellulars.cell[i].radio_type);
                cJSON_AddNumberToObject(fld, "mobileCountryCode", post_data->cellulars.cell[i].mcc);
                cJSON_AddNumberToObject(fld, "mobileNetworkCode", post_data->cellulars.cell[i].mnc);
                cJSON_AddNumberToObject(fld, "locationAreaCode", post_data->cellulars.cell[i].lac);
            }
            state = 1;
        }
    }

    cJsonBuffer = cJSON_Print(root);
    // printf("@@@@@\r\n%s\r\n", cJsonBuffer);
    buffer = (char *)malloc(strlen(cJsonBuffer));
    if (NULL == buffer)
    {
        wayz_error("point_cJson_handler create malloc failure.");
        return NULL;
    }

    sprintf(buffer, "%s", cJsonBuffer);
    cJSON_Delete(root);
    free(cJsonBuffer);

    if (1 != state)
    {
        wayz_error("point_cJson_handler: location No wifi, GNSS, cellulars signal data");
        return NULL;
    }
    
    return buffer; // 需要free
}

/**
 * @param location: print location info
 * 
 * @warning printf no print float/double
 *          printf is ok
 * 
*/
void location_print(tlocation_info location)
{
    printf("-------------------location result-------------------------------\r\n");
    printf("timestamp: %s ms \r\n", location.timestamp);
    printf("gcj02:\r\n\tlatitude:%f\r\n\tlongitude:%f\r\n", location.point.gcj02.latitude, location.point.gcj02.longitude);
    printf("wgs84:\r\n\tlatitude:%0.12f\r\n\tlongitude:%0.12f\r\n", location.point.wgs84.latitude, location.point.wgs84.longitude);

    printf("POI: {\"id\": %s,\"type\": %s,\"name\": %s,\"categories\":[{\"id\": %s,\"name\": %s}]}\r\n", 
        location.place.id, location.place.type, location.place.name, location.place.category.id, location.place.category.name);
    printf("-------------------location result end---------------------------\r\n");
}

#if 1 // parse cjson/string handler

static char __parse_str_handler(const char *headbuf, const char *key, char *parse_data)
{
    char *ret = strstr(headbuf, key);
    int cnt = 0;
    char result = WAYZ_OK;
    if (NULL == ret)
    {
        result = WAYZ_FAIL;
        goto str_fail;
    }
    char *ret1 = strstr(ret, ":");
    char *ret2 = strstr(ret1, ",");
    memset(parse_data, 0, sizeof (NAME_LENGTH));
    cnt = ret2 - ret1 - 1;
    strncpy(parse_data, ret1 + 1, ret2 - ret1 - 1);
    parse_data[cnt] = '\0';
    // memcpy(parse_data, ret1 + 1, ret2 - ret1 - 1);
str_fail:
    return result;
}

static void updata_system_time(char *timestamp)
{
    char buf_time[20] = {0};
    uint64_t cur_time = 0;
    static char updata_time_flag = 0;
    
    if (updata_time_flag != 0)
    {
        return ;
    }
    memcpy(buf_time, timestamp, 10);

    cur_time = atoll(buf_time);
    printf("updata system time cur_time: %lld\r\n", cur_time);
    struct timeval now = { .tv_sec = cur_time };
    settimeofday(&now, NULL);
    updata_time_flag = 1;
}

static void parse_point_str_handler(char *data, tlocation_info *location)
{
    char *place = NULL;
    char *categories = NULL;
    double mgLat = 0;
    double mgLon = 0;
    char valueBuf[NAME_LENGTH] = {0};

    if (WAYZ_FAIL == __parse_str_handler(data, "longitude", valueBuf))
    {
        printf("post request for data failed. \r\n");
        return ;
    }
    location->point.gcj02.longitude = atof(valueBuf);
    if (WAYZ_FAIL == __parse_str_handler(data, "latitude", valueBuf))
    {
        return ;
    }
    location->point.gcj02.latitude = atof(valueBuf);
    gcj02towgs84(location->point.gcj02.latitude, location->point.gcj02.longitude, &mgLat, &mgLon);
    location->point.wgs84.latitude = mgLat;
    location->point.wgs84.longitude = mgLon;

    if (WAYZ_FAIL == __parse_str_handler(data, "timestamp", valueBuf))
    {
        return ;
    }
    memcpy(location->timestamp, valueBuf, sizeof(location->timestamp));
    // memset(valueBuf, 0, sizeof (valueBuf));
    // strncpy(valueBuf, location->timestamp, 10);
    // mgLat = atoll(valueBuf);
    // memset(valueBuf, 0, sizeof (valueBuf));
    // strncpy(valueBuf, location->timestamp + 10, 3);
    // mgLon = atoi(valueBuf);
    // pre_time = mgLat * 1000 + mgLon;

    place = strstr(data, "place");
    if (NULL == place)
    {
        return ;
    }

    if (WAYZ_FAIL == __parse_str_handler(place, "timestamp", valueBuf)) // 覆盖之前数据，表示最新数据
    {
        return ;
    }
    memcpy(location->timestamp, valueBuf, sizeof(location->timestamp));
    updata_system_time(location->timestamp);

    if (WAYZ_FAIL == __parse_str_handler(place, "id", valueBuf))
    {
        return ;
    }
    memcpy(location->place.id, valueBuf, sizeof (location->place.id));

    if (WAYZ_FAIL == __parse_str_handler(place, "type", valueBuf))
    {
        return ;
    }
    memcpy(location->place.type, valueBuf, strlen(valueBuf));

    if (WAYZ_FAIL == __parse_str_handler(place, "name", valueBuf))
    {
        return ;
    }
    memcpy(location->place.name, valueBuf, sizeof(location->place.name));

    categories = strstr(data, "categories");
    if (NULL == categories)
    {
        return ;
    }

    if (WAYZ_FAIL == __parse_str_handler(categories, "id", valueBuf))
    {
        return ;
    }
    memcpy(location->place.category.id, valueBuf, strlen(valueBuf));

    if (WAYZ_FAIL == __parse_str_handler(categories, "name", valueBuf))
    {
        return ;
    }
    memcpy(location->place.category.name, valueBuf, strlen(valueBuf));
}

/**
 * parse point cjson handler
*/
static unsigned char parse_point_cJson_handler(char *data, tlocation_info *location)
{
    cJSON *root, *object, *position, *point, *place, *category;
	cJSON *item;
	cJSON *temp = NULL;
    char *buffer = NULL;
    double mgLat = 0;
    double mgLon = 0;
    unsigned char ret = WAYZ_OK;
    
    root = cJSON_Parse(data);
    if (!root) {
        printf("\033[31;22m\r\n[E/wayz]Error cJSON_Parse root failure.\033[0m\n");
        goto _root_fail;
    }

    object = cJSON_GetObjectItem(root, "location");
    if (!object)
    {
        wayz_error("object failure\r\n");
        goto _root_fail;
    }
    
    position = cJSON_GetObjectItem(object, "position");
    if (!position)
    {
        wayz_error("position failure\r\n");
        goto _root_fail;
    }

    item = cJSON_GetObjectItem(position, "timestamp");
    if (item)
    {
        buffer = cJSON_Print(item);
        memset(location->timestamp, 0, sizeof (location->timestamp));
        sprintf(location->timestamp, "%s", buffer);
        updata_system_time(location->timestamp);
        free(buffer);
        buffer = NULL;
        // char valueBuf[NAME_LENGTH] = {0};
        // memset(valueBuf, 0, sizeof (valueBuf));
        // strncpy(valueBuf, location->timestamp, 10);
        // mgLat = atoll(valueBuf);
        // memset(valueBuf, 0, sizeof (valueBuf));
        // strncpy(valueBuf, location->timestamp + 10, 3);
        // mgLon = atoi(valueBuf);
        // pre_time = mgLat * 1000 + mgLon;
        
    }

    point = cJSON_GetObjectItem(position, "point");
    if (!point)
    {
        wayz_error("point failure\r\n");
        goto _root_fail;
    }

    item = cJSON_GetObjectItem(point, "longitude");
    if (item)
    {
        buffer = cJSON_Print(item);
        location->point.gcj02.longitude = atof(buffer);
        free(buffer);
        buffer = NULL;
        // printf("longitude: %s, %3.6f\r\n", cJSON_Print(item), location->point.gcj02.longitude);
        item = cJSON_GetObjectItem(point, "latitude");
        buffer = cJSON_Print(item);
        location->point.gcj02.latitude = atof(buffer);
        free(buffer);
        buffer = NULL;
    
        gcj02towgs84(location->point.gcj02.latitude, location->point.gcj02.longitude, &mgLat, &mgLon);
        location->point.wgs84.latitude = mgLat;
        location->point.wgs84.longitude = mgLon;
    }
    
    place = cJSON_GetObjectItem(object, "place");
    if (!place)
    {
        wayz_error("place failure\r\n");
        goto _root_fail;
    }

    item = cJSON_GetObjectItem(place, "id");
    if (item)
    {
        buffer = cJSON_Print(item);
        memset(location->place.id, 0, sizeof (location->place.id));
        sprintf(location->place.id , "%s", buffer);
        free(buffer);
        buffer = NULL;
    }

    item = cJSON_GetObjectItem(place, "type");
    if (item)
    {
        buffer = cJSON_Print(item);
        memset(location->place.type, 0, sizeof (location->place.type));
        sprintf(location->place.type , "%s", buffer);
        free(buffer);
        buffer = NULL;
    }

    item = cJSON_GetObjectItem(place, "name");
    if (item)
    {
        buffer = cJSON_Print(item);
        memset(location->place.name, 0, sizeof (location->place.name));
        sprintf(location->place.name , "%s", buffer);
        free(buffer);
        buffer = NULL;
    }

    category = cJSON_GetObjectItem(place, "categories");
    if (category)
    {
        temp = cJSON_GetArrayItem(category, 0);
        if (temp)
        {
            item = cJSON_GetObjectItem(temp, "id");
            if (item)
            {
                buffer = cJSON_Print(item);
                memset(location->place.category.id, 0, sizeof (location->place.category.id));
                sprintf(location->place.category.id , "%s", buffer);
                free(buffer);
                buffer = NULL;
            }

            item = cJSON_GetObjectItem(temp, "name");
            if (item)
            {
                buffer = cJSON_Print(item);
                memset(location->place.category.name, 0, sizeof (location->place.category.name));
                sprintf(location->place.category.name , "%s", buffer);
                free(buffer);
                buffer = NULL;
            }
        }
    }
    
    cJSON_Delete(root);
    return ret;
_root_fail:
    parse_point_str_handler(data, location);
    cJSON_Delete(root);
    return ret;
}

#endif

static char * register_dev_cjson_handler(tdeviec_info *dev_info)
{
    cJSON *root = NULL;
    cJSON *fmt = NULL;
    cJSON *img = NULL;
    char macBuf[MAC_LEN] = {0};
    char *cJsonBuffer = NULL;
    char *buffer = NULL;
    // time_t cur_time;
    // uint64_t time = 0;
    char temp[10] = {0};
    struct  timeval start;
    char time_buf[20] = {0};

    root = cJSON_CreateArray();

    sprintf(temp, "%01d.%01d.%01d", VER_H, VER_M, VER_L);
    sprintf(macBuf, "%s", tchip_info.sta_addr);
    cJSON_AddItemToArray(root, fmt = cJSON_CreateObject());
    cJSON_AddStringToObject(fmt, "id", macBuf);
    cJSON_AddStringToObject(fmt, "name", dev_info->dev_name);
    cJSON_AddStringToObject(fmt, "manufacturer", dev_info->manufacturer);
    cJSON_AddStringToObject(fmt, "macAddress", macBuf);
    cJSON_AddStringToObject(fmt, "serialNumber", dev_info->SN);
    cJSON_AddItemToObject(fmt, "firmware", img = cJSON_CreateObject());
    cJSON_AddStringToObject(img, "version", temp);

    set_system_time();
    
    // cur_time = http_get_time(time_buf);
    // if (-1 == cur_time)
    // {
    //     // cur_time = wayz_get_time_by_ntp(NULL);
    // }
    
    gettimeofday(&start, NULL);
    // if (cur_time)
    // {
        // time = start.tv_sec * 1000 + (start.tv_usec / 1000);
        // time = start.tv_sec;
        sprintf(time_buf, "%ld%ld", start.tv_sec, start.tv_usec / 1000);
        // printf("cur_time: %ld, %ld \r\n", cur_time, time);
        // pre_time = time;
    // }
    // else
    // {
    //     time = (pre_time == 0 ? TIMESTAMP_DEL : (pre_time + 5000 + (start.tv_usec / 1000)));
    // }

    // printf("pre_time: %ld, %ld \r\n", pre_time, time);
    // cJSON_AddNumberToObject(fmt, "createTime", time);
    // cJSON_AddNumberToObject(fmt, "updateTime", time);
    cJSON_AddNumberToObject(fmt, "manufactureTime", atoll(time_buf));
    cJSON_AddStringToObject(fmt, "product", dev_info->product);
    cJSON_AddStringToObject(fmt, "tenant", dev_info->tenant);

    cJsonBuffer = cJSON_Print(root);
    buffer = (char *)malloc(strlen(cJsonBuffer));
    if (NULL == buffer)
    {
        printf("create malloc failure.\r\n");
        return NULL;
    }

    sprintf(buffer, "%s", cJsonBuffer);

    cJSON_Delete(root);
    free(cJsonBuffer);

    return buffer; // need free
}

#if 1 // webclient get/post interface
static unsigned char * wayz_webclient_get_data(const char *url)
{
    unsigned char *buffer = NULL;
    int length = 0;
    
    length = webclient_request(url, HOST_SERVER, NULL, &buffer);
    if (length < 0)
    {
        wayz_error("webclient GET request response data error.");
        free(buffer);
        return NULL;
    }

    return buffer;
}

/* HTTP client upload data to server by POST request */
static unsigned char * wayz_webclient_post_data(const char *URI, const char *post_data)
{
    unsigned char *buffer = NULL;
    int length = 0;

    length = webclient_request(URI, HOST_SERVER, post_data, &buffer);
    if (length < 0)
    {
        printf("\033[31;22m[E/wayz]:webclient POST request response data error.\033[0m\r\n");
        free(buffer);
        return NULL;
    }

    return buffer;
}

#endif

#if 1 // register device handle function
static char query_device()
{
    char *url = NULL;
    char mac_addr[MAC_LEN] = {0};
    unsigned char *buffer = NULL;
    char *temp = NULL;
    char result = 0;
    url = (char *)malloc(strlen(DEV_QUERY_URL) + MAC_LEN + strlen(access_key));
    if (NULL == url)
    {
        result = WAYZ_FAIL;
        goto _malloc_fail;
    }
    memset(url, 0, strlen(DEV_QUERY_URL) + MAC_LEN + strlen(access_key));
    sprintf(mac_addr, "%s", tchip_info.sta_addr);
    sprintf(url, ""DEV_QUERY_URL, mac_addr, access_key);
    buffer = wayz_webclient_get_data(url);
    if (NULL == buffer)
    {
        printf("\033[31;22m[E/wayz]: visiting %s failure\033[0m\r\n", url);
        result = WAYZ_FAIL;
        goto _url_fail;
    }

    temp = strstr((char *)buffer, mac_addr);
    if (NULL == temp)
    {
        result = WAYZ_FAIL;
        goto _query_fail;
    }
    result = WAYZ_OK;

_query_fail:
    free(buffer);
_url_fail:
    free(url);
    url = NULL;
_malloc_fail:
    return result;
}

static char register_device(tdeviec_info *dev_info)
{
    char *url = NULL;
    unsigned char *buffer = NULL;
    char *temp = NULL;
    char result = 0;
    char *cJsonBuffer = NULL;
    char mac_addr[MAC_LEN] = {0};
    url = (char *)malloc(strlen(DEV_REGISTER_URL) + strlen(access_key));
    if (NULL == url)
    {
        result = WAYZ_FAIL;
        goto _malloc_fail;
    }
    memset(url, 0, strlen(DEV_REGISTER_URL) + strlen(access_key));
    sprintf(url, ""DEV_REGISTER_URL, access_key);
    cJsonBuffer = register_dev_cjson_handler(dev_info);
    if (NULL == cJsonBuffer)
    {
        printf("\033[31;22m[E/wayz]: register_dev_cjson_handler failure\033[0m\r\n");
        result = WAYZ_FAIL;
        goto _url_fail;
    }
    
    buffer = wayz_webclient_post_data(url, cJsonBuffer);
    if (NULL == buffer)
    {
        printf("\033[31;22m[E/wayz]: visiting http failure\033[0m\r\n");
        result = WAYZ_FAIL;
        goto _url_fail;
    }
    sprintf(mac_addr, "%s", tchip_info.sta_addr);
    temp = strstr((char *)buffer, mac_addr);
    if (NULL == temp)
    {
        result = WAYZ_FAIL;
        printf("\033[31;22m[E/wayz]: register buffer %s\033[0m\r\n", buffer);
        goto _query_fail;
    }
    result = WAYZ_OK;

_query_fail:
    free(buffer);
    buffer = NULL;
_url_fail:
    free(url);
    free(cJsonBuffer);
    url = NULL;
    cJsonBuffer = NULL;
_malloc_fail:
    return result;
}

static void dev_free(tdeviec_info *dev_info)
{
    free(dev_info->dev_name);
    free(dev_info->manufacturer);
    free(dev_info->SN);
    free(dev_info->product);
    free(dev_info->tenant);
}

/**
 * Connected to the Internet to register
 *
 * @param dev_info device info ,(dev_name、manufacturer、SN[SN_LENGTH]、product[NAME_LENGTH]、tenant)
 * 
 * @param key Visiting the website key
 *
 * @return =0: wifi connect failure
 *         =1: device register success
 *         =2: device register failure
 */
char dev_register_init(tdeviec_info *dev_info, char *key)
{
    get_wifi_sta_mac(&tchip_info);
    if (NULL == key && 0 == strcmp(key, ""))
    {
        wayz_error("Visiting the website key failure");
    }
    else
    {
        memcpy(access_key, key, strlen(key));
    }
    
    // 1 Check whether the device is registered
    if (WAYZ_OK == query_device())
    {
        wayz_error("the device has been registered, no need to register again");
        return DEV_REGISTER_OK;
    }
    // 2 registered handler
    if (WAYZ_OK == register_device(dev_info))
    {
        printf("register device success.\r\n");
    }
    else
    {
        wayz_error("register device failure.");
        return DEV_REGISTER_FAIL;
    }

    dev_free(dev_info);
    return DEV_REGISTER_OK;
}
#endif

/**
 * Get the positioning result function
 * 
 * @param key Visiting the website key
 * 
 * @param location get location result
 * 
 * @return >0: success
 *         =0: location failure
 * 
*/
char get_position_info(char *key, tpost_data *post_data, tlocation_info *location)
{
    char *url = NULL;
    unsigned char *buffer = NULL;
    char result = WAYZ_OK;
    char *cJsonBuffer = NULL;
    tap_info ap_infos = {0};
    get_wifi_sta_mac(&tchip_info);
    if (NULL == key && 0 == strcmp(key, ""))
    {
        wayz_error("Visiting the website key failure");
    }
    else
    {
        memcpy(access_key, key, strlen(key));
    }
    // 1 scan wifi info 
    scan_file_handle(&ap_infos);

    // 2 Upload data to get positioning results
    url = (char *)malloc(strlen(DEV_POSTION_URL) + strlen(access_key));
    if (NULL == url)
    {
        result = WAYZ_FAIL;
        goto _malloc_fail;
    }

    memset(url, 0, strlen(DEV_POSTION_URL) + strlen(access_key));
    sprintf(url, ""DEV_POSTION_URL, access_key);
    cJsonBuffer = point_cJson_handler(post_data, ap_infos);
    if (NULL == cJsonBuffer)
    {
        wayz_error("get_position_info: location No wifi, GNSS, cellulars signal data");
        result = WAYZ_FAIL;
        goto _url_fail;
    }

    buffer = wayz_webclient_post_data(url, cJsonBuffer);
    if (NULL == buffer)
    {
        printf("\033[31;22m[E/wayz]:get_position_info visiting http failure\033[0m\r\n");
        result = WAYZ_FAIL;
    }
    free(cJsonBuffer);
    cJsonBuffer = NULL;

    if (NULL != buffer)
    {
        parse_point_cJson_handler((char *)buffer, location);

        free(buffer);
        buffer = NULL;
    }

_url_fail:
    free(url);
    url = NULL;
_malloc_fail:

    return result;
}


