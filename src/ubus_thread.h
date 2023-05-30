#include <unistd.h>
#include <signal.h>
#include <json-c/json.h>
#include <libubox/blobmsg_json.h>
#include "libubus.h"
#include "country_channel.h"

#define SCAN_BUSY       1
#define SCAN_IDLE       2
#define SCAN_NOT_START  0

#define FAIL -1
#define SUCCESS 0

#define MAX_CHANNEL_NUM 200 

enum {
    CODE,
	BAND,
    CHANNEL_BITMAP,
    SCAN_TIME,
	__SCAN_MAX
};

struct user_input {
    int code;
    int band;
    int channel_bitmap;
    int channel_num;
    int scan_time;
};

struct channel_info {
    int channel;
    int floornoise;
    int utilization;
    int bw;
    int obss_util;
    int tx_util;
    int rx_util;
};


void *ubus_thread(void *arg);
static int scan(struct ubus_context *ctx, struct ubus_object *obj,
		      struct ubus_request_data *req, const char *method,
		      struct blob_attr *msg);
static int get(struct ubus_context *ctx, struct ubus_object *obj,
		      struct ubus_request_data *req, const char *method,
		      struct blob_attr *msg);

