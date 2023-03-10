#include "chatserver.hpp"
#include "json.hpp"
#include "chatservice.hpp"
#include <functional>
#include <string>
using namespace std;
using namespace placeholders;
using json = nlohmann::json;

//3、初始化聊天服务器对象
ChatServer::ChatServer(EventLoop *loop, const InetAddress& listenAddr, const string& nameArg)
                        :_server(loop, listenAddr, nameArg)
                        ,_loop(loop)
{
    //注册用户连接的创建和断开事件的回调
    _server.setConnectionCallback(std::bind(&ChatServer::onConnection, this, _1));

    //注册用户读写事件的回调
    _server.setMessageCallback(std::bind(&ChatServer::onMessage, this, _1, _2, _3));    

    //设置服务器线程数量 1个I/O线程，3个工作线程
    _server.setThreadNum(4);
}

//启动服务，开启事件循环
void ChatServer::start()
{
    _server.start();
}

//上报连接相关信息的回调函数:参数为连接信息
void ChatServer::onConnection(const TcpConnectionPtr &conn)
{
    if (!conn->connected()) //客户端断开连接，释放连接资源 moduo库会打印相应日志
    {
        ChatService::instance()->clientCloseException(conn); //处理客户端异常关闭
        conn->shutdown();
    }

}

//网络模块与业务模块解耦：不直接调用相应方法，业务发生变化此处代码也不需要改动
//上报读写事件相关信息的回调函数:参数分别为连接、缓冲区、接收到数据的事件信息
void ChatServer::onMessage(const TcpConnectionPtr &conn, Buffer *buffer, Timestamp time)
{
    string buf = buffer->retrieveAllAsString(); //将buffer缓冲区收到的数据存入字符串
    json js = json::parse(buf); //数据反序列化

    //完全解耦网络模块与业务模块代码：通过js读出的js["msgid"] =》 获取业务处理器handler =》 conn js time
    auto msgHandler = ChatService::instance()->getHandler(js["msgid"].get<int>()); //获取msgid相应的事件处理器
    msgHandler(conn, js, time); //回调消息绑定好的事件处理器，执行业务处理
}