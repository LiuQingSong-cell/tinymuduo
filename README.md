# tinymuduo

## 项目简介
本项目对muduo网络库的核心部分用c++11标准进行了重写，不再需要安装boost库。

下图是muduo库的整体框架：

![muduo网络库整体框架](https://img-blog.csdnimg.cn/aa027fa915cc446d8301bdbd4846ba4c.png?x-oss-process=image/watermark,type_d3F5LXplbmhlaQ,shadow_50,text_Q1NETiBA5oiR5Zyo5Zyw6ZOB56uZ6YeM5ZCD6Ze45py6,size_14,color_FFFFFF,t_70,g_se,x_16)

muduo网络库使用reactor模式，其中mainReactor只负责监听是否有新的连接到来，对于一个新的连接mainReactor封装一个TcpConnection, 
通过轮询的方式选择一个subReactor, 将这个TcpConnection交由改subReactor进行读写事件的监听，当有读写事件发生就调用设置的回调函数。


## 模块划分
1、辅助模块

**[noncopyable](#jump_1)**

**[CurrentThread](#jump_2)**

**[logger](#jump_3)**

**[InetAddress](#jump_4)**

**[Socket](#jump_5)**

**[Callbacks](#jump_6)**

**[Timestamp](#jump_6)**

2、核心模块

**[Channel](#jump_7)**

**[EpollPoller](#jump_8)**

**[EventLoop](#jump_9)**

**[EventLoopThread和EventThreadPool](#jump_10)**

**[TcpConnection和TcpServer](#jump_11)**


## 模块介绍

1、**noncopyable**<a id="jump_1"></a>

该类主要作用是让所有继承自它的类无法被复制，好处是增加代码的可读性，减少码字。

2、CurrentThread<a id="jump_2"></a>

CurrentThread的作用是通过系统调用 syscall(SYS_gettid) 返回当前线程的tid，用于对象判断是否在同一个线程。

3、logger<a id="jump_3"></a>

logger设置了日志的等级，并提供了打印不同级别日志的宏定义。

4、InetAddress<a id="jump_4"></a>
InetAddress是对**sockaddr_in**进行了封装，方便我们设置IP地址和端口号

5、Socket<a id="jump_5"></a>
Socket封装了socket相关的操作，如设置相关socket属性

6、Callbacks和Timestamp<a id="jump_6"></a>
Callbacks typedef了回调函数的类型, Timestamp 实现了时间戳功能

7、Channel<a id="jump_7"></a>
Channel类封装了一个sockfd和这个sockfd感兴趣的事件，以及感兴趣事件的回调方法，当事件发生时会从EpollPoller上返回活跃的Channel列表给EventLoop，
此时会在EventLoop中执行设置的回调方法。

8、EpollPoller<a id="jump_8"></a>
该类是对epoll的封装， 负责监听事件的发生并从poll接口向EventLoop返回所有的有事件发生的Channel

9、EventLoop<a id="jump_9"></a>
EventLoop含有多个Channel和一个EpollPoller，开始事件循环后可以从poller中获得有事件发生的所有channel，并调用回调函数

10、EventLoopThread和EventThreadPool<a id="jump_10"></a>
EventLoopThread是对thread的封装，里面有一个threadFunc的接口作为线程的任务函数，负责生成一个EventLoop, 并执行相应的事件循环；
EventLoopthreadPoll是EventLoopThread的集合，通过设置线程数可以指定线程池中的EventLoop的个数，这里的所有EventLoop都是上图中的subReactor，
该类也向TcpServer提供了轮询算法选择subReactor的接口。

11、TcpConnection和TcpServer<a id="jump_11"></a>
TcpConnection类是mainReactor和subReactor沟通的桥梁，当有新的连接建立，mainReactor就会把新的连接封装成一个TcpConnection，然后通过轮询算法
选择一个subReactor，然后分发给subReactor，随之subReactor开始监听新连接的事件发生；

TcpServer是提供给用户使用的类，用户通过设置线程数以及指定相应的回调方法即可开始处理业务逻辑。 
这种设计让用户只用关心如何设计业务逻辑，而不用关心底层的网络行为。