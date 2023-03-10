#include "chatservice.hpp"
#include "public.hpp"
#include <muduo/base/Logging.h>
#include <vector>
using namespace std;
using namespace muduo;

// 获取单例对象的接口函数
ChatService *ChatService::instance()
{
    static ChatService service;
    return &service;
}

// 构造函数：注册消息以及对应的回调操作 实现网络模块与业务模块解耦的核心
ChatService::ChatService()
{
    _msgHandlerMap.insert({LOGIN_MSG, std::bind(&ChatService::login, this, _1, _2, _3)});
    _msgHandlerMap.insert({LOGIN_OUT_MSG, std::bind(&ChatService::loginOut, this, _1, _2, _3)});
    _msgHandlerMap.insert({REG_MSG, std::bind(&ChatService::reg, this, _1, _2, _3)});
    _msgHandlerMap.insert({ONE_CHAT_MSG, std::bind(&ChatService::oneChat, this, _1, _2, _3)});
    _msgHandlerMap.insert({ADD_FRIEND_MSG, std::bind(&ChatService::addFriend, this, _1, _2, _3)});

    _msgHandlerMap.insert({CREATE_GROUP_MSG, std::bind(&ChatService::createGroup, this, _1, _2, _3)});
    _msgHandlerMap.insert({ADD_GROUP_MSG, std::bind(&ChatService::addGroup, this, _1, _2, _3)});
    _msgHandlerMap.insert({GROUP_CHAT_MSG, std::bind(&ChatService::groupChat, this, _1, _2, _3)});

    //连接redis服务器
    if (_redis.connect())
    {
        //设置上报消息的回调
        _redis.init_notify_handler(std::bind(&ChatService::handleRedisSubscribeMessage, this, _1, _2));
    }
}

// 获取消息msgid对应的处理器
MsgHandler ChatService::getHandler(int msgid)
{
    auto it = _msgHandlerMap.find(msgid);
    if (it == _msgHandlerMap.end())
    {
        return [=](const TcpConnectionPtr &conn, json &js, Timestamp)
        {
            LOG_ERROR << "msgid:" << msgid << " can not find handler!";
        }; // msgid没有对应处理器，打印日志，返回一个默认处理器，空操作
    }
    else
    {
        return _msgHandlerMap[msgid];
    }
}

// 处理登录业务 需要处理id、password字段
void ChatService::login(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    // 1、获取id、password字段
    int id = js["id"].get<int>();
    string password = js["password"];

    // 传入用户id，返回相应数据
    User user = _userModel.query(id);
    if (user.getId() == id && user.getPwd() == password) // 登录成功
    {
        if (user.getState() == "online") // 用户已登录，不允许重复登录
        {
            json response;
            response["msgid"] = LOGIN_MSG_ACK;
            response["errno"] = 2; // 重复登录
            response["errmsg"] = "this account is using, input another!";
            conn->send(response.dump());
        }
        else // 用户未登录，此时登录成功
        {
            {
                lock_guard<mutex> lock(_connMutex);
                _userConnMap.insert({id, conn}); // 登录成功记录用户连接信息
            }

            //id用户登录成功后，向redis订阅channel(id)通道的事件
            _redis.subscribe(id);

            user.setState("online");
            _userModel.updateState(user); // 更新用户状态信息

            json response;
            response["msgid"] = LOGIN_MSG_ACK;
            response["errno"] = 0;
            response["id"] = user.getId();
            response["name"] = user.getName();

            //登录成功，查询该用户是否有离线消息
            vector<string> vec = _offlineMsgModel.query(id);
            if (!vec.empty())
            {
                response["offlinemsg"] = vec; //查询到离线消息，发送给用户
                _offlineMsgModel.remove(id); //发送离线消息完成，将本次的离线消息删除掉
            }
            
            //登录成功，查询该用户的好友信息并返回
            vector<User> userVec = _friendModel.query(id);
            if (!userVec.empty())
            {
                vector<string> vec2;
                for (User &user : userVec)
                {
                    json js;
                    js["id"] = user.getId();
                    js["name"] = user.getName();
                    js["state"] = user.getState();
                    vec2.push_back(js.dump());
                }
                response["friends"] = vec2;
            }

            // 登录成功，查询用户的群组信息并返回
            vector<Group> groupuserVec = _groupModel.queryGroups(id);
            if (!groupuserVec.empty())
            {
                // group:[{groupid:[xxx, xxx, xxx, xxx]}]
                vector<string> groupV;
                for (Group &group : groupuserVec)
                {
                    json grpjson;
                    grpjson["id"] = group.getId();
                    grpjson["groupname"] = group.getName();
                    grpjson["groupdesc"] = group.getDesc();
                    vector<string> userV;
                    for (GroupUser &user : group.getUsers())
                    {
                        json js;
                        js["id"] = user.getId();
                        js["name"] = user.getName();
                        js["state"] = user.getState();
                        js["role"] = user.getRole();
                        userV.push_back(js.dump());
                    }
                    grpjson["users"] = userV;
                    groupV.push_back(grpjson.dump());
                }

                response["groups"] = groupV;
            }

            conn->send(response.dump());
        }
    }
    else // 该用户不存在或密码输入错误，登录失败
    {
        json response;
        response["msgid"] = LOGIN_MSG_ACK;
        response["errno"] = 1;
        response["errmsg"] = "id or password is invalid!";
        conn->send(response.dump());
    }
}

// 处理注册业务 需要处理name、password字段
void ChatService::reg(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    // 1、获取name、password字段
    string name = js["name"];
    string password = js["password"];

    // 2、创建User对象，进行注册
    User user;
    user.setName(name);
    user.setPwd(password);
    bool state = _userModel.insert(user);
    if (state) // 注册成功
    {
        json response;
        response["msgid"] = REG_MSG_ACK; // 注册响应消息
        response["errno"] = 0;           // 错误标识 0:成功 1:失败
        response["id"] = user.getId();
        conn->send(response.dump());
    }
    else // 注册失败
    {
        json response;
        response["msgid"] = REG_MSG_ACK;
        response["errno"] = 1;
        conn->send(response.dump());
    }
}

//处理一对一聊天业务
void ChatService::oneChat(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    //1、先获取目的id
    int toid = js["toid"].get<int>();

    {
        lock_guard<mutex> lock(_connMutex);
        
        auto it = _userConnMap.find(toid);
        if (it != _userConnMap.end()) //2、目的id在线，进行消息转发，服务器将源id发送的消息中转给目的id
        {
            it->second->send(js.dump());
            return;
        }
    }

    //查询toid是否在线
    User user = _userModel.query(toid);
    if (user.getState() == "online")
    {
        _redis.publish(toid, js.dump());
        return;
    }

    //目的id不在线，将消息存储到离线消息里
    _offlineMsgModel.insert(toid, js.dump());
}

//添加好友业务
void ChatService::addFriend(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    //1、获取当前用户id、要添加好友id
    int userid = js["id"].get<int>();
    int friendid = js["friendid"].get<int>();

    //2、数据库中存储要添加好友的信息
    _friendModel.insert(userid, friendid);
}

//创建群组业务
void ChatService::createGroup(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    //1、获取创建群的用户id、群名称、群功能
    int userid = js["id"].get<int>();
    string name = js["groupname"];
    string desc = js["groupdesc"];

    //2、存储新创建的群组信息
    Group group(-1, name, desc);
    if (_groupModel.createGroup(group))
    {
        _groupModel.addGroup(userid, group.getId(), "creator"); //存储群组创建人信息
    }
}

//加入群组业务
void ChatService::addGroup(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    //1、获取要加入群用户的id、要加入的群组id
    int userid = js["id"].get<int>();
    int groupid = js["groupid"].get<int>();

    //2、将用户加入群组
    _groupModel.addGroup(userid, groupid, "normal");
}

//群组聊天业务
void ChatService::groupChat(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    //1、获取要发送消息的用户id、要发送的群组id
    int userid = js["id"].get<int>();
    int groupid = js["groupid"].get<int>();

    //2、查询该群组其它用户id
    vector<int> useridVec = _groupModel.queryGroupUsers(userid, groupid);

    //3、进行用户查找
    lock_guard<mutex> lock(_connMutex);
    for (int id : useridVec)
    {
        auto it = _userConnMap.find(id);
        if (it != _userConnMap.end()) //用户在线，转发群消息
        {
            it->second->send(js.dump());
        }
        else //用户不在线，存储离线消息 或 在其它服务器上登录的
        {
            //查询toid是否在线
            User user = _userModel.query(id);
            if (user.getState() == "online") //在其它服务器上登录的
            {
                _redis.publish(id, js.dump());
            }
            else //存储离线消息
            {
                _offlineMsgModel.insert(id, js.dump());
            }
        }
    }
}

//处理注销业务
void ChatService::loginOut(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    //1、获取要注销用户的id，删除对应连接
    int userid = js["id"].get<int>();

    {
        lock_guard<mutex> lock(_connMutex);
        auto it = _userConnMap.find(userid);
        if (it != _userConnMap.end())
        {
            _userConnMap.erase(it);
        }
    }

    //用户注销下线，在redis中取消订阅通道
    _redis.unsubscrible(userid);

    //2、更新用户状态信息
    User user(userid, "", "", "offline");
    _userModel.updateState(user);
}

//处理客户端异常退出
void ChatService::clientCloseException(const TcpConnectionPtr &conn)
{
    User user;
    {
        lock_guard<mutex> lock(_connMutex);

        // 1、从map表删除用户的连接信息
        for (auto it = _userConnMap.begin(); it != _userConnMap.end(); ++it)
        {
            if (it->second == conn)
            {
                user.setId(it->first);
                _userConnMap.erase(it);
                break;
            }
        }
    }

    //用户注销下线，在redis中取消订阅通道
    _redis.unsubscrible(user.getId());

    // 2、更新用户的状态信息
    if (user.getId() != -1)
    {
        user.setState("offline");
        _userModel.updateState(user);
    }
}

//处理服务器异常退出:业务重置
void ChatService::reset()
{
    //将online状态的用户设置为offline
    _userModel.resetState();
}

// 从redis消息队列中获取订阅的消息：通道号 + 消息
void ChatService::handleRedisSubscribeMessage(int userid, string msg)
{
    lock_guard<mutex> lock(_connMutex);
    auto it = _userConnMap.find(userid);
    if (it != _userConnMap.end())
    {
        it->second->send(msg);
        return;
    }

    // 存储该用户的离线消息：在从通道取消息时，用户下线则发送离线消息
    _offlineMsgModel.insert(userid, msg);
}