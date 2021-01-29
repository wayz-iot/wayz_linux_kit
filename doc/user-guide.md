# 使用指南

这里主要介绍 wayz_linux_iotkit 程序的基本使用流程，并针对使用过程中经常涉及到的结构体和重要 API 进行简要说明。

wayz_linux_iotkit 的基本工作流程如下所示：

- 初始化设备相关信息
- 将设备注册到平台上
- 获取定位结果(gcj02和wgs84标准)以及POI信息

## 工作原理
设备中wifi模块获取周围环境中的wifi信息，组包通过http方式上传到WAYZ定位云平台，云平台进行分析后将经纬度等其他信息一并返回到软件包，软件包通过处理可以得到gcj02和wgs84标准的经纬度，和POI信息。也可以通过传入gnss、基站等相关数据获取定位结果及其POI信息

## wifi、设备相关初始化
```c
typedef struct _device_info_            // 设备信息
{
    char *dev_name;                     // 设备名称
    char *manufacturer;                 // 设备制造厂家
    char *SN;                           // 设备序列号
    char *product;                      // 设备所属产品
    char *tenant;                       // 设备所属租户，通常是开放平台的用户 ID
}tdeviec_info;

`tdeviec_info` 用于保存注册到平台设备信息，在设备注册使用。用户在使用连接会话前，必须定义一个存储会话内容的结构体，如下所示：

```c
tdeviec_info *dev_info;
dev_info = dev_para_init(DEV_NAME, VENDER, PRODUCT, SN, TENANT);
```

## 设备注册

应用程序使用`dev_register_init`函数注册设备到平台。**其中ACCESS_KEY需要在平台申请**

示例代码如下所示：
```c
ret = dev_register_init(wlan_info, dev_info, ACCESS_KEY);
if (ret != DEV_REGISTER_OK)
{
    rt_kprintf("\033[31;22mdevice register failure. \033[0m\n");
    return ;
}
```

## 填充GNSS、基站信息定位
```c
typedef struct _gnss_unit_
{
    uint64_t timestamp;         // 数据收集的时间戳（UTC 时间，单位：毫秒）
    double lng;                 // 经度
    double lat;                 // 纬度 
    float accuracy;             // 卫星定位水平精度，单位：米
}tgnss_unit;

typedef struct _cell_unit_
{
    uint64_t timestamp;         // 数据收集的时间戳（UTC 时间，单位：毫秒）
    uint32_t cell_id;           // 小区 ID，当 CDMA 时，为 BID（Base Station ID）
    char radio_type[7];         // 基站类型，只能是以下值：gsm, wcdma, lte, cdma
    uint32_t mcc;               // mobileCountryCode：MCC 码
    uint32_t mnc;               // mobileNetworkCode：当 CDMA 时，为 SID（System ID）码
    uint32_t lac;               // locationAreaCode：当 CDMA 时，为 NID（Network ID）；
                                                     当 LTE 时，为 TAC（Tracking Area code）
}tcell_unit;
```
通过填充GNSS、基站等数据，传入定位接口即可获取定位结果信息


## 获取定位结果

应用程序使用`get_position_info`函数从平台端获取位置信息。**其中ACCESS_KEY需要在平台上申请**
`location_print`函数是打印位置相关信息

示例代码如下所示：
```c
tlocation_info location = {0};
ret = get_position_info(ACCESS_KEY, RT_NULL, &location); 
if (WAYZ_FAIL == ret)
{
    printf("\033[31;22mthe device failed to obtain latitude and longitude information.\033[0m\n");
}
else
{
    location_print(location);
}
```
- 其中`get_position_info`函数第三个参数为填充的`GNSS和基站数据`，相关操作可以参照[示例文档](docs/samples.md)

**打印位置信息结果**
```c
-------------------location result-------------------------------
timestamp: 1606293694990 ms
gcj02:
	latitude:30.515105
	longitude:114.401555
wgs84:
	latitude:30.517407914397
	longitude:114.396014616712
POI: {"id": "7SkEZdfXQfS","type": "Residential","name": "中建东湖明珠国际公馆","categories":[{"id": 10200,"name": "住宅"}]}
-------------------location result end---------------------------
```

## 定位轮询频次

该引用可以通过循环的模式来设置定位频次，**其中POINT_FRQ为定位频次的设置**

示例代码如下所示：
```c
while (1)
{
    ret = get_position_info(wlan_info, ACCESS_KEY, RT_NULL, &location); 
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
```


