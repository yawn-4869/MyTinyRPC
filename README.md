### 日志模块开发
#### 同步日志开发（输出到终端） 20240111-20240115
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

#### 更新异步日志 20240223
添加异步日志类AsyncLogger，输出到文件

实现思路：
```
设置异步日志定时任务，每隔一定时间读取一次缓冲区内的数据

读取完毕后启动额外的日志线程来输出到指定文件
```

修复了bug：异步日志一直阻塞的问题

1. 自定义的Mutex类locker.h修改
```
原来的：
void unlock() {
    if(m_is_lock) {
        m_mutex.unlock();
        m_is_lock = false;
    }
}
修改后：
void unlock() {
    if(m_is_lock) {
        m_mutex.unlock();
    }
}
```

2. 定时器错误
```
原来的onTimer函数执行任务流程有错误：
while(it != m_pending_events.end()) {
    if(it->first > now) {
        break;
    }
    if(!it->second->isCancel()) {
        tmp_events.push_back(it->second);
        tmp_tasks.push_back(std::make_pair(it->first, it->second->getCallback()));
    }
    it++;
}
在需要执行的任务队列tmp_tasks中，存放的应该是到期的时间, 而不是it->first, 但好像没什么问题？
经测试没有问题，因此只是自定义互斥锁类的问题
while(it != m_pending_events.end()) {
    if(it->first > now) {
        break;
    }
    if(!it->second->isCancel()) {
        tmp_events.push_back(it->second);
        tmp_tasks.push_back(std::make_pair(it->second->getArriveTime(), it->second->getCallback()));
    }
    it++;
}
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
### IO事件 ###
使用FdEvent来对发生的epoll事件进行封装, 包括EPOLL_IN和EPOLL_OUT
```
提供的公用方法: 设置未阻塞，绑定事件与函数，绑定函数的返回，获取fd
```

为了近一步方便管理fdEvent, 设置了FdEventPool, 设计思路与IOThreadPool类似

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

#### TCPServer设计 20240226
使用主从Reactor的架构

```
* 主线程mainReactor: 通过epoll监听listenfd的可读事件, 当有可读事件发生时, 调用accept函数获取clientfd, 随机抽取一个subReactor,
                     将clientfd的读写事件注册到这个subReactor的epoll上
                     mainReactor只负责建立连接事件，不进行业务处理，也不关心套接字的IO事件

* 从线程subReactor: 每个subReactor都由一个线程来运行, 注册clientfd的读写事件, 当IO事件发生后, 需要进行对应的业务处理
```

#### TCPConnection设计 20240306
主要业务流程：read -> excute -> write
```
read: 读取客户端发来的数据, 存入到读入缓冲区inbuffer, 

excute: 从inbuffer中读取数据, 解码组装为rpc请求
解析rpc请求, 执行业务逻辑, 获取rpc响应, 
获取到的rpc请求将会在编码后存入到写出缓冲区outBuffer, 准备向客户端发送

write: 从outBuffer将数据解码生成rpc响应, 在对应的fd可写的情况下, 将rpc响应返回给客户端
```
#### TcpClient设计 20240308
主要业务流程：connect -> write -> read
```
connect: 采用非阻塞方式连接对端机器(异步)
对于非阻塞方式的connect, 无论是否连接成功都立即返回: 
1. 返回0: 连接成功
2. 返回-1, errno = EINPROGRESS: 连接正在建立, 可以在epoll中添加监听可写事件
等可写事件就绪, 调用getsockopt获取fd上的错误, 若返回0, 则连接成功
3. 其他errno：不做处理, 直接报错

write: connect成功后, 把message对象写入到Connection中buffer中,监听可写事件
对于客户端的write函数，将请求与对应的函数写入到TcpConnection中的m_dones中后，再由TcpConnection的onWrite函数进行编码
编码完成后，push进buffer

read: 
```

### Rpc模块设计 20240322
#### RpcDecoder

自定义了一个tinyrpc的协议格式, 便于对请求进行分割(请求开始和结束的界限)、匹配对应的请求和响应、定位错误信息

主要是为了解决粘包问题
```
虽然我们可以通过protobuf将请求进行序列化后发送序列化后的结果

但是由于Tcp是字节流的方式传输, 没有包的概念, 因此protobuf序列化后的结果只是一串没有意义的字符流

因此需要实现RpcDecoder
```

TinyPB协议

```
开始符: 固定, 0x02 char
包长度: 整包字节数, 包括开始和结束符 int32
MsgID长度: length of MsgID int32
MsgID: rpc请求的唯一标识, 请求和响应的MsgIDy应当一致 string
方法名长度: length of method name int32
方法名: rpc方法的完整名 string
错误码: 若rpc调用发生系统异常, 设置错误码, 正常情况下为0 int32
错误信息长度: length of error msg, 正常情况为0 int32
错误信息: rpc调用异常的详细错误信息 string
数据: protobuf序列化后的实际数据(字符串保存) string
校验和: 对整包进行校验, 用于防篡改, 校验算法待定
结束码: 固定, 0x03
```

由于需要进行数据的序列化和反序列化, 因此将协议通过struct存储
使用网络字节序(大端存储)

#### RpcDispatcher


#### RpcController
功能介绍: 

继承自google::protobuf::RpcController类

在rpc调用过程中可以通过controller获取rpc调用的某些信息, 方便进行错误处理和debug