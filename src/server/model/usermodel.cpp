#include "usermodel.hpp"
#include "db.h"
#include <iostream>
using namespace std;

//增加新用户
bool UserModel::insert(User &user)
{
    //1、组装SQL语句
    char sql[1024] = {0};
    sprintf(sql, "insert into user(name, password, state) values('%s', '%s', '%s')", 
                user.getName().c_str(), user.getPwd().c_str(), user.getState().c_str());
    
    //2、发送SQL语句，进行处理
    MySQL mysql;
    if (mysql.connect())
    {
        if (mysql.update(sql))
        {
            //id为自增键，设置回去user对象添加新生成的用户id
            user.setId(mysql_insert_id(mysql.getConnection()));
            return true;
        }
    }

    return false;
}

//根据用户id查询用户信息
User UserModel::query(int id)
{
    //1、组装SQL语句
    char sql[1024] = {0};
    sprintf(sql, "select * from user where id = %d", id);

    //2、发送SQL语句，进行处理
    MySQL mysql;
    if (mysql.connect())
    {
        MYSQL_RES *res = mysql.query(sql); //查询id对应的数据
        if (res != nullptr) //查询成功
        {
            MYSQL_ROW row = mysql_fetch_row(res); //获取行数据
            if (row != nullptr)
            {
                User user; 
                user.setId(atoi(row[0]));
                user.setName(row[1]);
                user.setPwd(row[2]);
                user.setState(row[3]);

                mysql_free_result(res); //释放res动态开辟的资源
                return user; //返回user对应的信息
            }
        }
    }

    return User(); //未找到,返回默认的user对象
}

//更新用户状态信息
bool UserModel::updateState(User user)
{
    //1、组装SQL语句
    char sql[1024] = {0};
    sprintf(sql, "update user set state = '%s' where id = %d", user.getState().c_str(), user.getId());

    //2、发送SQL语句，进行相应处理
    MySQL mysql;
    if (mysql.connect())
    {
        if (mysql.update(sql))
        {
            return true;
        } 
    }
    return false;
}

//重置用户的状态信息
void UserModel::resetState()
{
    //1、组装SQL语句
    char sql[1024] = "update user set state = 'offline' where state = 'online'";

    //2、发送SQL语句，进行相应处理
    MySQL mysql;
    if (mysql.connect())
    {
        mysql.update(sql);
    }
}