#include "ThreadPool.h"

namespace COS518 {
    ThreadPool::ThreadPool(function<void()> f, int size) {
        pool = new queue<thread>();
        
        for (int i = 0; i < size; i++) {
            pool->push(thread(f));
        }
    }
    
    void ThreadPool::join() {
        while (!pool->empty()) {
            pool->front().join();
            pool->pop();
        }
    }    
    
    ThreadPool::~ThreadPool() {
        join();       
        delete pool;
    }
}
