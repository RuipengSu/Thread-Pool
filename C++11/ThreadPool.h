#ifndef THREADPOOL_H_INCLUDED
#define THREADPOOL_H_INCLUDED

#include <vector>
#include <queue>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <future>
#include <functional>

class ThreadPool{
public:
    ThreadPool(size_t); //���캯����size_t��ʾ������

    template<class F, class... Args>
    auto enqueue(F&& f, Args&&... args) //����ܵ�����
    -> std::future<typename std::result_of<F(Args...)>::type>;  //����β���޶���  std future������ȡ�첽����Ľ��

    ~ThreadPool();

private:
    std::vector<std::thread> workers;   //׷���߳�
    std::queue<std::function<void()>> tasks; //������У����ڴ��û�д���������ṩ�������

    //ͬ��
    std::mutex queue_mutex; //������
    std::condition_variable condition;  //��������
    bool stop;
};

inline ThreadPool::ThreadPool(size_t threads): stop(false)
{
    for(size_t i = 0; i < threads; ++i)
    {
        workers.emplace_back(   //����һ���߳�
            [this]
            {
                for(;;)
                {
                    std::function<void()> task; //�߳��еĺ�������
                    {
                        //�����ţ���ʱ�����������ڣ�����lock��ʱ��
                        std::unique_lock<std::mutex> lock(this->queue_mutex);
                        this->condition.wait(lock,
                            [this]{return this->stop || !this->tasks.empty();});
                        if(this->stop && this->tasks.empty())   return;
                        task = std::move(this->tasks.front());
                        this->tasks.pop();
                    }
                    task(); //���ú��������к���
                }
            }
        );
    }
}

//���̳߳��м����¹���
template<class F, class... Args>
auto ThreadPool::enqueue(F&& f, Args&&... args) //�����޶�������������ֵ���ã�  �˴���ʾ��������һ������
-> std::future<typename std::result_of<F(Args...)>::type>
{
    using return_type = typename std::result_of<F(Args...)>::type;
    //packaged_task�Ƕ������һ���������ǿ��Ը��䴫��һ������������乹�졣֮������Ͷ�ݸ��κ��߳�ȥ��ɣ�ͨ��
    //packaged_task.get_future()������ȡ��future����ȡ������ɺ�Ĳ���ֵ
    auto task = std::make_shared<std::packaged_task<return_type()> >(   //ָ��F����������ָ��
        std::bind(std::forward<F>(f), std::forward<Args>(args)...)  //���ݺ������й���
                                                                         );
    //futureΪ������get_future��ȡ������ɺ�Ĳ���ֵ
    std::future<return_type> res = task->get_future();  //��ȡfuture�������task��״̬��ready����������ǰ������
    {
        std::unique_lock<std::mutex> lock(queue_mutex); //���ֻ����ԣ��������߳�ִ��ͬһ������

        if(stop)    throw std::runtime_error("enqueue on stopped ThreadPool");
        tasks.emplace([task](){ (*task)(); });  //��task�����߳�ȥ��ɣ�vectorβ��ѹ��
    }
    condition.notify_one(); //ѡ��һ��wait״̬���߳̽��л��ѣ���ʹ����ö����ϵ������������(�������߳��޷����ʶ���)
    return res;
}//notify_one���ܱ�֤��������߳�������Ҫ����������˿��ܲ�������

//�����������������߳�
inline ThreadPool::~ThreadPool()
{
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        stop = true;
    }
    condition.notify_all(); //֪ͨ����wait״̬���߳̾�������Ŀ���Ȩ�����������߳�ִ��
    for(std::thread &worker: workers)   worker.join();  //��Ϊ�̶߳���ʼ�����ˣ�����һ����ִ���꣬join�ɵȴ��߳�ִ����
}


#endif // THREADPOOL_H_INCLUDED
