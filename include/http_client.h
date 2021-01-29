
#ifndef  __HTTP_CLIENT_H_
#define  __HTTP_CLIENT_H_

#define  WAYZ_FAIL      -1
#define  WAYZ_OK        0

int webclient_request(const char *URI, const char *host_server, const char *post_data, unsigned char **response);
long http_get_time(char *time_data);
char set_system_time(void);

#endif

