### 日志模块开发 20240111-20240115

日志模块
```
1. 日志级别: DEBUG INFO ERR
2. 日志输出文件名（支持日期命名），日志滚动（若日志文件过大，是输出到新文件？还是覆盖？）
3. c 格式化风格 printf + 符号，比c++的流式输出更加便利
4. 线程安全(待完善): 
    * 对于日志实例g_logger, 在一开始就调用Init方法创建，
    后续所有线程调用都使用Get方法来获取日志实例，
    避免因多线程访问到只为NULL的g_logger后创建多个日志实例
    * 对于日志缓冲池，用封装好的互斥锁类进行加锁访问
5. 异步日志（待完善）
```

LogLevel: 
使用枚举类型来定义日志级别，共三个级别
```
DEBUG
INFO
ERROR
```

LogEvent:
```
文件名、行号
MsgNo: rpc请求信息的符号(待完善)
线程id、进程号
日期与时间 精确到毫秒
自定义消息: 输出的字符串信息
```

日志格式
```
[LogLevel][%y-%m-%d %H:%M:%S]\t[pid:thread_id]\t[filename:line][%msg]
```

Logger 日志器
```
1. 提供打印日志的方法
2. 设置打印日志的路径（目前为printf输出到控制台，需要完善成输出到指定文件）
```

### 配置模块开发 20240115
通过第三方库tinyxml读取xml配置文件

### 互斥锁的封装 20240116
包含互斥锁的类Mutex和调用的类ScopeLocker, 但感觉不是很好，后续看情况完善

### 事件循环EventLoop 20240124-20240125
选取的事件循环模型：Reactor
实现伪代码:
```c++
void loop() {
    // 开启主循环
    while(!stop) {
        // 清理任务
        for(task : tasks) {
            task();
        }
        // 1.判断超时 时间间隔为设定的最大时间与下次定时任务的时间的最大值
        int time_out = Max(1000, getNextTimerCallback());
        int rt = epoll_wait(epfd, fds, ..., time_out);
        if(rt < 0) {
            // epoll调用失败
        } else {
            foreach(fd in fds) {
                tasks.push(fd);
            }
        }
    }
}
```

由于本项目使用的是主从Reactor模型，因此会存在跨线程的IO操作，从而引发线程安全问题
以下代码提供了因跨线程IO操作引发的线程安全问题的避免方法
```c++
void EventLoop::addEpollEvent(FdEvent* event) {
    if(isInLoopThread()) {
        // 在主线程中调用epoll事件，直接添加
        ADD_TO_EPOLL();
    } else {
        // 在子线程中调用epoll事件，将添加epoll事件的回调函数加入到子线程的任务队列
        // 只有在loop的epoll_wait成功返回后，清理完毕任务，才能将事件添加成功
        auto cb = [this, event]() {
            ADD_TO_EPOLL()
        };
        addTask(cb);
    }
}
```