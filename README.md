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

### 定时器 20240126
#### 定时任务 TimerEvent
类成员变量包含：
```
1. arrive_time 指定时间，通过该变量来判定定时任务是否超时 ms
2. interval 定时时间 ms
3. is_repeated 是否为周期性任务，需要重复执行
4. is_canceled 任务取消标志
5. task 回调函数
```
类成员函数包含：
```
cancel()
cancelRepeated()
```
#### 定时器 Timer
定时任务的集合，用于管理定时任务

为了在eventloop监听Timer事件, Timer需要继承FdEvent

### IO线程封装 20240131
创建一个io线程，他会帮我们执行: 
1. 创建一个新线程
2. 在新线程中创建一个EventLoop,完成初始化
3. 开启loop
```
class {
    pthread_t m_thread;
    pid_t m_thread_id;
    EventLoop event_loop;
}
```
#### IO线程池 20240221
将所有IO线程进行封装，统一管理

### TCP相关模块设计 20240221
#### TCPBuffer设计 20240221
为什么需要应用层buffer？

```
1. 方便数据处理，进行包的组装和拆解

2. 方便异步发送（接收线程—>缓冲区->发送线程）

3. 提高效率，多个包一次性发送

4. 粘包？在发送和接收过程中，可能出现读取到不完整包的情况，缓冲区可以帮助线程异步完整读取包
```

TCPBuffer实现

```
简单起见使用双指针数组实现

左指针：readIndex指向要读取的位置
右指针：writeIndex指向要写入的位置

提供的方法：
public:
可读、可写、写入、读取、buffer扩容、向右移动读指针、向右移动写指针
private:
为防止内存泄漏, 提供了将可读内容移动到buffer最左端的方法

成员变量:
m_size, m_buffer, m_read_idx, m_write_idx
```

#### TCPAcceptor设计 20240222
将socket通信的主要流程进行简单的封装：

socket => bind => listen => accept

使用了自己封装的net_addr类来代替linux系统的sockaddr, 以自定义更多的功能, 并将其作为TCPAcceptor的地址类

