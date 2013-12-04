#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <thread>
#include <queue>

using namespace std;

namespace COS518 {
    class ThreadPool {
        private:
    
        queue<thread> *pool;
    
        public:
        ThreadPool(function<void()>, int);
       ~ThreadPool();
        void join();
    };
}

#endif
