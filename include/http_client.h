
#ifndef  __HTTP_CLIENT_H_
#define  __HTTP_CLIENT_H_


int webclient_request(const char *URI, const char *host_server, const char *post_data, unsigned char **response);
long http_get_time(char *time_data);
char set_system_time(void);

#endif

