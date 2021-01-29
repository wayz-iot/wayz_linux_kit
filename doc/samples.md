
# wayz_linux_iotkit 示例程序 #

| 示例程序路径                   | 说明 |
| ----                          | ---- |
| examples/location_client.c.c  | wayz_linux_iotkit 测试例程 |

**注意：设备需要连接网络，内部需要提供wpa_supplicant和iw命令**

## 运行示例 ##

## 1、申请ACCESS_KEY

- 需要在平台申请ACCESS_KEY，目前只能提供人工申请的方式
在示例代码中的宏中填写申请的ACCESS_KEY字段，例如：
```c
#define  ACCESS_KEY     "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX"
```
## 2、填写设备相关信息
修改设备相关信息，例如：
```c
#define  DEV_NAME       "STM32MP1"      // 设备名称
#define  VENDER         "ALIENTEK"	   // 设备生产商
#define  PRODUCT        "FINDU01"	   // 设备所属产品
#define  SN             "1234567"	   // 产品序列号
#define  TENANT         "WAYZ"		   // 设备所属租户，通常是开放平台的用户 ID
```

## 3、GNSS、基站数据的填充

```c
tpost_data post_data = {0};					// 定义上传数据结构体

post_data.gnss.timestamp = 1606729066000;
post_data.gnss.lng = 114.39583641301239;
post_data.gnss.lat = 30.51769862171484;
post_data.gnss.accuracy = 8;

post_data.cellulars.count = 1;
post_data.cellulars.cell[0].timestamp = 1515743846504;
post_data.cellulars.cell[0].cell_id = 149833211;
sprintf(post_data.cellulars.cell[0].radio_type, "%s", "gsm");
post_data.cellulars.cell[0].mcc = 460;
post_data.cellulars.cell[0].mnc = 11;
post_data.cellulars.cell[0].lac = 36558;

// 传入获取定位函数中
get_position_info(ACCESS_KEY, &post_data, &location); 
```

## 4、在`terminal`命令行中运行示例代码


## 示例结果 ##
```c
timestamp: 1606293694990 ms
gcj02:
	latitude:30.515105
	longitude:114.401555
wgs84:
	latitude:30.517407914397
	longitude:114.396014616712
POI: {"id": "7SkEZdfXQfS","type": "Residential","name": "中建东湖明珠国际公馆","categories":[{"id": 10200,"name": "住宅"}]}
```