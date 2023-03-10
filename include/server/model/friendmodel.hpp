#ifndef FRIENDMODEL_H
#define FRIENDMODEL_H
#include "user.hpp"
#include <vector>
using namespace std;

//Friend用户表的数据操作类：针对表的增删改查
class FriendModel
{
public:
    //添加好友关系
    void insert(int userid, int friendid);

    //返回用户好友列表:返回用户好友id、名称、登录状态信息
    vector<User> query(int userid);
};

#endif