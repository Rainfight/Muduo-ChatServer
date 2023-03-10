#ifndef CHATSERVICE_H
#define CHARSERVICE_H

#include "json.hpp"
#include <unordered_map>
#include <functional>
#include <muduo/net/TcpConnection.h>
#include <mutex>
#include "model/groupmodel.hpp"
#include "model/usermodel.hpp"
#include "model/friendmodel.hpp"
#include "model/offlinemessagemodel.hpp"
#include "redis.hpp"
using namespace std;
using namespace muduo;
using namespace muduo::net;
using json = nlohmann::json;

//处理消息事件回调方法类型
using MsgHandler = std::function<void(const TcpConnectionPtr &conn, json &js, Timestamp)>;

//聊天服务器业务类,设计为单例模式：给msgid映射事件回调(一个消息id映射一个事件处理)
class ChatService
{
public:
    //获取单例对象的接口函数
    static ChatService* instance();

    //获取消息msgid对应的处理器
    MsgHandler getHandler(int msgid);

    //处理登录业务
    void login(const TcpConnectionPtr &conn, json &js, Timestamp time);

    //处理注销业务
    void loginOut(const TcpConnectionPtr &conn, json &js, Timestamp time);

    //处理注册业务
    void reg(const TcpConnectionPtr &conn, json &js, Timestamp time);

    //处理一对一聊天业务
    void oneChat(const TcpConnectionPtr &conn, json &js, Timestamp time);

    //添加好友业务
    void addFriend(const TcpConnectionPtr &conn, json &js, Timestamp time);

    //创建群组业务
    void createGroup(const TcpConnectionPtr &conn, json &js, Timestamp time);

    //加入群组业务
    void addGroup(const TcpConnectionPtr &conn, json &js, Timestamp time);

    //群组聊天业务
    void groupChat(const TcpConnectionPtr &conn, json &js, Timestamp time);

    //处理客户端异常退出
    void clientCloseException(const TcpConnectionPtr &conn);

    //处理服务器异常退出:业务重置
    void reset();

    // 从redis消息队列中获取订阅的消息
    void handleRedisSubscribeMessage(int userid, string msg);

private:
    ChatService();
    unordered_map<int, MsgHandler> _msgHandlerMap; //消息处理器map表 每一个msgid对应一个业务处理方法
    unordered_map<int, TcpConnectionPtr> _userConnMap; //存储在线用户的通信连接
    mutex _connMutex; //互斥锁，保证线程安全
    UserModel _userModel; //用户表的数据操作类对象
    OfflineMsgModel _offlineMsgModel; //离线消息表的数据操作类对象
    FriendModel _friendModel; //好友表的数据操作类对象
    GroupModel _groupModel; //群组相关的数据操作类对象

    Redis _redis; //redis操作对象
};

#endif