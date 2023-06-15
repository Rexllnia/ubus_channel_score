/* ubus_thread.h */
#ifndef _UBUS_THREAD_H_
#define _UBUS_THREAD_H_

#include <unistd.h>
#include <signal.h>
#include <semaphore.h>
#include <json-c/json.h>
#include <libubox/blobmsg_json.h>
#include "libubus.h"
#include "country_channel.h"
#include "usock_udp.h"
#include "channel_score_config.h"
#include "device_list.h"
#include "tipc_func.h"

#define SCAN_BUSY       1
#define SCAN_IDLE       2
#define SCAN_NOT_START  0
#define SCAN_TIMEOUT  	3

#define MAX_CHANNEL_NUM 200 





enum {
	BAND,
    CHANNEL_BITMAP,
    SCAN_TIME,
	__SCAN_MAX
};


int get_remote_channel_list(struct device_list *dst_list,int wait_sec);

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
static void add_timestamp_blobmsg(struct blob_buf *buf,time_t *timestamp);
static void add_device_info_blobmsg(struct blob_buf *buf,struct device_info *device,int is_real_time);
static void add_score_list_blobmsg(struct blob_buf *buf,int channel_num,struct channel_info *channel_info_list);
static void add_channel_score_blobmsg(struct blob_buf *buf, struct channel_info *channel_info);
#endif
