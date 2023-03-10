#ifndef USER_H
#define USER_H
#include <iostream>
using namespace std;

//User用户表的映射类：映射表的相应字段
class User
{
public:
    //构造函数
    User(int i = -1, string n = "", string pwd = "", string st = "offline")
    {
        this->id = i;
        this->name = n;
        this->password = pwd;
        this->state = st;
    }

    //对外提供公有接口访问私有成员变量
    //设置相应字段
    void setId(int i){this->id = i;}
    void setName(string n) {this->name = n;}
    void setPwd(string pwd) {this->password = pwd;}
    void setState(string st) {this->state = st;}

    //获取相应字段
    int getId() {return this->id;}
    string getName() {return this->name;}
    string getPwd() {return this->password;}
    string getState() {return this->state;}
private:
    int id; //用户id
    string name; //用户名
    string password; //用户密码
    string state; //当前登录状态
};

#endif