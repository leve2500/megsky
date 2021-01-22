

/*=====================================================================
 * �ļ���thread_dev_insert.h
 *
 * ����������ִ���߳�
 *
 * ���ߣ�����			2020��9��27��17:10:06
 *
 * �޸ļ�¼��
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
//�����ϱ��ӿ�
void sg_send_link_down(char *reason);

//��ʱ������
//����
//������ʱ��

bool sg_calovertime(time_cnt_info_s *info);
int sg_poly_thread(void);




#ifdef __cplusplus
}
#endif 

#endif
