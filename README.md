# tinymuduo

## 项目简介
本项目对muduo网络库的核心部分用c++11标准进行了重写，不再需要安装boost库。

下图是muduo库的整体框架：

![muduo网络库整体框架](https://img-blog.csdnimg.cn/aa027fa915cc446d8301bdbd4846ba4c.png?x-oss-process=image/watermark,type_d3F5LXplbmhlaQ,shadow_50,text_Q1NETiBA5oiR5Zyo5Zyw6ZOB56uZ6YeM5ZCD6Ze45py6,size_14,color_FFFFFF,t_70,g_se,x_16)

muduo网络库使用reactor模式，其中mainReactor只负责监听是否有新的连接到来，对于一个新的连接mainReactor封装一个TcpConnection, 
通过轮询的方式选择一个subReactor, 将这个TcpConnection交由改subReactor进行读写事件的监听，当有读写事件发生就调用设置的回调函数。


## 模块划分
1、辅助模块

**noncopyable**

**CurrentThread**

**logger**

**InetAddress**

**Socket**

**Callbacks**

**Timestamp**

2、核心模块

**Channel**

**EpollPoller**

**EventLoop**

**EventThread和EventThreadPool**

**TcpConnection和TcpServer**
