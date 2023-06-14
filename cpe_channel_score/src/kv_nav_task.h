
#ifndef _KV_NAV_COMMON_DEAL_TASK_H_
#define _KV_NAV_COMMON_DEAL_TASK_H_
#include "list.h"

#define uint32_t unsigned int
#define uint16_t unsigned short
#define uint8_t  unsigned char

#define KV_NAV_SUC              0 
#define KV_NAV_ERR              -1  
#define KV_NAV_NOMEM            -2 
#define KV_NAV_INVALID_PARAM    -3
#define KV_NAV_COMMON_MEM_ALLOC(a,b) malloc(b)
#define KV_NAV_COMMON_MEM_FREE(a,b) free(b)
#define KV_NAV_LOG_E(a,b,c,...)     printf(c,##__VA_ARGS__)
#define KV_NAV_LOG_MAC(a,b,c,d,...) printf(d,##__VA_ARGS__)

typedef int (*kv_nav_task_deal_func_t) (void *handle, void *peer, unsigned short msg_type, char *buf, int len);
typedef void (*kv_nav_task_hold_resource_func_t) (void *handel);
typedef void (*kv_nav_task_release_resource_func_t) (void *handel);

#define THREAD_NUM_DEFAULT              4
#define PROC_NUM_DEFAULT                128
#define PROC_NUM_DEFAULT_STA_INFO       32
#define THREAD_NAME_SIZE                64
#define TASK_IN_LIST_MAX                1000

typedef enum {
    MODULE_CHANNEL_SCAN,
    MODE_MAX
} kv_task_moudle_e;

typedef struct kv_nav_task_info {
    struct list_head list;
    pthread_mutex_t list_lock;
    pthread_cond_t list_convar;
    pthread_t pthread;
    int pthread_idx; 
    kv_task_moudle_e moudle;
    int task_in_list;                      /* use to limet max task in list*/
} kv_nav_task_info_t;

typedef struct kv_nav_task_init_info {
    int pthread_num;                                 /* ptheread number in task moudle */
    int task_cnt;                                    /* use for load balance task in pthread*/
    int per_proc_num;                                /* can deal number of task in per proc */
    kv_nav_task_deal_func_t deal_func;                 /* deal function */
    kv_nav_task_hold_resource_func_t hold_func;        /* hold function */
    kv_nav_task_release_resource_func_t release_func;  /* hold function */
    kv_nav_task_info_t task[0];
} kv_nav_task_init_info_t;

typedef struct kv_nav_task_pkt {
    struct list_head list;
    void *handle;
    void *peer;
    uint16_t msg_type;
    void *buf;
    int len;
} kv_nav_task_pkt_t;

/**
 * @brief kv_nav_task_register register mulity threads kv nav task deal 
 * 
 * @param moudle        moudle name
 * @param pthread_num   register pthreads num
 * @param per_proc_num  register deal number of task in per proc
 * @param deal_func     deal function
 * @param hold_func     hold function (can be NULL)
 * @param release_func  release_func function (can be NULL)
 * @return *** int SUCCESS : 0 , ERROR: -1  KV_NAV_NOMEM: -2 KV_NAV_INVALID_PARAM: -3
 */
int kv_nav_task_register(kv_task_moudle_e moudle, uint32_t pthread_num, uint32_t per_proc_num, kv_nav_task_deal_func_t deal_func,
    kv_nav_task_hold_resource_func_t hold_func, kv_nav_task_release_resource_func_t release_func);

/**
 * @brief kv_nav_task_usr_add add task to deal moudle
 * 
 * @param moudle moudle name
 * @return *** int SUCCESS : 0 , ERROR: -1  KV_NAV_NOMEM: -2 KV_NAV_INVALID_PARAM: -3
 */
int kv_nav_task_usr_add(kv_task_moudle_e moudle, void *handle, void *peer, uint16_t msg_type, char *buf, uint32_t len);

/**
 * @brief kv_nav_task_unregister  unregister mulity threads kv nav task deal
 * 
 * @param moudle moudle name
 */
void kv_nav_task_unregister(uint16_t  moudle);

#endif /* _KV_NAV_COMMON_DEAL_TASK_H_ */ 

