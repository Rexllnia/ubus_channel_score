#include <unistd.h>
#include <signal.h>
#include <semaphore.h>
#include <json-c/json.h>
#include <libubox/blobmsg_json.h>
#include "libubus.h"
#include "country_channel.h"
#include "usock_udp.h"
#include "channel_score_config.h"

#define SCAN_BUSY       1
#define SCAN_IDLE       2
#define SCAN_NOT_START  0


#define MAX_CHANNEL_NUM 200 

enum {
    CODE,
	BAND,
    CHANNEL_BITMAP,
    SCAN_TIME,
	__SCAN_MAX
};




void *ubus_thread(void *arg);
static int scan(struct ubus_context *ctx, struct ubus_object *obj,
		      struct ubus_request_data *req, const char *method,
		      struct blob_attr *msg);
static int get(struct ubus_context *ctx, struct ubus_object *obj,
		      struct ubus_request_data *req, const char *method,
		      struct blob_attr *msg);
static int realtime_get(struct ubus_context *ctx, struct ubus_object *obj,
		      struct ubus_request_data *req, const char *method,
		      struct blob_attr *msg);
static void add_channel_info_blobmsg(struct blob_buf *buf,struct channel_info *channel_info,int channel_num);


