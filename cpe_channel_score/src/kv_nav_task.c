#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <sys/prctl.h>
#include <linux/types.h>
#include "kv_nav_task.h"

kv_nav_task_init_info_t *g_task_info[MODE_MAX];

static void *kv_nav_task_proc(kv_task_moudle_e moudle, kv_nav_task_info_t *task_info)
{
    int msg_count;
    kv_nav_task_pkt_t *pos, *next;
    bool flag = false;
    msg_count = 0;

    pthread_mutex_lock(&task_info->list_lock);
    list_for_each_entry_safe(pos, next, &task_info->list, list) {
        g_task_info[moudle]->deal_func(pos->handle, pos->peer, pos->msg_type, pos->buf, pos->len);
        /* release msg */
        if (g_task_info[moudle]->release_func != NULL) {
            g_task_info[moudle]->release_func(pos->handle);
        }

        list_del(&pos->list);
        KV_NAV_COMMON_MEM_FREE(KV_NAV_MOD_TASK, pos);
        __sync_fetch_and_sub(&task_info->task_in_list, 1);
        /* limit number of task in per proc */
        if (msg_count++ > g_task_info[moudle]->per_proc_num) {
            flag = true;
            break;
        }
    }

    pthread_mutex_unlock(&task_info->list_lock);
    if (flag) {
        pthread_cond_signal(&task_info->list_convar);
    }

    return NULL;
}

static void *kv_nav_task_deal(void *argv)
{

    kv_nav_task_info_t *task_info;
    task_info = (kv_nav_task_info_t *)argv;
    char thread_name[THREAD_NAME_SIZE];

    pthread_detach(pthread_self());
    sprintf(thread_name, "kv_nav_task_deal_moudle_%d_index_%d ", task_info->moudle, task_info->pthread_idx);
    prctl(PR_SET_NAME, thread_name);
    while (1) {
        pthread_mutex_lock(&task_info->list_lock);
        while (list_empty(&task_info->list)) {
            pthread_cond_wait(&task_info->list_convar, &task_info->list_lock);
        }

        pthread_mutex_unlock(&task_info->list_lock);
        kv_nav_task_proc(task_info->moudle, task_info);
    }

    return NULL;
}

int kv_nav_task_register(kv_task_moudle_e moudle, uint32_t pthread_num, uint32_t per_proc_num, kv_nav_task_deal_func_t deal_func,
    kv_nav_task_hold_resource_func_t hold_func, kv_nav_task_release_resource_func_t release_func)
{
    int ret;
    int i;

    if (moudle >= MODE_MAX) {
        return KV_NAV_INVALID_PARAM;
    }

    g_task_info[moudle] = KV_NAV_COMMON_MEM_ALLOC(KV_NAV_MOD_TASK, sizeof(kv_nav_task_init_info_t) + pthread_num * sizeof(kv_nav_task_info_t));
    if (g_task_info[moudle] == NULL) {
        KV_NAV_LOG_E(KV_NAV_TASK, M_AC_KVROAM, "kv_nav_task_register malloc fail");
        return KV_NAV_NOMEM;
    }

    g_task_info[moudle]->pthread_num = (pthread_num == 0) ? THREAD_NUM_DEFAULT : pthread_num;
    g_task_info[moudle]->per_proc_num = (per_proc_num == 0) ? PROC_NUM_DEFAULT : per_proc_num;
    g_task_info[moudle]->deal_func = deal_func;
    g_task_info[moudle]->hold_func = hold_func;
    g_task_info[moudle]->release_func = release_func;
    g_task_info[moudle]->task_cnt = 0;
    for (i = 0; i < g_task_info[moudle]->pthread_num; i++) {
        INIT_LIST_HEAD(&g_task_info[moudle]->task[i].list);
        g_task_info[moudle]->task[i].pthread_idx = i;
        g_task_info[moudle]->task[i].moudle = moudle;
        g_task_info[moudle]->task[i].task_in_list = 0;
        ret = pthread_cond_init(&g_task_info[moudle]->task[i].list_convar, NULL);
        if (ret < 0) {
             KV_NAV_LOG_E(KV_NAV_TASK, M_AC_KVROAM, "pthread_cond_init fail");
            goto exit;
        }
        
        ret = pthread_mutex_init(&g_task_info[moudle]->task[i].list_lock, NULL);
        if (ret < 0) {
            KV_NAV_LOG_E(KV_NAV_TASK, M_AC_KVROAM, "pthread_mutex_init fail");
            pthread_cond_destroy(&g_task_info[moudle]->task[i].list_convar);
            goto exit;
        }

        ret = pthread_create(&g_task_info[moudle]->task[i].pthread, NULL, kv_nav_task_deal, &g_task_info[moudle]->task[i]);
        if (ret < 0) {
            KV_NAV_LOG_E(KV_NAV_TASK, M_AC_KVROAM, "pthread_create fail");
            pthread_mutex_destroy(&g_task_info[moudle]->task[i].list_lock);
            pthread_cond_destroy(&g_task_info[moudle]->task[i].list_convar);
            goto exit;
        }
    }

    return KV_NAV_SUC;

exit:
    while(--i >= 0) {
        pthread_mutex_destroy(&g_task_info[moudle]->task[i].list_lock);
        pthread_cond_destroy(&g_task_info[moudle]->task[i].list_convar);
        pthread_cancel(g_task_info[moudle]->task[i].pthread);
    }

    KV_NAV_COMMON_MEM_FREE(KV_NAV_MOD_TASK, g_task_info[moudle]);
    return KV_NAV_ERR;
}

static int kv_nav_add_task(kv_task_moudle_e moudle, int index, void *handle, void *peer, unsigned short msg_type, char *buf, int len)
{
    kv_nav_task_pkt_t *task_pkt;

    if (buf == NULL) {
        return KV_NAV_INVALID_PARAM;
    }

    if (g_task_info[moudle]->task[index].task_in_list > TASK_IN_LIST_MAX) {
        KV_NAV_LOG_MAC(DBG_ERROR, M_AC_KVROAM, KV_NAV_DEBUG_MAC, "list full task_in_list is %d", g_task_info[moudle]->task[index].task_in_list);
        return KV_NAV_NOMEM;
    }

    task_pkt = (kv_nav_task_pkt_t *)KV_NAV_COMMON_MEM_ALLOC(KV_NAV_MOD_TASK, sizeof(kv_nav_task_pkt_t));
    if (task_pkt == NULL) {
        KV_NAV_LOG_MAC(DBG_ERROR, M_AC_KVROAM, KV_NAV_DEBUG_MAC, "kv_nav_task_pkt_t malloc fail");
        return KV_NAV_NOMEM;
    }

    memset(task_pkt, 0x00, sizeof(kv_nav_task_pkt_t));
    task_pkt->handle = handle;
    task_pkt->peer = peer;
    task_pkt->msg_type = msg_type;
    task_pkt->buf = buf;
    task_pkt->len = len;
    if (g_task_info[moudle]->hold_func != NULL) {
        g_task_info[moudle]->hold_func(handle);
    }

    __sync_fetch_and_add(&g_task_info[moudle]->task[index].task_in_list, 1);
    pthread_mutex_lock(&g_task_info[moudle]->task[index].list_lock);
    list_add_tail(&task_pkt->list, &g_task_info[moudle]->task[index].list);
    pthread_mutex_unlock(&g_task_info[moudle]->task[index].list_lock);
    pthread_cond_signal(&g_task_info[moudle]->task[index].list_convar);

    return KV_NAV_SUC;
}

int kv_nav_task_usr_add(kv_task_moudle_e moudle, void *handle, void *peer, unsigned short msg_type, char *buf, uint32_t len)
{

    if (++g_task_info[moudle]->task_cnt == g_task_info[moudle]->pthread_num) {
        g_task_info[moudle]->task_cnt = 0;
    }

    if (g_task_info[moudle]->task_cnt < g_task_info[moudle]->pthread_num) {
        return kv_nav_add_task(moudle, g_task_info[moudle]->task_cnt, handle, peer, msg_type, buf, len);
    }

    return KV_NAV_ERR;
}

void kv_nav_task_unregister(unsigned short  moudle)
{
    kv_nav_task_pkt_t *pos, *next;
    int i;

    if (g_task_info[moudle] != NULL) {
        for (i = 0; i < g_task_info[moudle]->pthread_num; i++) {
            if (!list_empty(&g_task_info[moudle]->task[i].list)) {
                pthread_mutex_lock(&g_task_info[moudle]->task[i].list_lock);
                list_for_each_entry_safe(pos, next, &g_task_info[moudle]->task[i].list, list) {
                    list_del_init(&pos->list);
                    KV_NAV_COMMON_MEM_FREE(KV_NAV_MOD_TASK, pos);
                }

                pthread_mutex_unlock(&g_task_info[moudle]->task[i].list_lock);
                pthread_mutex_destroy(&g_task_info[moudle]->task[i].list_lock);
                pthread_cond_destroy(&g_task_info[moudle]->task[i].list_convar);
                pthread_cancel(g_task_info[moudle]->task[i].pthread);
            }
        }

        KV_NAV_COMMON_MEM_FREE(KV_NAV_MOD_TASK, g_task_info[moudle]);
        g_task_info[moudle] = NULL;
    }
}

