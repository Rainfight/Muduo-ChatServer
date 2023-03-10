#include "groupmodel.hpp"
#include "db.h"

 //创建群组：在数据库层面即给群组allgroup表中添加一组信息，不能重复添加组
bool GroupModel::createGroup(Group &group)
{
    //1、组装SQL语句
    char sql[1024] = {0};
    sprintf(sql, "insert into allgroup(groupname, groupdesc) values('%s', '%s')", group.getName().c_str(), group.getDesc().c_str());

    //2、发送SQL语句，进行相应处理
    MySQL mysql;
    if (mysql.connect())
    {
        if (mysql.update(sql))
        {
            group.setId(mysql_insert_id(mysql.getConnection()));
            return true;
        }
    }

    return false;    
}

//加入群组：即给群组员groupuser表添加一组信息
void GroupModel::addGroup(int userid, int groupid, string role)
{
    //1、组装SQL语句
    char sql[1024] = {0};
    sprintf(sql, "insert into groupuser values(%d, %d, '%s')", groupid, userid, role.c_str());

    //2、发送SQL语句，进行相应处理
    MySQL mysql;
    if (mysql.connect())
    {
        mysql.update(sql);
    }
}

//查询用户所在群组信息：群信息以及组员信息
vector<Group> GroupModel::queryGroups(int userid)
{
    /*
    1、先根据userid在groupuser表中查询出该用户所属的群组详细信息
    2、再根据群组信息，查询属于该群组的所有用户的userid，并且和user表进行多表联合查询出用户的详细信息
    */

    //1、组装SQL语句
    char sql[1024] = {0};
    sprintf(sql, "select a.id,a.groupname,a.groupdesc from allgroup a inner join \
            groupuser b on a.id = b.groupid where b.userid=%d", userid);
    
    //2、发送SQL语句，进行相应处理
    vector<Group> groupVec;
    MySQL mysql;
    if (mysql.connect())
    {
        MYSQL_RES *res = mysql.query(sql);
        if (res != nullptr)
        {
            MYSQL_ROW row;
            //查出userid所有的群信息
            while ((row = mysql_fetch_row(res)) != nullptr)
            {
                Group group;
                group.setId(atoi(row[0]));
                group.setName(row[1]);
                group.setDesc(row[2]);
                groupVec.push_back(group);
            }
            mysql_free_result(res);
        }
    }

    //查询群组的用户信息
    for (Group &group : groupVec)
    {
        sprintf(sql, "select a.id,a.name,a.state,b.grouprole from user a \
            inner join groupuser b on b.userid = a.id where b.groupid=%d", group.getId());
        
        MYSQL_RES *res = mysql.query(sql);
        if (res != nullptr)
        {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res)) != nullptr)
            {
                GroupUser user;
                user.setId(atoi(row[0]));
                user.setName(row[1]);
                user.setState(row[2]);
                user.setRole(row[3]);
                group.getUsers().push_back(user);
            }
            mysql_free_result(res);
        }
    }
    return groupVec;
}

//根据指定的groupid查询群组用户id列表，除userid自己，给群组其它成员群发消息
vector<int> GroupModel::queryGroupUsers(int userid, int groupid)
{
    //1、组装SQL语句
    char sql[1024] = {0};
    sprintf(sql, "select userid from groupuser where groupid = %d and userid != %d", groupid, userid);

    //2、发送SQL语句，进行相应处理
    vector<int> idVec;
    MySQL mysql;
    if (mysql.connect())
    {
        MYSQL_RES *res = mysql.query(sql);
        if (res != nullptr)
        {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res)) != nullptr)
            {
                idVec.push_back(atoi(row[0]));
            }
            mysql_free_result(res);
        }
    }

    return idVec;
}