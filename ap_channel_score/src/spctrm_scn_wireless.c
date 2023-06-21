#include "spctrm_scn_wireless.h"



extern unsigned char g_mode;
extern struct device_list g_finished_device_list;
extern struct device_list g_device_list;
extern struct channel_info g_channel_info_5g[MAX_BAND_5G_CHANNEL_NUM];
extern struct channel_info realtime_channel_info_5g[36];

extern struct user_input g_input;
volatile int g_status,g_scan_time;
extern volatile long g_scan_timestamp;
extern long g_bitmap_2G,g_bitmap_5G;
extern pthread_mutex_t g_mutex;
extern sem_t g_semaphore;
time_t g_current_time;

void print_bits(long num) {
    int i;

    for (i = 0; i < sizeof(long) * 8; i++) {
        if ((num & (1L << i)) != 0) {
            printf("Bit %d is set\n", i);
        }
    }
}

#ifdef AP
int spctrm_scn_wireless_country_channel(int bw,long *bitmap_2G,long *bitmap_5G,int band)
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
    spctrm_scn_common_cmd(cmd,&rbuf);    
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
int spctrm_scn_wireless_country_channel(int bw,long *bitmap_2G,long *bitmap_5G,int band)
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
    spctrm_scn_common_cmd(cmd,&rbuf);    
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



static inline int channel_to_bitmap (int channel)
{
    if (channel >= 36 && channel <= 144) {
        return channel/4 - 9;
    }
    if (channel >= 149 && channel <= 181) {
        return (channel-1)/4 - 9;
    }
    return FAIL;
    
}

static inline int bitmap_to_channel (int bit_set)
{
    if (bit_set >= 0 && bit_set <= 27) {
        return (bit_set + 9 ) * 4;
    }
    if (bit_set >= 28 && bit_set <= 45) {
        return (bit_set + 9) * 4 + 1;
    }
    return FAIL;
}


void *spctrm_scn_wireless_scan_thread(void *arg) 
{

    char *json_str;
    int i,j,len;
    double score;
    struct channel_info current_channel_info;

    while (1) {
        sem_wait(&g_semaphore);
        
        if (g_status == SCAN_BUSY) {
            spctrm_scn_wireless_channel_info(&current_channel_info,PLATFORM_5G);
            spctrm_scn_dev_wds_list(&g_device_list);
            memset(realtime_channel_info_5g,0,sizeof(realtime_channel_info_5g));
            for (j = 0,i = 0; i < sizeof(long) * ONE_BYTE; i++) {
                if ((g_input.channel_bitmap& (1L << i)) != 0) {
                    
                    realtime_channel_info_5g[j].channel = bitmap_to_channel(i);

                    debug("change to channel : %d",realtime_channel_info_5g[j].channel);
                    
                    spctrm_scn_wireless_change_channel(realtime_channel_info_5g[j].channel);

                    channel_scan(&realtime_channel_info_5g[j],g_input.scan_time);
                    sleep(1);

                    debug("g_input.channel_bitmap : %ld",g_input.channel_bitmap);
                    realtime_channel_info_5g[j].score = spctrm_scn_wireless_channel_score(&realtime_channel_info_5g[j]);
                    printf("score %f\r\n",realtime_channel_info_5g[j].score);
                    printf("------------------\r\n");
                    j++;  
                }
            }
            
            /* timestamp */
            g_current_time = time(NULL);

            spctrm_scn_wireless_change_channel(current_channel_info.channel);
            
            spctrm_scn_dev_reset_stat(&g_device_list);
        	/* find AP */
	        i = spctrm_scn_dev_find_ap(&g_device_list);
            g_device_list.device[i].finished_flag = FINISHED;
            
            if (timeout_func() == FAIL) {
                pthread_mutex_lock(&g_mutex);
                g_status = SCAN_TIMEOUT;
                g_input.scan_time = MIN_SCAN_TIME; /* restore scan time */
                pthread_mutex_unlock(&g_mutex);
            } else {
                printf( "line : %d func %s g_status : %d,",__LINE__,__func__,g_status);
                pthread_mutex_lock(&g_mutex);
                memcpy(g_channel_info_5g,realtime_channel_info_5g,sizeof(realtime_channel_info_5g));
                /* 将CPE端的数据存入完成列表，此时AP端的数据还在g_channel_info中 */
                debug("g_finished_device_list.list_len %d",g_finished_device_list.list_len);
                memcpy(&g_finished_device_list,&g_device_list,sizeof(struct device_list));
                debug("g_finished_device_list.list_len %d",g_finished_device_list.list_len);
                g_status = SCAN_IDLE;
                g_input.scan_time = MIN_SCAN_TIME; /* restore scan time */
                pthread_mutex_unlock(&g_mutex);
            }

        }
    }
}

void *scan_func() 
{

    char *json_str;

    
    int i,j,len;
    double score;
    struct channel_info current_channel_info;
    
    
    while (1) {
        sem_wait(&g_semaphore);
     
        if (g_status == SCAN_BUSY) {
            /* timestamp */
            g_current_time = time(NULL);   
            debug("CPE SCAN START");
            spctrm_scn_wireless_channel_info(&current_channel_info,PLATFORM_5G);
            memset(realtime_channel_info_5g,0,sizeof(realtime_channel_info_5g));
            for (j = 0,i = 0; i < sizeof(long) * ONE_BYTE; i++) {
                if ((g_input.channel_bitmap& (1L << i)) != 0) {
                    
                    realtime_channel_info_5g[j].channel = bitmap_to_channel(i);
                    debug("change channel to %d ",realtime_channel_info_5g[j].channel);
                    
                    spctrm_scn_wireless_change_channel(realtime_channel_info_5g[j].channel);

                    channel_scan(&realtime_channel_info_5g[j],g_input.scan_time);
                    sleep(1);
                    printf("%ld\r\n",g_input.channel_bitmap);
                    realtime_channel_info_5g[j].score = spctrm_scn_wireless_channel_score(&realtime_channel_info_5g[j]);
                    printf("------------------\r\n");
                    j++;  
                }

                if (g_status == SCAN_TIMEOUT) {
                    goto timeout;
                }
            }
            

            spctrm_scn_wireless_change_channel(current_channel_info.channel);

            
            pthread_mutex_lock(&g_mutex);
            
            memcpy(g_channel_info_5g,realtime_channel_info_5g,sizeof(realtime_channel_info_5g));
            memset(realtime_channel_info_5g,0,sizeof(realtime_channel_info_5g));
            g_status = SCAN_IDLE;
            g_input.scan_time = MIN_SCAN_TIME; /* restore scan time */

            pthread_mutex_unlock(&g_mutex);

        }
timeout:
    if (g_status == SCAN_TIMEOUT) {
            spctrm_scn_wireless_change_channel(current_channel_info.channel);
            pthread_mutex_lock(&g_mutex);
            g_status = SCAN_IDLE;
            g_input.scan_time = MIN_SCAN_TIME; /* restore scan time */
            memset(realtime_channel_info_5g,0,sizeof(realtime_channel_info_5g));
            pthread_mutex_unlock(&g_mutex);
        }
    }
}
int quick_select(int* arr, int len, int k)
{
    int pivot, i, j, tmp;

    pivot = arr[len / 2];

    for (i = 0, j = len - 1;; i++, j--) {
        while (arr[i] < pivot) i++;
        while (arr[j] > pivot) j--;
        if (i >= j) break;
        tmp = arr[i];
        arr[i] = arr[j];
        arr[j] = tmp;
    }

    if (i == k - 1) {
        return pivot;
    }
    if (i < k - 1) {
        return quick_select(arr + i, len - i, k - i);
    }

    return quick_select(arr, i, k);
}


int median(int* arr, int len)
{
    int median;

    if (len % 2 == 0) {
        median = (quick_select(arr, len, len / 2) + quick_select(arr, len, len / 2 + 1)) / 2;
    } else {
        median = quick_select(arr, len, len / 2 + 1);
    }
        
    return median;
}

int spctrm_scn_wireless_channel_info(struct channel_info *info,int band) 
{
    FILE *fp;
    char *rbuf;
    char *p;
    char cmd[1024];

    if (info == NULL) {
         return FAIL;
    }

    if (band == 5) {
#ifdef BRIDGE_PLATFORM
        spctrm_scn_common_cmd("wlanconfig ra0 radio",&rbuf);
#elif defined AP
        spctrm_scn_common_cmd("wlanconfig rax0 radio",&rbuf);
#endif
    } else if (band == 2) {

    } else {
        return FAIL;
    }
    
    
    strtok(rbuf,"\n");

    strtok(NULL,":");
    p = strtok(NULL,"\n");


    info->channel=atoi(p);
      
    strtok(NULL,":");
    p = strtok(NULL,"\n");
    info->floornoise=atoi(p);
     
    strtok(NULL,":");
    p = strtok(NULL,"\n");
    info->utilization=atoi(p);
     
    strtok(NULL,"\n");
    strtok(NULL,":");
    p = strtok(NULL,"\n");
    info->bw = atoi(p);
    
    strtok(NULL,":");
    p = strtok(NULL,"\n");
    info->obss_util=atoi(p);
    
    strtok(NULL,":");
    p = strtok(NULL,"\n");
    info->tx_util=atoi(p);
   
    strtok(NULL,":");
    p = strtok(NULL,"\n");

    info->rx_util=atoi(p);
    
    free(rbuf);

    return SUCCESS;
}

void channel_scan(struct channel_info *input,int scan_time) 
{
    FILE *fp;
    int i;
    struct channel_info info[MAX_SCAN_TIME];
    int utilization_temp[MAX_SCAN_TIME];
    int obss_util_temp[MAX_SCAN_TIME];
    int floornoise_temp[MAX_SCAN_TIME];
    int channel_temp[MAX_SCAN_TIME];
    
    if (scan_time > MAX_SCAN_TIME) {
        scan_time = MAX_SCAN_TIME;
    } 

    if (scan_time < MIN_SCAN_TIME) {
        scan_time = MIN_SCAN_TIME;
    }     

    for (i = 0 ;i < scan_time ;i++) {
        sleep(1);

        spctrm_scn_wireless_channel_info(&info[i],PLATFORM_5G);
        debug("current channel %d",info[i].channel);
    }
    
    fp = fopen("./channel_info","a+");
    fprintf(fp,"\r\n********channel %d ***********\r\n",info[0].channel);
    fprintf(fp,"bw %d\r\n",input->bw=info[0].bw);

    fprintf(fp,"channel\t\t");
    fprintf(fp,"floornoise\t");
    fprintf(fp,"utilization\t");
    fprintf(fp,"obss_util\t");
    fprintf(fp,"rx_util\t\t");
    fprintf(fp,"tx_util\r\n");

    for (i = 0 ;i < scan_time ;i++) {
        fprintf(fp,"%d\t\t",channel_temp[i] = info[i].channel);
        fprintf(fp,"%d\t\t",floornoise_temp[i] = info[i].floornoise);
        fprintf(fp,"%d\t\t",utilization_temp[i] = info[i].utilization);
        fprintf(fp,"%d\t\t",obss_util_temp[i] = info[i].obss_util);
        fprintf(fp,"%d\t\t",info[i].rx_util);
        fprintf(fp,"%d\r\n",info[i].tx_util);
    }

    printf("median floornoise%d \r\n",input->floornoise = median(floornoise_temp,scan_time));
    printf("median utilization%d \r\n",input->utilization = median(utilization_temp,scan_time));
    printf("median obss_util%d \r\n",input->obss_util = median(obss_util_temp,scan_time));
    
    fprintf(fp,"*******************************************************\r\n");
    fclose(fp);

    debug("g_status %d",g_status);

}
    


void spctrm_scn_wireless_wds_state() 
{
    char *rbuf;
	json_object *rbuf_root;
    json_object *role_obj;

    char role[4];

    spctrm_scn_common_cmd("dev_sta get -m wds_status", &rbuf);
    rbuf_root = json_tokener_parse(rbuf);

    role_obj = json_object_object_get(rbuf_root,"role");

    if (strcmp(json_object_get_string(role_obj),"cpe") == 0) {
        g_mode = CPE_MODE;
    } else if (strcmp(json_object_get_string(role_obj),"ap") == 0) {
        g_mode = AP_MODE;
    }

    free(rbuf);
    debug("g_mode %d",g_mode);
    
    

}