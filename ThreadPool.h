#ifndef THREADPOOL_H_INCLUDED
#define THREADPOOL_H_INCLUDED

//�̳߳�ͷ�ļ�

#include "condition.h"

//��װ�̳߳��еĶ�����Ҫִ�е��������
typedef struct task
{
    void *(*run)(void *args);  //����ָ�룬��Ҫִ�е�����
    void *arg;              //����
    struct task *next;      //�����������һ������
}task_t;


//�������̳߳ؽṹ��
typedef struct threadpool
{
    condition_t ready;    //״̬��
    task_t *first;       //��������е�һ������
    task_t *last;        //������������һ������
    int counter;         //�̳߳��������߳���
    int idle;            //�̳߳���kongxi�߳���
    int max_threads;     //�̳߳�����߳���
    int quit;            //�Ƿ��˳���־
}threadpool_t;


//�̳߳س�ʼ��
void threadpool_init(threadpool_t *pool, int threads);

//���̳߳��м�������
void threadpool_add_task(threadpool_t *pool, void *(*run)(void *arg), void *arg);

//�ݻ��̳߳�
void threadpool_destroy(threadpool_t *pool);

#endif // THREADPOOL_H_INCLUDED
