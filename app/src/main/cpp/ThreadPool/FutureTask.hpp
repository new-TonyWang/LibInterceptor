//
// Created by ttongyuwang on 2021/8/19.
//

#ifndef INTERCEPTOR_LIB_ENHANCED_FUTURETASK_HPP
#define INTERCEPTOR_LIB_ENHANCED_FUTURETASK_HPP

#include "thread_pool.hpp"

class FutureTask {
public:
    void * get();
private:
    std::mutex m_mutex;
    const task_t* task_t;
    void * param;
    void * return_value;


};


#endif //INTERCEPTOR_LIB_ENHANCED_FUTURETASK_HPP
