#ifndef USERMODEL_H
#define USERMODEL_H
#include "user.hpp"

//User用户表的数据操作类：针对表的增删改查
class UserModel
{
public:
    //增加新用户
    bool insert(User &user);

    //根据用户id查询用户信息
    User query(int id);

    //更新用户状态信息
    bool updateState(User user);

    //重置用户的状态信息
    void resetState();
private:
};

#endif