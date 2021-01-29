# wayz_linux_iotkit API


## wayz_pos.h
```c
/**
 * device parameter initialized
 * 
 * @param dev_name: device name
 * 
 * @param manufacturer: device manufacturer
 * 
 * @param product: product name
 * 
 * @param SN: product serial number
 * 
 * @param tenant: tenant device
 * 
 * @return tdeviec_info: Dynamic allocation device info structure
*/
tdeviec_info *dev_para_init(const char *dev_name, const char *manufacturer, const char *product, \
                const char *SN, const char *tenant);

/**
 * Connected to the Internet to register
 *
 * @param dev_info: device info ,(dev_name、manufacturer、SN、product、tenant)
 * 
 * @param key: Visiting the website key
 *
 * @return =0: wifi connect failure
 *         =1: device register success
 *         =2: device register failure
 */
char dev_register_init(tdeviec_info *dev_info, char *key);

/**
 * Get the positioning result function
 * 
 * @param key Visiting the website key
 * 
 * @param post_data post gnss and cellulars data, obtain positioning results
 * 
 * @param location get location result
 * 
 * @return >0: success
 *         =0: location failure
 * 
*/
char get_position_info(char *key, tpost_data *post_data, tlocation_info *location);

/**
 * print location result
 * 
 * @param location location info
 * 
 * @return void:
 * 
*/
void location_print(tlocation_info location);
```

## dev_client.h

```c
/**
 * linux scan ap info handle function
 * 
 * @param cmd: linux scan cmd
 * 
 * @param data: linux scan date
 * 
 * @return  > 0: linux scan ap count
 *          < 0: linux scan ap failure
 * 
*/
int scan_file_handle(char* cmd, tap_info *data);

/**
 * linux get wifi sta mac address function
 * 
 * @param pchip_info: linux wifi chip info (mac/uuid)
 * 
 * @return  void: 
 * 
*/
void get_wifi_sta_mac(twifi_chip_info *pchip_info);
```