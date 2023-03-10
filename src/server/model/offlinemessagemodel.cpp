#include <offlinemessagemodel.hpp>
#include <db.h>

//存储用户的离线消息
void OfflineMsgModel::insert(int userid, string msg)
{
    //1、组装SQL语句
    char sql[1024] = {0};
    sprintf(sql, "insert into offlinemessage values(%d, '%s')", userid, msg.c_str());

    //2、发送SQL语句，进行相应处理
    MySQL mysql;
    if (mysql.connect())
    {
        mysql.update(sql);
    }
}

//删除用户的离线消息
void OfflineMsgModel::remove(int userid)
{
    //1、组装SQL语句
    char sql[1024] = {0};
    sprintf(sql, "delete from offlinemessage where userid=%d", userid);

    //2、发送SQL语句，进行相应处理
    MySQL mysql;
    if (mysql.connect())
    {
        mysql.update(sql);
    }
}

//查询用户的离线消息:离线消息可能有多个
vector<string> OfflineMsgModel::query(int userid)
{
    //1、组装SQL语句
    char sql[1024] = {0};
    sprintf(sql, "select message from offlinemessage where userid = %d", userid);

    //2、发送SQL语句，进行相应处理
    MySQL mysql;
    vector<string> vec; //存储离线消息,离线消息可能有多条

    if (mysql.connect())
    {
        MYSQL_RES *res = mysql.query(sql);
        if (res != nullptr)
        {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res)) != nullptr) //循环查找离线消息
            {
                vec.push_back(row[0]);
            }

            mysql_free_result(res);
        }
    }

    return vec;
}