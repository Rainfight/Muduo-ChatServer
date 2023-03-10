#include "friendmodel.hpp"
#include "db.h"

 //添加好友关系
void FriendModel::insert(int userid, int friendid)
{
    //1、组装SQL语句
    char sql[1024] = {0};
    sprintf(sql, "insert into friend values(%d, %d)", userid, friendid);

    //2、发送SQL语句，进行相应处理
    MySQL mysql;
    if (mysql.connect())
    {
        mysql.update(sql);
    }
}

//返回用户好友列表:返回用户好友id、名称、登录状态信息
vector<User> FriendModel::query(int userid)
{
    //1、组装SQL语句:多表联合查询
    char sql[1024] = {0};
    sprintf(sql, "select a.id,a.name,a.state from user a inner join friend b on b.friendid = a.id where b.userid=%d", userid);

    //2、发送SQL语句，进行相应处理
    vector<User> vec;
    MySQL mysql;
    if (mysql.connect())
    {
        MYSQL_RES *res = mysql.query(sql);
        if (res != nullptr)
        {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res)) != nullptr) //将userid好友的详细信息返回
            {
                User user;
                user.setId(atoi(row[0]));
                user.setName(row[1]);
                user.setState(row[2]);
                vec.push_back(user);
            }

            mysql_free_result(res);
        }
    }

    return vec;
}