#ifndef GROUPUSER_H
#define GROUPUSER_H

#include "user.hpp"

//GroupUser群组员表的映射类：映射表的相应字段
class GroupUser : public User
{
public:
    void setRole(string role) {this->role = role;}
    string getRole() {return this->role;}
private:
    string role; //组内角色
};

#endif