/*
 * Filename: User.h
 * Author: group DVoter
 */
#ifndef USER_H
#define USER_H

#include <string>

using namespace std;

/*
 * Class Name: User
 * Description:
 */
class User
{
private:
    string IP;
    string name;
    int port;
    bool isLeader;
public:
    /* Constructors */
    User();
    User(string IP, string name, int port);
    User(string IP, string name, int port, bool isLeader);
    /* Getters */
    string getIP();
    string getName();
    bool getIsLeader();
    /* Setters */
    void setIP(string IP);
    void setName(string name);
    void setIsLeader(bool isLeader);
};

#endif /* USER_H */
