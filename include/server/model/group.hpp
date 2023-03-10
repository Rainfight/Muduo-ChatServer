#ifndef GROUP_H
#define GROUP_H

#include "groupuser.hpp"
#include <string>
#include <vector>
using namespace std;

#endif

//Group群组表的映射类：映射表的相应字段
class Group
{
public:
    Group(int id = -1, string name = "", string desc = "")
    {
        this->id = id;
        this->name = name;
        this->desc = desc;
    }

    void setId(int id) {this->id = id;}
    void setName(string name) {this->name = name;}
    void setDesc(string desc) {this->desc = desc;}

    int getId() {return this->id;}
    string getName() {return this->name;}
    string getDesc() {return this->desc;}
    vector<GroupUser> &getUsers() {return this->users;}

private:
    int id; //群组id
    string name; //群组名称
    string desc; //群组功能描述
    vector<GroupUser> users; //存储组成员
};