

/*=====================================================================
 * 文件：thread_dev_insert.h
 *
 * 描述：任务执行线程
 *
 * 作者：田振超			2020年9月27日17:10:06
 *
 * 修改记录：
 =====================================================================*/


#ifndef _DEV_INSERT_H_
#define _DEV_INSERT_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef struct time_cnt_info {
    int order;
    clock_t m_dwLastCount;
    bool    m_bValidFlag;
    uint32_t m_dwDiffParam;
}time_cnt_info_s;

int sg_dev_insert_thread(void);

int sg_get_dev_ins_flag(void);
void sg_set_dev_ins_flag(int flag);
//离线上报接口
void sg_send_link_down(char *reason);

//延时任务处理
//链表
//启动定时器

bool sg_calovertime(time_cnt_info_s *info);
int sg_poly_thread(void);




#ifdef __cplusplus
}
#endif 

#endif
