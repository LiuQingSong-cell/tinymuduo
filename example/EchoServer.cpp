#include <tinymuduo/TcpServer.h>
#include <tinymuduo/EventLoop.h>
#include <tinymuduo/logger.h>
#include <tinymuduo/TcpConnection.h>

#include <iostream>
#include <string>
using namespace std;

class EchoServer
{
    public:
        EchoServer(EventLoop* loop, const InetAddress& listenAddr, string name):
            loop_(loop),
            server_(loop, listenAddr, name)
        {
            server_.setConnectionCallback(
                std::bind(&EchoServer::onConnection, this, std::placeholders::_1)
            );

            server_.setMessageCallback(
                std::bind(&EchoServer::OnMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)
            );
            server_.setThreadNum(2);
        }

        void start()
        {
            server_.start();
        }


    private:
        // 连接发生和断开时的回调函数
        void onConnection(const TcpConnectionPtr& conn)
        {
            LOG_INFO("EchoServe - %s -> %s", conn->peerAddress().toIpPort(),
                conn->localAddress().toIpPort());
        }
        // 有消息发过来时的回调
        void OnMessage(const TcpConnectionPtr& conn, Buffer* buf, Timestamp time)
        {
            string msg(buf->retriveAllAsString());
            LOG_INFO("EchoServer receive data is %s at time %s. Then send back", msg.c_str(), time.toString().c_str());
            conn->send(msg);
        }


    private:
        EventLoop* loop_;
        TcpServer server_;
};


int main()
{
    EventLoop loop;
    InetAddress listenAddr("127.0.0.1", 8000);
    EchoServer echoServer(&loop, listenAddr, "EchoServer");
    echoServer.start();
    loop.loop();
}