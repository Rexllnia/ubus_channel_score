#include "device_list.h"
#include "channel_score_config.h"
#include <stdio.h>

extern struct user_input g_input;
int modify_device(struct device_list *device_list,struct device_info *device)
{
    int pos;

    if (device_list == NULL || device == NULL) {
        return FAIL;
    } 

    pos = find_device_by_sn(device_list,device->series_no);

    
    memcpy(&device_list[pos],device,sizeof(struct device_info));
     
}
int find_ap(struct device_list *device_list)
{
	int i;
	for (i = 0;i < device_list->list_len;i++) {
		if (strcmp(device_list->device[i].role,"ap") == 0) {
			return i;
		}
	}
}
void clear_device_finish_flag(struct device_list *list) {
	struct device_info *p;
	int i;
	list_for_each_device(p, i, list) {
		p->finished_flag = NOT_FINISH;
	}
}

int show_device_info(struct device_info *device_info) {
	int i;
	debug("show info");
	printf(" series_no %s\r\n",device_info->series_no);
	for (i = 0; i < g_input.channel_num;i++) {
		printf("\r\n");
		printf(" channel %d\r\n",device_info->channel_info[i].channel);
		printf(" floornoise %d\r\n",device_info->channel_info[i].floornoise);
		printf(" obss_util %d\r\n",device_info->channel_info[i].obss_util);
		printf(" score %f\r\n",device_info->channel_info[i].score);
		printf("\r\n");
	}
	
}

int find_device_by_sn(struct device_list *device_list,char *series_no)
{
	int i;

	for (i = 0;i < device_list->list_len;i++) {
		if (strcmp(device_list->device[i].series_no,series_no) == 0) {
			return i;
		}
	}
	return FAIL;
}

void get_online_device(struct device_list *device_list)
{
	char *rbuf;

	int i;

	json_object *rbuf_root;
	json_object *list_all_obj;
	json_object *list_pair_obj;
	json_object *sn_obj,*role_obj,*mac_obj;

	
	execute_cmd("dev_sta get -m wds_list_all",&rbuf);

	rbuf_root = json_tokener_parse(rbuf);
	list_all_obj = json_object_object_get(rbuf_root,"list_all");
	json_object *list_all_elem = json_object_array_get_idx(list_all_obj,0);
	list_pair_obj = json_object_object_get(list_all_elem,"list_pair");

	device_list->list_len = json_object_array_length(list_pair_obj);

	for (i = 0;i < json_object_array_length(list_pair_obj);i++) {
		json_object *list_pair_elem;
		list_pair_elem = json_object_array_get_idx(list_pair_obj,i);
		sn_obj = json_object_object_get(list_pair_elem,"sn");
		role_obj = json_object_object_get(list_pair_elem,"role");
		mac_obj = json_object_object_get(list_pair_elem,"mac");
		strcpy(device_list->device[i].series_no,json_object_get_string(sn_obj));
		strcpy(device_list->device[i].role,json_object_get_string(role_obj));
		strcpy(device_list->device[i].mac,json_object_get_string(mac_obj));
	}

	free(rbuf);
	json_object_put(rbuf_root);
}

