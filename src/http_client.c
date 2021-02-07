#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "http_client.h"
#include "wayz_log.h"


#if 1  // get/post http
static const char *GET_REQUEST = "GET %s HTTP/1.0\r\n"
    "Host: %s\r\n"
    "\r\n";

static const char *POST_REQUEST = "POST %s HTTP/1.0\r\n"
    "Host: %s\r\n"
    "Content-Length: %d\r\n"
    "\r\n"
    "%s";

static int read_until(const char *buffer, char delim, int len)
{
    int i = 0;
    while (buffer[i] != delim && i < len) 
    {
        ++i;
    }
    return i + 1;
}

static int _esp_parse_http(const char *text, size_t total_len, size_t *parse_len)
{
    /* i means current position */
    int i = 0, i_read_len = 0;
    char *ptr = NULL, *ptr2 = NULL;
    char length_str[32];
    int length = 0;

    if ((ptr = (char *)strstr(text, "Content-Length")) != NULL) 
    {
        ptr += 16;
        ptr2 = (char *)strstr(ptr, "\r\n");
        memset(length_str, 0, sizeof(length_str));
        memcpy(length_str, ptr, ptr2 - ptr);
    }
    length = atoi(length_str);
    while (text[i] != 0 && i < total_len) {
        i_read_len = read_until(&text[i], '\n', total_len - i);

        if (i_read_len > total_len - i) {
            return length;
        }

        // if resolve \r\n line, http header is finished
        if (i_read_len == 2) {
            *parse_len = i + 2;

            return length;
        }

        i += i_read_len;
    }

    return length;
}

/**
 *  send request(GET/POST) to server and get response data.
 *
 * @param URI input server address
 * @param host_server send header data
 *             = NULL: use default header data, must be GET request
 *            != NULL: user custom header data, GET or POST request
 * @param post_data data sent to the server
 *             = NULL: it is GET request
 *            != NULL: it is POST request
 * @param response response buffer address
 *
 * @return <0: request failed
 *        >=0: response buffer size
 */
int webclient_request(const char *URI, const char *host_server, const char *post_data, unsigned char **response)
{
    const struct addrinfo hints = {
        .ai_family = AF_INET,
        .ai_socktype = SOCK_STREAM,
    };
    struct addrinfo *res;
    struct in_addr *addr;
    int s, r;
    char recv_buf[4096];
    char *post_buf = NULL;
    // int i = 0;

    int err = getaddrinfo(host_server, "80", &hints, &res);

    if(err != 0 || res == NULL) 
    {
        WAYZ_LOGE("DNS lookup failed err=%d res=%p.", err, res);
        return WAYZ_FAIL;
    }

    /* Code to print the resolved IP.
       Note: inet_ntoa is non-reentrant, look at ipaddr_ntoa_r for "real" code */
    addr = &((struct sockaddr_in *)res->ai_addr)->sin_addr;
    WAYZ_LOGD("DNS lookup succeeded. IP=%s\r\n", inet_ntoa(*addr));

    s = socket(res->ai_family, res->ai_socktype, 0);
    if(s < 0) {
        WAYZ_LOGE("... Failed to allocate socket.");
        freeaddrinfo(res);
        return WAYZ_FAIL;
    }
    WAYZ_LOGD("... allocated socket");

    if(connect(s, res->ai_addr, res->ai_addrlen) != 0) {
        WAYZ_LOGE("... socket connect failed errno");
        close(s);
        freeaddrinfo(res);
        return WAYZ_FAIL;
    }

    WAYZ_LOGD("... connected\r\n");
    freeaddrinfo(res);

    if (NULL == post_data)
    {
        sprintf(recv_buf, GET_REQUEST, URI, host_server);
        if (write(s, recv_buf, strlen(recv_buf)) < 0) {
            WAYZ_LOGE("... socket send failed");
            close(s);
            return WAYZ_FAIL;
        }
    }
    else
    {
        post_buf = (char *) malloc(strlen(POST_REQUEST) + strlen(URI) + strlen(host_server) + strlen(post_data) + 10);
        sprintf(post_buf, POST_REQUEST, URI, host_server, strlen(post_data), post_data);
        if (write(s, post_buf, strlen(post_buf)) < 0) {
            WAYZ_LOGE("... socket send failed");
            close(s);
            return WAYZ_FAIL;
        }

        free(post_buf);
        post_buf = NULL;
    }

    WAYZ_LOGD("... socket send success");

    struct timeval receiving_timeout;
    receiving_timeout.tv_sec = 5;
    receiving_timeout.tv_usec = 0;
    if (setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &receiving_timeout, sizeof(receiving_timeout)) < 0) {
        WAYZ_LOGE("... failed to set socket receiving timeout");
        close(s);
        return WAYZ_FAIL;
    }
    WAYZ_LOGD("... set socket receiving timeout success");

    /* Read HTTP response */
    bzero(recv_buf, sizeof(recv_buf));
    
    r = read(s, recv_buf, sizeof(recv_buf) - 1);
    if (r < 0)
    {
        WAYZ_LOGE("webclient_request data failure.");
        return WAYZ_FAIL;
    }

    for(int i = 0; i < r; i++) {
        putchar(recv_buf[i]);
    }
    close(s);
    WAYZ_LOGD("... done reading from socket. Last read return=%d.", r);
    size_t parse_len = 0;
    int length = 0;
    length = _esp_parse_http(recv_buf, r, &parse_len);
    *response = NULL;
    unsigned char *response_buf = NULL;
    response_buf = (unsigned char *)malloc(r - parse_len);
    if (NULL == response_buf)
    {
        WAYZ_LOGE("webclient_request malloc failure.");
        return WAYZ_FAIL;
    }
    memset(response_buf, 0, length);
// WAYZ_LOGD("----------------\r\n");
    memcpy(response_buf, recv_buf + parse_len, length);
    *response = response_buf;
    // for(int i = 0; i < length; i++) {
    //     putchar(response_buf[i]);
    // }
    return length;
}
#endif

#if 1 // ntp time request


#ifdef NETUTILS_NTP_TIMEZONE
#define NTP_TIMEZONE                   NETUTILS_NTP_TIMEZONE
#endif

#ifdef NETUTILS_NTP_HOSTNAME
#define NTP_HOSTNAME                   NETUTILS_NTP_HOSTNAME
#endif

#define NTP_TIMESTAMP_DELTA            2208988800ull
#define NTP_GET_TIMEOUT                1

#ifndef NTP_TIMEZONE
#define NTP_TIMEZONE                   8
#endif

#ifndef NTP_HOSTNAME
#define NTP_HOSTNAME                   "cn.pool.ntp.org"//"cn.ntp.org.cn"
#endif

#define LI(packet)   (uint8_t) ((packet.li_vn_mode & 0xC0) >> 6) // (li   & 11 000 000) >> 6
#define VN(packet)   (uint8_t) ((packet.li_vn_mode & 0x38) >> 3) // (vn   & 00 111 000) >> 3
#define MODE(packet) (uint8_t) ((packet.li_vn_mode & 0x07) >> 0) // (mode & 00 000 111) >> 0

// Structure that defines the 48 byte NTP packet protocol.
typedef struct {

    uint8_t li_vn_mode;      // Eight bits. li, vn, and mode.
                         // li.   Two bits.   Leap indicator.
                         // vn.   Three bits. Version number of the protocol.
                         // mode. Three bits. Client will pick mode 3 for client.

    uint8_t stratum;         // Eight bits. Stratum level of the local clock.
    uint8_t poll;            // Eight bits. Maximum interval between successive messages.
    uint8_t precision;       // Eight bits. Precision of the local clock.

    uint32_t rootDelay;      // 32 bits. Total round trip delay time.
    uint32_t rootDispersion; // 32 bits. Max error aloud from primary clock source.
    uint32_t refId;          // 32 bits. Reference clock identifier.

    uint32_t refTm_s;        // 32 bits. Reference time-stamp seconds.
    uint32_t refTm_f;        // 32 bits. Reference time-stamp fraction of a second.

    uint32_t origTm_s;       // 32 bits. Originate time-stamp seconds.
    uint32_t origTm_f;       // 32 bits. Originate time-stamp fraction of a second.

    uint32_t rxTm_s;         // 32 bits. Received time-stamp seconds.
    uint32_t rxTm_f;         // 32 bits. Received time-stamp fraction of a second.

    uint32_t txTm_s;         // 32 bits and the most important field the client cares about. Transmit time-stamp seconds.
    uint32_t txTm_f;         // 32 bits. Transmit time-stamp fraction of a second.

} ntp_packet;              // Total: 384 bits or 48 bytes.

static ntp_packet packet = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

static void ntp_error(char* msg)
{
    printf("\033[31;22m[E/NTP]: ERROR %s\033[0m\n", msg); // Print the error message to stderr.
}

/**
 * Get the UTC time from NTP server
 *
 * @param host_name NTP server host name, NULL: will using default host name
 *
 * @note this function is not reentrant
 *
 * @return >0: success, current UTC time
 *         =0: get failed
 */
time_t wayz_get_time_by_ntp(const char *host_name)
{
    int sockfd, n; // Socket file descriptor and the n return result from writing/reading from the socket.

    int portno = 123; // NTP UDP port number.

    time_t new_time = 0;

    fd_set readset;
    struct timeval timeout;

    // Using default host name when host_name is NULL
    if (host_name == NULL)
    {
        host_name = NTP_HOSTNAME;
    }

    // Create and zero out the packet. All 48 bytes worth.

    memset(&packet, 0, sizeof(ntp_packet));

    // Set the first byte's bits to 00,011,011 for li = 0, vn = 3, and mode = 3. The rest will be left set to zero.

    *((char *) &packet + 0) = 0x1b; // Represents 27 in base 10 or 00011011 in base 2.

    // Create a UDP socket, convert the host-name to an IP address, set the port number,
    // connect to the server, send the packet, and then read in the return packet.

    struct sockaddr_in serv_addr; // Server address data structure.
    struct hostent *server;      // Server data structure.

    sockfd = socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP); // Create a UDP socket.

    if (sockfd < 0) {
        ntp_error("opening socket");
        return 0;
    }

    server = gethostbyname(host_name); // Convert URL to IP.

    if (server == NULL) {
        ntp_error("no such host");
        goto __exit;
    }

    // Zero out the server address structure.

    memset((char *) &serv_addr, 0, sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;

    // Copy the server's IP address to the server address structure.

    memcpy((char *) &serv_addr.sin_addr.s_addr, (char *) server->h_addr, server->h_length);

    // Convert the port number integer to network big-endian style and save it to the server address structure.

    serv_addr.sin_port = htons(portno);

    // Call up the server using its IP address and port number.

    if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        ntp_error("connecting");
        goto __exit;
    }

    // Send it the NTP packet it wants. If n == -1, it failed.

    n = send(sockfd, (char*) &packet, sizeof(ntp_packet), 0);

    if (n < 0) {
        ntp_error("writing to socket");
        goto __exit;
    }

    timeout.tv_sec = NTP_GET_TIMEOUT;
    timeout.tv_usec = 0;

    FD_ZERO(&readset);
    FD_SET(sockfd, &readset);

    if (select(sockfd + 1, &readset, NULL, NULL, &timeout) <= 0) {
        ntp_error("select the socket timeout(5s)");
        goto __exit;
    }

    // Wait and receive the packet back from the server. If n == -1, it failed.

    n = recv(sockfd, (char*) &packet, sizeof(ntp_packet), 0);

    if (n < 0) {
        ntp_error("reading from socket");
        goto __exit;
    }

    // These two fields contain the time-stamp seconds as the packet left the NTP server.
    // The number of seconds correspond to the seconds passed since 1900.
    // ntohl() converts the bit/byte order from the network's to host's "endianness".

    packet.txTm_s = ntohl(packet.txTm_s); // Time-stamp seconds.
    packet.txTm_f = ntohl(packet.txTm_f); // Time-stamp fraction of a second.

    // Extract the 32 bits that represent the time-stamp seconds (since NTP epoch) from when the packet left the server.
    // Subtract 70 years worth of seconds from the seconds since 1900.
    // This leaves the seconds since the UNIX epoch of 1970.
    // (1900)------------------(1970)**************************************(Time Packet Left the Server)

    new_time = (time_t) (packet.txTm_s - NTP_TIMESTAMP_DELTA);

__exit:

    close(sockfd);

    return new_time;
}

#endif

#if 1  // other time request 

enum TIME_DEF
{
    SEC = 1,
    MIN = SEC * 60,
    HOUR = MIN * 60,
    DAY = HOUR * 24,
    YEAR = DAY * 365,
};
 
static time_t time_diff = 8 * HOUR;
static time_t mon_yday[2][12] =
{
    {0,31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334},
    {0,31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335},
};
 
static int leap(int year) {
    if ((year % 400 == 0) || (year % 4 == 0 && year % 100 != 0)) {
        return 1;
    }
    return 0;
}

static long long get_day(int year) {
    year = year - 1;
    int leap_year_num = year / 4 - year / 100 + year / 400;
    long long tol_day = year * 365 + leap_year_num;
    return tol_day;
}
 
static time_t mymktime(int year,int mon,int day,int hour,int min,int sec) {
    long long tol_day = 0;
    tol_day = get_day(year) - get_day(1970);
    tol_day += mon_yday[leap(year)][mon - 1];
    tol_day += day - 1;
 
    long long ret = 0;
    ret += tol_day * DAY;
    ret += hour * HOUR;
    ret += min * MIN;
    ret += sec * SEC;
 
    return ret - time_diff;
}

#define TIME_SERVER "quan.suning.com"
#define TIME_URL "http://quan.suning.com/getSysTime.do"

static const char *REQUEST = "GET " TIME_URL " HTTP/1.0\r\n"
    "Host: "TIME_SERVER"\r\n"
    "\r\n";

time_t http_get_time(char *time_data)
{
    const struct addrinfo hints = {
        .ai_family = AF_INET,
        .ai_socktype = SOCK_STREAM,
    };
    struct addrinfo *res;
    struct in_addr *addr;
    int s, r;
    char recv_buf[1024];

    int err = getaddrinfo(TIME_SERVER, "80", &hints, &res);

    if(err != 0 || res == NULL) {
        WAYZ_LOGE("DNS lookup failed err=%d res=%p", err, res);
        return WAYZ_FAIL;
    }

    /* Code to print the resolved IP.
        Note: inet_ntoa is non-reentrant, look at ipaddr_ntoa_r for "real" code */
    addr = &((struct sockaddr_in *)res->ai_addr)->sin_addr;
    WAYZ_LOGD("DNS lookup succeeded. IP=%s", inet_ntoa(*addr));

    s = socket(res->ai_family, res->ai_socktype, 0);
    if(s < 0) {
        WAYZ_LOGE("... Failed to allocate socket.");
        freeaddrinfo(res);
        return WAYZ_FAIL;
    }
    WAYZ_LOGD("... allocated socket\r\n");

    if(connect(s, res->ai_addr, res->ai_addrlen) != 0) {
        WAYZ_LOGE("... socket connect failed errno");
        close(s);
        freeaddrinfo(res);
        return WAYZ_FAIL;
    }

    WAYZ_LOGD("... connected\r\n");
    freeaddrinfo(res);

    if (write(s, REQUEST, strlen(REQUEST)) < 0) {
        WAYZ_LOGE("... socket send failed");
        close(s);
        return WAYZ_FAIL;
    }
    WAYZ_LOGD("... socket send success");

    struct timeval receiving_timeout;
    receiving_timeout.tv_sec = 3;
    receiving_timeout.tv_usec = 0;
    if (setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &receiving_timeout,
            sizeof(receiving_timeout)) < 0) {
        WAYZ_LOGE("... failed to set socket receiving timeout");
        close(s);
        return WAYZ_FAIL;
    }
    WAYZ_LOGD("... set socket receiving timeout success");

    bzero(recv_buf, sizeof(recv_buf));
    r = read(s, recv_buf, sizeof(recv_buf)-1);
    WAYZ_LOGD("... done reading from socket. Last read return=%d", r);
    close(s);
    char *p = strstr(recv_buf, "sysTime2");
    if (NULL == p)
    {
        WAYZ_LOGE("strstr systime2 failure");
        return WAYZ_FAIL;
    }
    char buf_time[20] = {0};
    bzero(buf_time, sizeof (buf_time));
    memcpy(buf_time, p + 11, 19);
    if (NULL != time_data)
    {
        memcpy(time_data, buf_time, sizeof (buf_time));
        time_data[19] = '\0';
    }
    int year = 0, mon = 0, day = 0, hour = 0, min = 0, sec = 0;
    sscanf(buf_time, "%d-%d-%d %d:%d:%d", &year, &mon, &day, &hour, &min, &sec);

    time_t time = mymktime(year, mon, day, hour, min, sec);

    return time;
}

#endif

// 掉电无法设置
char set_system_time(void)
{
    // char buf[40] = {0};
    time_t t = http_get_time(NULL);
    if (WAYZ_FAIL == t)
    {
        WAYZ_LOGE("set system time failure.");
        t = wayz_get_time_by_ntp(NULL);
        if (0 == t)
        {
            WAYZ_LOGE("wayz_get_time_by_ntp failure. ");
            return WAYZ_FAIL;
        }
    }

    // set system time 
    struct timeval now = { .tv_sec = t };
    settimeofday(&now, NULL);
    return WAYZ_OK;
}

