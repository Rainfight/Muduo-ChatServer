#ifndef DB_H
#define DB_H

#include <mysql/mysql.h>
#include <string>
using namespace std;

//数据库操作类型
class MySQL
{
public:
    //初始化连接:开辟存储连接的资源空间
    MySQL();
    
    //释放连接:释放存储连接的资源空间
    ~MySQL();

    //连接数据库
    bool connect();

    //更新操作
    bool update(string sql);

    //查询操作
    MYSQL_RES* query(string sql);

    //获取连接
    MYSQL* getConnection();
private:
    MYSQL *_conn;  //与MySQL Server的一条连接
};

#endif