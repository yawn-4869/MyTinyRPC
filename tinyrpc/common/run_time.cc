#include "tinyrpc/common/run_time.h"

namespace MyTinyRPC {

thread_local RunTime* t_run_time = NULL;

RunTime* RunTime::GetRunTime() {
    if(t_run_time) {
        return t_run_time;
    }

    t_run_time = new RunTime();
    return t_run_time;
} 
}