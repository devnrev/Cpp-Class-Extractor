//
//  Created by Axel on 05.05.12.
//


#ifndef __ConcurrentQueue_H_
#define __ConcurrentQueue_H_
#include <mutex>
#include <queue>
#include <condition_variable>

namespace Container{
namespace Concurrent{

/*
 * Single consumer thread safe queue
 */

template<class T>
class ConcurrentQueue {
public:
    ConcurrentQueue();

    void emplace(T&& elem);
    T const& front();
    void pop();
    size_t getSize();
    bool isEmpty();
    void waitForData();

protected:

private:
    std::mutex mutex_;
    std::queue<T> queue_;
    std::condition_variable notEmpty_;
};


template <class T>
ConcurrentQueue<T>::ConcurrentQueue() {

}
template <class T>
void ConcurrentQueue<T>::pop() {
    std::lock_guard<std::mutex> lock(mutex_);
    queue_.pop();
}

template <class T>
bool  ConcurrentQueue<T>::isEmpty() {
    std::lock_guard<std::mutex> lock(mutex_);
    return queue_.empty();
}

template <class T>
size_t ConcurrentQueue<T>::getSize() {
    std::lock_guard<std::mutex> lock(mutex_);
    return queue_.size();
}

template <class T>
void ConcurrentQueue<T>::waitForData(){
    std::unique_lock<std::mutex> lock(mutex_);
    if (queue_.empty()){
        notEmpty_.wait_for(lock,std::chrono::milliseconds(500));
    }
}

template <class T>
void ConcurrentQueue<T>::emplace(T&& elem){
    std::lock_guard<std::mutex> lock(mutex_);
    bool wasEmpty = queue_.empty();
    queue_.emplace(std::forward<T>(elem));
    if (wasEmpty){
        notEmpty_.notify_one();
    }
}

template <class T>
T const& ConcurrentQueue<T>::front(){
    std::lock_guard<std::mutex> lock(mutex_);
    return queue_.front();
}



}
}
#endif //__ConcurrentQueue_H_
