#include "country_channel.h"

struct hlist_head g_country_channel_list[MAX_CHANNEL_NUM];

int hash_function(int channel)
{
    return channel%MAX_CHANNEL_NUM;
}
int lookup_channel(int channel)
{
    int bucket;
    struct country_channel_info *pos;
    struct hlist_node *p;
    struct hlist_node *temp;


    bucket = hash_function(channel);
    hlist_for_each_entry_safe(pos,p, temp,&g_country_channel_list[bucket], node) {
        if( pos->channel == channel) {
            printf("channel : %d\r\n", pos->channel);
        }  
    }

    return FAIL;
}
static int add_node(struct hlist_head *hashtable ,char *frequency, int channel, int NopOccupancy)
{
 
    int bucket;
    struct country_channel_info *ptr; 
    struct country_channel_info *pos;
    struct hlist_node *p;
    struct hlist_node *temp;

    ptr = malloc(sizeof(struct country_channel_info));
    if (ptr == NULL) {
        return FAIL;
    }
    strcpy(ptr->frequency,frequency);
    ptr->channel = channel;
    ptr->NopOccupancy = NopOccupancy;

    bucket = hash_function(channel);

    hlist_for_each_entry_safe(pos,p, temp,&g_country_channel_list[bucket], node) {
        if( pos->channel == channel) {
            return FAIL;
        }  
    }

    hlist_add_head(&(ptr->node), &hashtable[bucket]);

    return SUCCESS;
} 
void print_bits(long num) {
    int i;

    for (i = 0; i < sizeof(long) * 8; i++) {
        if ((num & (1L << i)) != 0) {
            printf("Bit %d is set\n", i);
        }
    }
}

#ifdef AP
int get_country_channel_bitmap(int bw,long *bitmap_2G,long *bitmap_5G,int band)
{

#ifdef UNIFY_FRAMEWORK_ENABLE
    uf_cmd_msg_t *msg_obj;
#elif defined POPEN_CMD_ENABLE
    char cmd[MAX_POPEN_BUFFER_SIZE];
#endif     
    int ret;
    char*rbuf,*param_input;    
    char bw_str[20];

    
    struct json_object *list = json_object_new_array();
    json_object *input_param_root,*output_param_root;
    json_object *qry_type_obj,*band_obj;
	int i,p;

	json_object *channel_2G_obj,*channel_5G_obj;
	json_object *frequency_obj,*channel_obj,*NopOccupancy_obj;
	char channel[8],frequency[8],NopOccupancy[8];

    rbuf = NULL;
    *bitmap_2G = 0;
    *bitmap_5G = 0;

	input_param_root = json_object_new_object();

    if (bw == 20) {
		json_object_object_add(input_param_root, "band", json_object_new_string("BW_20"));
    } else if (bw == 40) {
		json_object_object_add(input_param_root, "band", json_object_new_string("BW_40"));
    } else if (bw == 80) {
		json_object_object_add(input_param_root, "band", json_object_new_string("BW_80"));
    } else if (bw == 160) {
		json_object_object_add(input_param_root, "band", json_object_new_string("BW_160"));
    } else {
        return FAIL;
    }
    
    
    json_object_object_add(input_param_root, "qry_type", json_object_new_string("channellist"));
    
    param_input = json_object_to_json_string(input_param_root);
	printf("%s\n",param_input);

#ifdef UNIFY_FRAMEWORK_ENABLE
    msg_obj = (uf_cmd_msg_t*)malloc(sizeof(uf_cmd_msg_t));
    if (msg_obj == NULL) {
        return FAIL;
    }
    memset(msg_obj, 0, sizeof(uf_cmd_msg_t));

    msg_obj->param = param_input;
    msg_obj->ctype = UF_DEV_STA_CALL;    /* 调用类型 ac/dev/.. */
    msg_obj->cmd = "get";
    msg_obj->module = "country_channel";               /* 必填参数，其它可选参数根据需要使用 */
    msg_obj->caller = "group_change";       /* 自定义字符串，标记调用者 */
    ret = uf_client_call(msg_obj, &rbuf, NULL);
    printf("%s\n",rbuf);

#elif defined POPEN_CMD_ENABLE
	printf("%s\n",param_input);
    sprintf(cmd,"dev_sta get -m country_channel '%s'",param_input);
    printf("%s\r\n",cmd);
    execute_cmd(cmd,&rbuf);    
#endif

	output_param_root=json_tokener_parse(rbuf);
	
    

    if (band == PLATFORM_5G || band == PLATFORM_BOTH) {
        channel_5G_obj = json_object_object_get(output_param_root, "channel_5G");

        for (i = 0; i < json_object_array_length(channel_5G_obj); i++) {
        struct json_object *elem = json_object_array_get_idx(channel_5G_obj, i);
        frequency_obj = json_object_object_get(elem, "frequency");
		channel_obj = json_object_object_get(elem, "channel");
        strcpy(channel,json_object_get_string(channel_obj));
		printf("%s\r\n",channel);
        *bitmap_5G |= 1 << channel_to_bitmap(atoi(channel));  /*36 ~ 144    149 153 157 161 165 169 173 177 181*/
        }
    }
    if (band == PLATFORM_2G || band == PLATFORM_BOTH) {
        channel_2G_obj = json_object_object_get(output_param_root, "channel_2G");

        for (i = 0; i < json_object_array_length(channel_2G_obj); i++) {
            struct json_object *elem = json_object_array_get_idx(channel_2G_obj, i);
            frequency_obj = json_object_object_get(elem, "frequency");
            channel_obj = json_object_object_get(elem, "channel");
            strcpy(channel,json_object_get_string(channel_obj));
            printf("%s\r\n",channel);
            *bitmap_2G |= 1<<atoi(channel);
        }
    } 


    printf("bitmap_5G %d\n",*bitmap_5G);
    printf("bitmap_2G %d\n",*bitmap_2G);
    print_bits(*bitmap_5G);

	json_object_put(input_param_root);
    /* 资源需要调用者释放 */
    if (rbuf) {
      free(rbuf);
    }

#ifdef UNIFY_FRAMEWORK_ENABLE
    free(msg_obj);
#endif

	return SUCCESS;
}
#elif defined BRIDGE_PLATFORM
int get_country_channel_bitmap(int bw,long *bitmap_2G,long *bitmap_5G,int band)
{

#ifdef UNIFY_FRAMEWORK_ENABLE
    uf_cmd_msg_t *msg_obj;
#elif defined POPEN_CMD_ENABLE
    char cmd[MAX_POPEN_BUFFER_SIZE];
#endif     
    int ret;
    int channel_num;
    char*rbuf,*param_input;    
    char bw_str[20];

    
    struct json_object *list = json_object_new_array();
    json_object *input_param_root,*output_param_root;
    json_object *qry_type_obj,*band_obj;
	int i,p;

	json_object *frequency_obj,*channel_obj;
	char channel[8],frequency[8];

    rbuf = NULL;
    *bitmap_2G = 0;
    *bitmap_5G = 0;

	input_param_root = json_object_new_object();

    if (bw == 20) {
		json_object_object_add(input_param_root, "band", json_object_new_string("BW_20"));
    } else if (bw == 40) {
		json_object_object_add(input_param_root, "band", json_object_new_string("BW_40"));
    } else if (bw == 80) {
		json_object_object_add(input_param_root, "band", json_object_new_string("BW_80"));
    } else if (bw == 160) {
		json_object_object_add(input_param_root, "band", json_object_new_string("BW_160"));
    } else {
        return FAIL;
    }
    
    
    json_object_object_add(input_param_root, "qry_type", json_object_new_string("channellist"));
    json_object_object_add(input_param_root, "range", json_object_new_string("5G"));

    
    param_input = json_object_to_json_string(input_param_root);
	printf("%s\n",param_input);

#ifdef UNIFY_FRAMEWORK_ENABLE
    msg_obj = (uf_cmd_msg_t*)malloc(sizeof(uf_cmd_msg_t));
    if (msg_obj == NULL) {
        return FAIL;
    }
    memset(msg_obj, 0, sizeof(uf_cmd_msg_t));

    msg_obj->param = param_input;
    msg_obj->ctype = UF_DEV_STA_CALL;    /* 调用类型 ac/dev/.. */
    msg_obj->cmd = "get";
    msg_obj->module = "country_channel";               /* 必填参数，其它可选参数根据需要使用 */
    msg_obj->caller = "group_change";       /* 自定义字符串，标记调用者 */
    ret = uf_client_call(msg_obj, &rbuf, NULL);
    printf("%s\n",rbuf);

#elif defined POPEN_CMD_ENABLE
	printf("%s\n",param_input);
    sprintf(cmd,"dev_sta get -m country_channel '%s'",param_input);
    printf("%s\r\n",cmd);
    execute_cmd(cmd,&rbuf);    
#endif

	output_param_root=json_tokener_parse(rbuf);
    if (band == PLATFORM_5G || band == PLATFORM_BOTH) {
        channel_num = json_object_array_length(output_param_root);
        for (i = 0; i < channel_num; i++) {
        struct json_object *elem = json_object_array_get_idx(output_param_root, i);
        frequency_obj = json_object_object_get(elem, "frequency");
		channel_obj = json_object_object_get(elem, "channel");
        strcpy(channel,json_object_get_string(channel_obj));
		printf("%s\r\n",channel);
        *bitmap_5G |= 1 << channel_to_bitmap(atoi(channel));  /*36 ~ 144    149 153 157 161 165 169 173 177 181*/
        }
    }
    if (band == PLATFORM_2G || band == PLATFORM_BOTH) {
        channel_num = json_object_array_length(output_param_root);
        for (i = 0; i < channel_num; i++) {
            struct json_object *elem = json_object_array_get_idx(output_param_root, i);
            frequency_obj = json_object_object_get(elem, "frequency");
            channel_obj = json_object_object_get(elem, "channel");
            strcpy(channel,json_object_get_string(channel_obj));
            printf("%s\r\n",channel);
            *bitmap_2G |= 1<<atoi(channel);
        }
    }
    printf("bitmap_5G %d\n",*bitmap_5G);
    printf("bitmap_2G %d\n",*bitmap_2G);
    print_bits(*bitmap_5G);


	json_object_put(input_param_root);
    /* 资源需要调用者释放 */
    if (rbuf) {
      free(rbuf);
    }

#ifdef UNIFY_FRAMEWORK_ENABLE
    free(msg_obj);
#endif

	return channel_num;
}
#endif 



int channel_to_bitmap (int channel)
{
    if (channel >= 36 && channel <= 144) {
        return channel/4 - 9;
    }
    if (channel >= 149 && channel <= 181) {
        return (channel-1)/4 - 9;
    }
    return FAIL;
    
}

int bitmap_to_channel (int bit_set)
{
    if (bit_set >= 0 && bit_set <= 27) {
        return (bit_set + 9 ) * 4;
    }
    if (bit_set >= 28 && bit_set <= 45) {
        return (bit_set + 9) * 4 + 1;
    }
    return FAIL;
}
