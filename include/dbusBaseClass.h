
#ifndef DBUS_BASE_CLASS_h_
#define DBUS_BASE_CLASS_h_

#include <string>
#include <list>

/******************************************************************
 * 作者：韩健
 * 参考官方网站：   https://dbus.freedesktop.org/doc/api/html/index.html
 * 个人github网址： https://github.com/JanHanw
 * 日期：20210905
 * 依赖于C的API的dbus程序，用于进程间通讯
 ******************************************************************/
/*******************/

#define WRITE_LOG(pLog, pInfo, level) \
    cout << pInfo << endl;


struct DBusParam
{
    // msgType服务端才使用，客户端不使用
    unsigned msgType;                       // DBUS_MESSAGE_TYPE_METHOD_CALL,或者DBUS_MESSAGE_TYPE_SIGNAL，其他类型不使用
    // 服务端的连接名称，只有客户端使用
    std::string dbusName;

    std::string funcSigName;                // 方法或者信号名称
    std::string interName;                  // 匹配规则，interface名
    std::list<int64_t> numParam;            // 数字参数
    std::list<std::string> strParam;        // 字符参数
};

struct StoredInfo;
// dbus服务端的基类
class DBusBaseServer
{
public:
    DBusBaseServer();
    // reqName: 连接（注册）名; objNmae interface名;不能为空
    bool DbusInit(const std::string& reqName, const std::string& rule);
    void Stop();
    // 子类重写此方法
    virtual void Run(DBusParam& recvInfo);
protected:
    virtual ~DBusBaseServer();

private:
    // 服务监听线程
    static void* DbusSrvListen(void *arg);
private:
    StoredInfo *pStored;
};


// dbus客户端的基类
class DBusBaseClien
{
public:
    DBusBaseClien();
    virtual ~DBusBaseClien();

public:
    bool DbusSendMessage(DBusParam& sendInfo);

private:
    bool SendSignal(DBusParam& sendInfo);
};



#endif