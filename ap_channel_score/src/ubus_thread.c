/*
 * Copyright (C) 2011-2014 Felix Fietkau <nbd@openwrt.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 2.1
 * as published by the Free Software Foundation
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */


#include "ubus_thread.h"
#include "channel_score_config.h"
#include "composite_score.h"
#include "device_list.h"
#include <stdbool.h>
#include <stdio.h>
#include <math.h> 

#define MAX(a,b) ((a)>(b)?(a):(b))

struct channel_info g_channel_info_5g[MAX_BAND_5G_CHANNEL_NUM];
extern struct channel_info realtime_channel_info_5g[36];
extern time_t g_current_time;
extern sem_t g_semaphore;
extern int g_status;
extern pthread_mutex_t g_mutex;
static struct ubus_context *ctx;
static struct ubus_subscriber test_event;
static struct blob_buf b;
struct user_input g_input;

struct device_list g_finished_device_list;
struct device_list g_device_list;


struct scan_request {
	struct ubus_request_data req;
	struct uloop_timeout timeout;
	int fd;
	int idx;
	char data[];
};
struct get_request {
	struct ubus_request_data req;
	struct uloop_timeout timeout;
	int fd;
	int idx;
	char data[];
};
struct realtime_get_request {
	struct ubus_request_data req;
	struct uloop_timeout timeout;
	int fd;
	int idx;
	char data[];
};
static const struct blobmsg_policy scan_policy[] = {
	[BAND] = { .name = "band", .type = BLOBMSG_TYPE_INT32 },
	[CHANNEL_BITMAP] = { .name = "channel_bitmap", .type = BLOBMSG_TYPE_ARRAY },
	[SCAN_TIME] = { .name = "scan_time", .type = BLOBMSG_TYPE_INT32 },
};
static const struct ubus_method channel_score_methods[] = {
	UBUS_METHOD_NOARG("get", get),
	UBUS_METHOD_NOARG("realtime_get",realtime_get),
	UBUS_METHOD("scan", scan, scan_policy),
};

static struct ubus_object_type channel_score_object_type =
	UBUS_OBJECT_TYPE("channel_score", channel_score_methods);

static struct ubus_object channel_score_object = {
	.name = "channel_score",
	.type = &channel_score_object_type,
	.methods = channel_score_methods,
	.n_methods = ARRAY_SIZE(channel_score_methods),
};


extern int  get_channel_info(struct channel_info *info,int band);
extern double calculate_channel_score(struct channel_info *info);
extern double calculate_N(struct channel_info *info);

int send_remote_scan_msg(struct device_list *list,int wait_sec) 
{
	struct device_info *p;
	int instant = 0;
	int i;
	
	memset(list,0,sizeof(struct device_list));
	get_online_device(list);

	list_for_each_device(p,i,list) {
		if (strcmp(p->role,"ap") != 0) {
			instant = rg_mist_mac_2_nodeadd(p->mac);
			printf("line : %d fun : %s instant : %x \r\n",__LINE__,__func__,instant);
			if (tipc_msg_send(SERVER_TYPE_SCAN,instant,&g_input,sizeof(struct user_input),wait_sec) != FAIL) {
				printf("%x\r\n",g_input.channel_bitmap);
			}
		}	
	}
}
int get_remote_channel_list(struct device_list *dst_list,int wait_sec)
{
	struct device_info *p;
	int i,instant;

		list_for_each_device(p,i,dst_list) {
		if (strcmp(p->role,"ap") != 0) {	
			instant = rg_mist_mac_2_nodeadd(p->mac);
			printf("line : %d fun : %s instant : %x \r\n",__LINE__,__func__,instant);
			if (tipc_msg_send_receive(SERVER_TYPE_GET,instant,p->channel_info,36 * sizeof(struct channel_info),wait_sec) == FAIL) {
				return FAIL;
			}
			printf("test : %d\r\n",p->channel_info[0].floornoise);
		}	
	}
	return SUCCESS;
}

static void scan_reply(struct uloop_timeout *t)
{
	struct scan_request *req = container_of(t, struct scan_request, timeout);
	int fds[2];
	char temp[100];

	struct channel_info current_channel_info;

	blob_buf_init(&b, 0);

	get_channel_info(&current_channel_info, PLATFORM_5G);
	current_channel_info.score = calculate_channel_score(&current_channel_info);

	sprintf(temp,"%d",current_channel_info.channel);
	blobmsg_add_string(&b, "current_channel", temp);

	sprintf(temp,"%d",current_channel_info.floornoise);
	blobmsg_add_string(&b, "floornoise", temp);

	sprintf(temp,"%d",current_channel_info.utilization);
	blobmsg_add_string(&b, "utilization", temp);

	sprintf(temp,"%f",current_channel_info.score);
	blobmsg_add_string(&b, "score", temp);

	blobmsg_add_string(&b, "status_code", req->data);


	ubus_send_reply(ctx, &req->req, b.head);

	if (pipe(fds) == -1) {
		fprintf(stderr, "Failed to create pipe\n");
		return;
	}
    
	ubus_request_set_fd(ctx, &req->req, fds[0]);
	ubus_complete_deferred_request(ctx, &req->req, 0);
	req->fd = fds[1];
	free(req);
}


static void realtime_get_reply(struct uloop_timeout *t)
{
	struct realtime_get_request *req = container_of(t, struct realtime_get_request, timeout);
	int fds[2];
	static struct blob_buf buf;
	
	char temp[512];
	int i;

	blob_buf_init(&buf, 0);
	printf("%s\r\n",__func__);

	memset(&g_device_list,0,sizeof(g_device_list)); /* xxx */

	get_online_device(&g_device_list);
	/* find AP */
	i = find_ap(&g_device_list);
	printf("%s i = %d\r\n",__func__,i);
	
	memcpy(g_device_list.device[i].channel_info,realtime_channel_info_5g,sizeof(realtime_channel_info_5g));
	g_device_list.device[i].timestamp = time(NULL);
	g_device_list.device[i].input = g_input;
	g_device_list.device[i].status = g_status;

	get_remote_channel_list(&g_device_list,4);


	/* scan list*/
	void * const scan_list_obj = blobmsg_open_array(&buf, "scan_list");
	
	for (i = 0;i < g_device_list.list_len;i++) {
		add_device_info_blobmsg(&buf,&g_device_list.device[i],true);
	}

	blobmsg_close_array(&buf, scan_list_obj);

	ubus_send_reply(ctx, &req->req, buf.head);

	if (pipe(fds) == -1) {
		fprintf(stderr, "Failed to create pipe\n");
		return;
	}
    
	ubus_request_set_fd(ctx, &req->req, fds[0]);
	ubus_complete_deferred_request(ctx, &req->req, 0);
	req->fd = fds[1];

	
	free(req);
}
static void add_channel_score_blobmsg(struct blob_buf *buf, struct channel_info *channel_info)
{
	char *temp[64];
	void *const channel_score_table = blobmsg_open_table(buf, NULL);
	sprintf(temp,"%d",channel_info->channel);
	blobmsg_add_string(buf,"channel",temp);
	sprintf(temp,"%f",channel_info->score);
	blobmsg_add_string(buf,"score",temp);
	blobmsg_close_table(buf, channel_score_table);
}
static void add_score_list_blobmsg(struct blob_buf *buf,int channel_num,struct channel_info *channel_info_list)
{
	int i;
	void * const score_list = blobmsg_open_array(buf, "score_list");

	for (i = 0;i < channel_num;i++) {
		add_channel_score_blobmsg(buf,&channel_info_list[i]);
	}

	blobmsg_close_array(buf, score_list);

}
static void add_bw80_blobmsg(struct blob_buf *buf,struct device_info *device) 
{
	int j;
	void *const bw80_table = blobmsg_open_table(buf, "bw_80");
	printf("line : %d fun : %s \r\n",__LINE__,__func__);
	for ( j = 0;j < g_input.channel_num / 4;j++) {
		device->bw80_channel[j] = device->channel_info[4*j];
		printf("line : %d fun : %s \r\n",__LINE__,__func__);
		device->bw80_channel[j].floornoise =MAX(MAX(MAX(device->channel_info[4*j].floornoise,
												device->channel_info[4*j+1].floornoise), 
												device->channel_info[4*j+2].floornoise),
												device->channel_info[4*j+3].floornoise);
		printf("line : %d fun : %s \r\n",__LINE__,__func__);
		device->bw80_channel[j].score = ((double)1 - calculate_N(&(device->bw80_channel[j])) / 20) * 
								(double)((double)1 - (double)(device->channel_info[4*j].obss_util
								+ device->channel_info[4*j+1].obss_util
								+ device->channel_info[4*j+2].obss_util
								+ device->channel_info[4*j+3].obss_util)
								/ (95 * BW_80 / 20)) * 1200 * 0.75;
	}
	printf("line : %d fun : %s \r\n",__LINE__,__func__);
	add_score_list_blobmsg(buf,g_input.channel_num / 4,device->bw80_channel);
	printf("line : %d fun : %s \r\n",__LINE__,__func__);
	blobmsg_close_table(buf, bw80_table);
}
static void add_bw40_blobmsg(struct blob_buf *buf,struct device_info *device) 
{
	void *const bw40_table = blobmsg_open_table(buf, "bw_40");
	
	int j;

	for ( j = 0;j < g_input.channel_num / 2;j++) {
		printf("line : %d fun : %s \r\n",__LINE__,__func__);
		device->bw40_channel[j] = device->channel_info[2*j];
		printf("line : %d fun : %s \r\n",__LINE__,__func__);
		device->bw40_channel[j].floornoise = MAX(device->channel_info[2*j].floornoise,device->channel_info[2*j+1].floornoise);
		
		device->bw40_channel[j].score = ((double)1-calculate_N(&(device->bw40_channel[j]))/20) * 
										(double)((double)1 -(double)(device->channel_info[2*j].obss_util + 
										device->channel_info[2*j+1].obss_util ) / (95 * BW_40/20)) * 600 * 0.75;
	}
	printf("line : %d fun : %s \r\n",__LINE__,__func__);
	add_score_list_blobmsg(buf,g_input.channel_num / 2,device->bw40_channel);
	printf("line : %d fun : %s \r\n",__LINE__,__func__);
	blobmsg_close_table(buf, bw40_table);
}

static void add_device_info_blobmsg(struct blob_buf *buf,struct device_info *device,int is_real_time)
{
	void * const device_obj = blobmsg_open_table(buf, NULL);

	printf("%s  %s \r\n",__func__,device->series_no);
	printf("%s  %s \r\n",__func__,device->role);
	blobmsg_add_string(buf, "SN",device->series_no);
	blobmsg_add_string(buf, "role",device->role);
	
	add_timestamp_blobmsg(buf,&(device->timestamp));
	
	
	/* 5G */
	void * const BAND_5G_obj = blobmsg_open_table(buf, "5G");
	// blobmsg_add_string(buf, "band","5");

	if (g_status == SCAN_BUSY) { 
		blobmsg_add_string(buf, "status","busy");
		blobmsg_add_string(buf, "status_code","1");
	} else if (g_status == SCAN_IDLE) {
		blobmsg_add_string(buf, "status","idle");
		blobmsg_add_string(buf, "status_code","2");
	} else if (g_status == SCAN_NOT_START) {
		blobmsg_add_string(buf, "status","not start");
		blobmsg_add_string(buf, "status_code","0");
	}

	void *const bw20_table = blobmsg_open_table(buf, "bw_20");
	// blobmsg_add_string(buf,"bw","20");
	add_score_list_blobmsg(buf,g_input.channel_num,device->channel_info);
	// add_channel_info_blobmsg(buf,realtime_channel_info_5g,g_input.channel_num);
	blobmsg_close_table(buf, bw20_table);

	if (is_real_time == false) {
		printf("line : %d fun : %s \r\n",__LINE__,__func__);
		add_bw40_blobmsg(buf,device);
		add_bw80_blobmsg(buf,device);
		printf("line : %d fun : %s \r\n",__LINE__,__func__);
	}


	
	blobmsg_close_table(buf, BAND_5G_obj);
	blobmsg_close_table(buf, device_obj);
}
static void add_timestamp_blobmsg(struct blob_buf *buf,time_t *timestamp)
{
	char temp[256];
	struct tm *local_time;  // 将时间戳转换为本地时间
	local_time = localtime(timestamp);
	sprintf(temp,"当前时间：%d年%d月%d日 %d:%d:%d", 
	local_time->tm_year + 1900, local_time->tm_mon + 1, local_time->tm_mday, 
	local_time->tm_hour, local_time->tm_min, local_time->tm_sec);
	blobmsg_add_string(buf, "current time",temp);
	sprintf(temp,"%ld",*timestamp);
	blobmsg_add_string(buf, "timestamp",temp);
}
static void add_channel_info_blobmsg(struct blob_buf *buf,struct channel_info *channel_info,int channel_num)
{
	char temp[512];
	int i = 0;

	if (channel_info == NULL || buf == NULL) {
		return;
	}

	for (i = 0; i < channel_num;i++) {
		void * const obj = blobmsg_open_table(buf, NULL);

		sprintf(temp,"%d",channel_info[i].channel);
		blobmsg_add_string(buf, "channel",temp);
		
		sprintf(temp,"%d",channel_info[i].floornoise);
		blobmsg_add_string(buf, "floornoise", temp);

		sprintf(temp,"%d",channel_info[i].utilization);
		blobmsg_add_string(buf, "utilization", temp);

		sprintf(temp,"%d",channel_info[i].bw);
		blobmsg_add_string(buf, "bw", temp);
		
		sprintf(temp,"%d",channel_info[i].obss_util);
		blobmsg_add_string(buf, "obss_util", temp);
		
		sprintf(temp,"%d",channel_info[i].tx_util);
		blobmsg_add_string(buf, "tx_util", temp);

		sprintf(temp,"%d",channel_info[i].rx_util);
		blobmsg_add_string(buf, "rx_util", temp);

		sprintf(temp,"%f",channel_info[i].score);
		blobmsg_add_string(buf, "score", temp);

		blobmsg_close_table(buf, obj);
	}
}
void add_bw40_best_channel_blobmsg(struct blob_buf *buf,struct device_list *list) 
{
	void *const bw40_table = blobmsg_open_table(buf, "bw_40");
	int best_channel_ptr;
	struct channel_info bw40_channel[18];

	int j,i;
	double channel_avg_score[36];
	char temp[100];
	struct device_info *p;

	memset(channel_avg_score,0,36);

	for (j = 0;j < g_input.channel_num / 2; j++) {
		list_for_each_device(p, i, list) {
			channel_avg_score[j] += p->bw40_channel[j].score;
		}
		channel_avg_score[j] /= (list->list_len);
	}

	

	best_channel_ptr = 0;
	for (i = 0 ;i < g_input.channel_num / 2;i++) {
		if (channel_avg_score[best_channel_ptr] < channel_avg_score[i]) {
			best_channel_ptr = i;
		}
	}

	sprintf(temp,"%f",channel_avg_score[best_channel_ptr]);
	blobmsg_add_string(buf,"score",temp);

	sprintf(temp,"%d",g_channel_info_5g[best_channel_ptr].channel);
	blobmsg_add_string(buf,"channel",temp);

	blobmsg_close_table(buf, bw40_table);
}
void add_bw80_best_channel_blobmsg(struct blob_buf *buf,struct device_list *list)
{
	void *const bw80_table = blobmsg_open_table(buf, "bw_80");
	int best_channel_ptr;
	struct channel_info bw80_channel[9];

	int j,i;
	double channel_avg_score[36];
	char temp[100];
	struct device_info *p;

	memset(channel_avg_score,0,36);

	for (j = 0;j < g_input.channel_num / 4; j++) {
		list_for_each_device(p, i, list) {
			channel_avg_score[j] += p->bw80_channel[j].score;
		}
		channel_avg_score[j] /= (list->list_len);
	}

	

	best_channel_ptr = 0;
	for (i = 0 ;i < g_input.channel_num / 4;i++) {
		if (channel_avg_score[best_channel_ptr] < channel_avg_score[i]) {
			best_channel_ptr = i;
		}
	}

	sprintf(temp,"%f",channel_avg_score[best_channel_ptr]);
	blobmsg_add_string(buf,"score",temp);

	sprintf(temp,"%d",g_channel_info_5g[best_channel_ptr].channel);
	blobmsg_add_string(buf,"channel",temp);

	blobmsg_close_table(buf, bw80_table);
}
static void get_reply(struct uloop_timeout *t)
{
	struct get_request *req = container_of(t, struct get_request, timeout);
	int fds[2];
	static struct blob_buf buf;
	struct device_info *p;
	char temp[512];
	int i;
	int code;

	blob_buf_init(&buf, 0);
	printf("%s\r\n",__func__);

	
	/* find AP */
	i = find_ap(&g_finished_device_list);
	printf("%s i = %d\r\n",__func__,i);
	
	memcpy(g_finished_device_list.device[i].channel_info,g_channel_info_5g,sizeof(g_channel_info_5g));
	g_finished_device_list.device[i].timestamp = g_current_time;
	g_finished_device_list.device[i].input = g_input;
	g_finished_device_list.device[i].status = g_status;


	if (g_status == SCAN_TIMEOUT) {
		goto scan_timeout;
	}
	if (g_status == SCAN_BUSY) {
		goto scan_busy;
	}
	if (g_status == SCAN_NOT_START) {
		goto scan_not_start;
	}


	int instant = 0;
	
	list_for_each_device(p,i,&g_finished_device_list) {
		if (strcmp(p->role,"ap") != 0) {	
			instant = rg_mist_mac_2_nodeadd(p->mac);
			printf("line : %d fun : %s instant : %x \r\n",__LINE__,__func__,instant);
			tipc_msg_send_receive(SERVER_TYPE_GET,instant,p->channel_info,36 * sizeof(struct channel_info),4);
			printf("test : %d\r\n",p->channel_info[0].floornoise);
		}	
	}

	/* scan list*/
	void * const scan_list_obj = blobmsg_open_array(&buf, "scan_list");
	
	for (i = 0;i < g_finished_device_list.list_len;i++) {
		add_device_info_blobmsg(&buf,&g_finished_device_list.device[i],false);
	}

	blobmsg_close_array(&buf, scan_list_obj);

	/* best channel*/
	void * const best_channel_obj = blobmsg_open_table(&buf, "best_channel");

	add_bw40_best_channel_blobmsg(&buf,&g_finished_device_list);
	add_bw80_best_channel_blobmsg(&buf,&g_finished_device_list);

	blobmsg_close_table(&buf, best_channel_obj);

	ubus_send_reply(ctx, &req->req, buf.head);
	if (pipe(fds) == -1) {
		fprintf(stderr, "Failed to create pipe\n");
		return;
	}
    
	ubus_request_set_fd(ctx, &req->req, fds[0]);
	ubus_complete_deferred_request(ctx, &req->req, 0);
	req->fd = fds[1];
	
	free(req);
	return ;

scan_not_start:

	blobmsg_add_string(&buf,"status","not start");
	blobmsg_add_string(&buf,"status_code","0");
	ubus_send_reply(ctx, &req->req, buf.head);
	if (pipe(fds) == -1) {
		fprintf(stderr, "Failed to create pipe\n");
		return;
	}
    
	ubus_request_set_fd(ctx, &req->req, fds[0]);
	ubus_complete_deferred_request(ctx, &req->req, 0);
	req->fd = fds[1];
	
	free(req);
	return;

scan_timeout:
	blobmsg_add_string(&buf,"status","timeout");
	blobmsg_add_string(&buf,"status_code","3");
	ubus_send_reply(ctx, &req->req, buf.head);
	if (pipe(fds) == -1) {
		fprintf(stderr, "Failed to create pipe\n");
		return;
	}
    
	ubus_request_set_fd(ctx, &req->req, fds[0]);
	ubus_complete_deferred_request(ctx, &req->req, 0);
	req->fd = fds[1];
	
	free(req);
	return;

scan_busy:
	blobmsg_add_string(&buf,"status","busy");
	blobmsg_add_string(&buf,"status_code","1");
	ubus_send_reply(ctx, &req->req, buf.head);
	if (pipe(fds) == -1) {
		fprintf(stderr, "Failed to create pipe\n");
		return;
	}
    
	ubus_request_set_fd(ctx, &req->req, fds[0]);
	ubus_complete_deferred_request(ctx, &req->req, 0);
	req->fd = fds[1];
	
	free(req);
}

int channel_check(int channel) {

	if(channel < 36 || channel > 181)
	{
		return FAIL;				
	}

	if (channel >= MAX_BAND_5G_CHANNEL_NUM && channel <= 144) {
		if(channel % 4 != 0) {
			return FAIL;
		}

	}

	if (channel >= 149 && channel <= 181) {
		if ((channel - 1) % 4 != 0) {

			return FAIL;
		}
	}  

	return SUCCESS;
}

static int scan(struct ubus_context *ctx, struct ubus_object *obj,
		      struct ubus_request_data *req, const char *method,
		      struct blob_attr *msg)
{
	struct channel_info channel_info;
	struct scan_request *hreq;
	size_t len;
	struct blob_attr *tb[__SCAN_MAX];
	char format[100];
	struct remote_message scan_message;
	struct blob_attr *channel_bitmap_array[MAX_CHANNEL_NUM];
	static struct blobmsg_policy channel_bitmap_policy[MAX_CHANNEL_NUM];
	int i,total_band_num;

	char msgstr[100] = "(unknown)";
	long bitmap_2G,bitmap_5G;

	for (i = 0;i < MAX_CHANNEL_NUM;i++) {
		channel_bitmap_policy[i].type = BLOBMSG_TYPE_INT32;
	}
	
	bitmap_5G = 0;
	bitmap_2G = 0;

	blobmsg_parse(scan_policy, ARRAY_SIZE(scan_policy), tb, blob_data(msg), blob_len(msg));

	if (g_status == SCAN_BUSY) {

		len = sizeof(*hreq) + sizeof(msgstr) + 1;
		hreq = calloc(1, len);
		if (!hreq) {
			return UBUS_STATUS_UNKNOWN_ERROR;
		}
		sprintf(hreq->data,"%d",FAIL);
		goto error;
	}

	if (tb[BAND] ) {
		printf("g_status %d \r\n",g_status);
		if (blobmsg_get_u32(tb[BAND]) == 5 || blobmsg_get_u32(tb[BAND]) == 2) {

		} else {
			len = sizeof(*hreq) + sizeof(msgstr) + 1;
			hreq = calloc(1, len);
			if (!hreq) {
				return UBUS_STATUS_UNKNOWN_ERROR;
			}
			sprintf(hreq->data,"%d",FAIL);
			goto error;
		}

		get_channel_info(&channel_info, blobmsg_get_u32(tb[BAND]));

		total_band_num = get_country_channel_bitmap(channel_info.bw,&bitmap_2G,&bitmap_5G,PLATFORM_5G);
		g_input.band = blobmsg_get_u32(tb[BAND]);

		if (tb[SCAN_TIME]) {
			g_input.scan_time = blobmsg_get_u32(tb[SCAN_TIME]);
		}

		if (tb[CHANNEL_BITMAP] && blobmsg_check_array(tb[CHANNEL_BITMAP],BLOBMSG_TYPE_INT32)) {
			g_input.channel_num = blobmsg_check_array(tb[CHANNEL_BITMAP],BLOBMSG_TYPE_INT32);
			g_input.channel_bitmap = 0;
			blobmsg_parse_array(channel_bitmap_policy, ARRAY_SIZE(channel_bitmap_policy), channel_bitmap_array, blobmsg_data(tb[CHANNEL_BITMAP]), blobmsg_len(tb[CHANNEL_BITMAP]));
			printf("len %d\n",g_input.channel_num);
			for (i = 0;i < g_input.channel_num;i++) {
				printf("%d\r\n",blobmsg_get_u32(channel_bitmap_array[i]));
				if (channel_check(blobmsg_get_u32(channel_bitmap_array[i])) == FAIL) {
					len = sizeof(*hreq) + sizeof(msgstr) + 1;
					hreq = calloc(1, len);
					if (!hreq) {
						return UBUS_STATUS_UNKNOWN_ERROR;
					}
					printf("line : %d fun : %s \r\n",__LINE__,__func__);
					sprintf(hreq->data,"%d",FAIL);
					goto error;
				}
				printf("line : %d fun : %s \r\n",__LINE__,__func__);
				g_input.channel_bitmap |= 1<< (blobmsg_get_u32(channel_bitmap_array[i])/4 - 9);
			}
			printf("input bitmap %d\r\n",g_input.channel_bitmap);
			printf("input  %d\r\n",g_input.channel_bitmap & bitmap_5G);
			if ((g_input.channel_bitmap & bitmap_5G ) != g_input.channel_bitmap) {
				len = sizeof(*hreq) + sizeof(msgstr) + 1;
				hreq = calloc(1, len);
				if (!hreq) {
					return UBUS_STATUS_UNKNOWN_ERROR;
				}
				sprintf(hreq->data,"%d",FAIL);
				goto error;
			}
		} else {
			g_input.channel_bitmap = bitmap_5G;
			g_input.channel_num = total_band_num;
		}
		printf("band : %d",g_input.band);
		len = sizeof(*hreq) + sizeof(format) + 1;
		hreq = calloc(1, len);
		if (!hreq) {
			return UBUS_STATUS_UNKNOWN_ERROR;
		}
		sprintf(hreq->data,"%d",SUCCESS);	


		pthread_mutex_lock(&g_mutex);

		g_status = SCAN_BUSY;

		send_remote_scan_msg(&g_finished_device_list,1000);

		sem_post(&g_semaphore);
		pthread_mutex_unlock(&g_mutex);
		
	} else {
		len = sizeof(*hreq) + sizeof(msgstr) + 1;
		hreq = calloc(1, len);
		if (!hreq) {
			return UBUS_STATUS_UNKNOWN_ERROR;
		}
		sprintf(hreq->data,"{\"code\":%d}",FAIL);
		goto error;
	}

	ubus_defer_request(ctx, req, &hreq->req);
	hreq->timeout.cb = scan_reply;
	uloop_timeout_set(&hreq->timeout, 1000);
	return 0;

error:
	ubus_defer_request(ctx, req, &hreq->req);
	hreq->timeout.cb = scan_reply;
	uloop_timeout_set(&hreq->timeout, 1000);

	return 0;
}

static int realtime_get(struct ubus_context *ctx, struct ubus_object *obj,
		      struct ubus_request_data *req, const char *method,
		      struct blob_attr *msg)
{
	struct realtime_get_request *hreq;
	char format[100];
	const char *msgstr = "(error)";


	size_t len = sizeof(*hreq) + sizeof(format) + strlen(obj->name) + strlen(msgstr) + 1;
	hreq = calloc(1, len);
	if (!hreq) {
		return UBUS_STATUS_UNKNOWN_ERROR;
	}
	
	ubus_defer_request(ctx, req, &hreq->req);
	hreq->timeout.cb = realtime_get_reply;
	uloop_timeout_set(&hreq->timeout, 1000);

	return 0;
}

static int get(struct ubus_context *ctx, struct ubus_object *obj,
		      struct ubus_request_data *req, const char *method,
		      struct blob_attr *msg)
{
	struct get_request *hreq;
	char format[100];
	const char *msgstr = "(error)";

	size_t len = sizeof(*hreq) + sizeof(format) + strlen(obj->name) + strlen(msgstr) + 1;
	hreq = calloc(1, len);
	if (!hreq) {
		return UBUS_STATUS_UNKNOWN_ERROR;
	}
	
	ubus_defer_request(ctx, req, &hreq->req);
	hreq->timeout.cb = get_reply;
	uloop_timeout_set(&hreq->timeout, 1000);

	return 0;
}



static void server_main(void)
{
	int ret;

	ret = ubus_add_object(ctx, &channel_score_object);
	if (ret)
		fprintf(stderr, "Failed to add object: %s\n", ubus_strerror(ret));

	ret = ubus_register_subscriber(ctx, &test_event);
	if (ret)
		fprintf(stderr, "Failed to add watch handler: %s\n", ubus_strerror(ret));

	uloop_run();
}
void *ubus_thread(void *arg) 
{
	const char *ubus_socket = NULL;



	uloop_init();
	signal(SIGPIPE, SIG_IGN);

	ctx = ubus_connect(ubus_socket);
	if (!ctx) {
		fprintf(stderr, "Failed to connect to ubus\n");
		return -1;
	}

	ubus_add_uloop(ctx);

	server_main();

	ubus_free(ctx);
	uloop_done();
}
