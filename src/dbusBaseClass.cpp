#include "dbusBaseClass.h"
#include <dbus/dbus.h>
#include <pthread.h>
#include <unistd.h>
#include <iostream>

using namespace std;

#define LISTEN_SLEEP_TIME 0.5

struct StoredInfo
{
    DBusError m_err;
    DBusConnection *m_pConn;
    DBusMessage *m_msg;
    DBusMessageIter m_args;
    DBusMessage *m_reply;

    string m_reqName;
    string m_objName;
    string m_signalName;
    string m_methodName;

    pthread_t m_pThread;
    bool m_isRuning;
    DBusParam m_recvInfo;

    // 获取连接
    bool BusGet();
    // 请求连接名
    bool BusRequestName(const string &reqName);
    // 添加匹配规则
    bool BusAddMatch(const string &objNmae);

    void Run(DBusBaseServer* pSrv);
};

bool StoredInfo::BusGet()
{
    dbus_error_init(&m_err);
    m_pConn = dbus_bus_get(DBUS_BUS_SESSION, &m_err);
    if (dbus_error_is_set(&m_err))
    {
        WRITE_LOG(pLog, "DBus连接总线失败，可能DBus总线未启动！", ERR_LOG);
        WRITE_LOG(pLog, m_err.message, ERR_LOG);
        dbus_error_free(&m_err);
        return false;
    }
    else if (m_pConn == nullptr)
    { // 连接为空
        WRITE_LOG(pLog, "DBus获得了一个空连接！", ERR_LOG);
        return false;
    }
    return true;
}

bool StoredInfo::BusRequestName(const string &reqName)
{
    // 请求连接名称，如果该名称已被使用，则不连接
    int ret = dbus_bus_request_name(m_pConn, reqName.c_str(), DBUS_NAME_FLAG_DO_NOT_QUEUE, &m_err);
    string strName("Dbus请求连接总线，连接名为：");
    strName += reqName;
    WRITE_LOG(pLog, strName.c_str(), DEBUG_LOG);

    if (dbus_error_is_set(&m_err))
    {
        WRITE_LOG(pLog, "DBus注册总线失败！", ERR_LOG);
        WRITE_LOG(pLog, m_err.message, ERR_LOG);
        dbus_error_free(&m_err);
        return false;
    }
    else if (ret != DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER)
    { // 当前不是连接所有者
        WRITE_LOG(pLog, "DBus注册总线失败，未获得当前请求名的所有权！", ERR_LOG);
        return false;
    }

    m_reqName = reqName;
    return true; // 请求连接成功。
}

//一个示例是:
// type ='signal'，sender ='org.freedesktop.DBus'，interface ='org.freedesktop.DBus'，member ='Foo'，
//    path ='/ bar / foo'，destination ='：452345.34 '”
//您可以匹配的可能的键是类型，发件人，接口，成员，路径，目标和编号键，以匹配消息args（键为'arg0'，'arg1'等）。从规则中省略键表示通配符匹配
bool StoredInfo::BusAddMatch(const string &objNmae)
{
    string rule("path='");
    rule += objNmae;
    rule += "'";

    string strName = "Dbus添加匹配规则： ";
    strName += rule;
    WRITE_LOG(pLog, strName.c_str(), DEBUG_LOG);

    dbus_bus_add_match(m_pConn, rule.c_str(), &m_err);
    dbus_connection_flush(m_pConn);
    if (dbus_error_is_set(&m_err))
    {
        strName = "Dbus添加匹配规则失败 ：";
        strName += m_err.message;
        WRITE_LOG(pLog, strName.c_str(), ERR_LOG);
        dbus_error_free(&m_err);
        return false;
    }
    return true;
}

void StoredInfo::Run(DBusBaseServer* pSrv)
{
    WRITE_LOG(pLog, "Dbus服务器开始运行---------------！", INFO_LOG);
    m_isRuning = true;
    const char* objName = nullptr;
    const char* memberName = nullptr;             // 信号或者方法名
    const char* sendName = nullptr;
    int current_type = DBUS_TYPE_INVALID;
    int argInt64 = 0;
    char* argStr = nullptr;
    string msgInfo;
    // 去除首部的两次确认消息
    dbus_connection_read_write(m_pConn, 0);        
    m_msg = dbus_connection_pop_message(m_pConn);
    dbus_message_unref(m_msg); 
    dbus_connection_read_write(m_pConn, 0);        
    m_msg = dbus_connection_pop_message(m_pConn); 
    dbus_message_unref(m_msg);

    while (true) {
        if (!m_isRuning) {
            WRITE_LOG(pLog, "主程序请求停止运行,Dbus服务器即将停止---------------！", INFO_LOG);
            break;
        }
        dbus_connection_read_write(m_pConn, 0);        // 进入监听循环
        m_msg = dbus_connection_pop_message(m_pConn);  // 从总线上取出消息
        if (m_msg == nullptr) {                     // 没有消息
            sleep(LISTEN_SLEEP_TIME);
            continue;
        }
        WRITE_LOG(pLog, "Dbus服务器接收到一条消息。-------------------------！", INFO_LOG);

        objName = dbus_message_get_path(m_msg);
        memberName = dbus_message_get_member(m_msg);
        if (objName == nullptr || memberName == nullptr) {
            WRITE_LOG(pLog, "Dbus服务器收到与规则不匹配的消息, 不处理！", WARN_LOG);
            dbus_message_unref(m_msg);
            continue;
        }
        m_recvInfo.msgType = dbus_message_get_type(m_msg);

        msgInfo = "消息名：";
        msgInfo += memberName;
        WRITE_LOG(pLog, msgInfo.c_str(), DEBUG_LOG);
        
        if (!dbus_message_iter_init(m_msg, &m_args)) {      // 消息不携带参数
            dbus_message_unref(m_msg);
            continue;
        }
        

        msgInfo = "消息参数：";
        bool argErr = false;
        bool argEnd = false;
        m_recvInfo.numParam.clear();
        m_recvInfo.strParam.clear();

        // 先解数字参数
        while (true) {
            current_type = dbus_message_iter_get_arg_type(&m_args);
            if (current_type != DBUS_TYPE_INT64) break;
            dbus_message_iter_get_basic(&m_args, &argInt64);
            m_recvInfo.numParam.push_back(argInt64);
            if (!dbus_message_iter_next(&m_args)) {
                argEnd = true;
                break;
            }
        }
        if (argErr) {
            dbus_message_unref(m_msg);
            continue;
        }
        // 解字符参数
        if (!argEnd && current_type == DBUS_TYPE_STRING) {
            while (true) {
                dbus_message_iter_get_basic(&m_args, &argStr);
                m_recvInfo.strParam.push_back(argStr);
                if (!dbus_message_iter_next(&m_args)) {         // 没有下一个参数
                    break;
                }
                current_type = dbus_message_iter_get_arg_type(&m_args);
                if (current_type != DBUS_TYPE_STRING) break;
            }
        }
        
        if (argErr) {
            dbus_message_unref(m_msg);
            continue;
        }


        sendName = dbus_message_get_sender(m_msg);
        if (sendName != nullptr) {
            m_recvInfo.dbusName = sendName;
        }
        m_recvInfo.interName = objName;
        m_recvInfo.funcSigName = memberName;
        
        pSrv->Run(m_recvInfo);
    
        dbus_message_unref(m_msg);
    } 
    WRITE_LOG(pLog, "Dbus服务器停止运行---------------！", INFO_LOG);
}




DBusBaseServer::DBusBaseServer() : pStored(nullptr)
{
}

DBusBaseServer::~DBusBaseServer()
{
    if (pStored != nullptr)
    {
        delete pStored;
        pStored = nullptr;
    }
}

bool DBusBaseServer::DbusInit(const string &reqName, const string &objName)
{
    if (pStored == nullptr)
    {
        pStored = new StoredInfo;
        if (pStored == nullptr)
        {
            WRITE_LOG(pLog, "内存不足，初始化失败！", ERR_LOG);
            return false;
        }
    }
    if (!pStored->BusGet() || !pStored->BusRequestName(reqName) || !pStored->BusAddMatch(objName))
        return false;

    int ret = pthread_create(&pStored->m_pThread, nullptr, DBusBaseServer::DbusSrvListen, this);
    if (ret) {
        WRITE_LOG(pLog, "Dbus服务器启动失败，创建线程未成功。", ERR_LOG);
        return false;
    }

    return true;
}

void DBusBaseServer::Stop()
{
    pStored->m_isRuning = false;
    pthread_join(pStored->m_pThread, NULL);     // 等待线程结束
}

void DBusBaseServer::Run(DBusParam& recvInfo)
{
    cout << "DBusBaseServer recv dbus message!" << endl;
}

// 服务监听线程
void *DBusBaseServer::DbusSrvListen(void *arg)
{
    DBusBaseServer* pSrv = (DBusBaseServer*)arg;
    pSrv->pStored->Run(pSrv);
    return nullptr;
}








bool CheckNameFormat(const char* name, const char* path)
{
    string check(name);
    int ret = check.find('.');
    if (ret < 0) {
        check = "检查DBus注册名格式，当前格式不符合DBus规则：";
        check += name;
        WRITE_LOG(pLog, check.c_str(), ERR_LOG);
        return false;
    }
    check[ret] = '0';
    ret = check.find('.');
    if (ret < 0) {
        check = "检查DBus注册名格式，当前格式不符合DBus规则：";
        check += name;
        WRITE_LOG(pLog, check.c_str(), ERR_LOG);
        return false;
    }

    check = path;
    ret = check.find('/');
    if (ret < 0) {
        check = "检查DBus匹配规则格式，当前格式不符合DBus规则：";
        check += path;
        WRITE_LOG(pLog, check.c_str(), ERR_LOG);
        return false;
    }
    check[ret] = '0';
    ret = check.find('/');
    if (ret < 0) {
        check = "检查DBus匹配规则格式，当前格式不符合DBus规则：";
        check += path;
        WRITE_LOG(pLog, check.c_str(), ERR_LOG);
        return false;
    }
    return true;
}




DBusBaseClien::DBusBaseClien()
{

}

DBusBaseClien::~DBusBaseClien()
{
    
}


bool DBusBaseClien::DbusSendMessage(DBusParam& sendInfo)
{

}


bool DBusBaseClien::SendSignal(DBusParam& sendInfo)
{
    if (!CheckNameFormat(sendInfo.dbusName.c_str(), sendInfo.interName.c_str())) {
        return false;
    }
    DBusError err;
    DBusConnection *conn = dbus_bus_get(DBUS_BUS_SESSION, &err);
    if (dbus_error_is_set(&err)) {
        WRITE_LOG(pLog, "DBus连接总线失败，可能DBus总线未启动！", ERR_LOG);
        WRITE_LOG(pLog, err.message, ERR_LOG);
        dbus_error_free(&err);
        return false;
    } else if (nullptr == conn) {
        WRITE_LOG(pLog, "DBus获得了一个空连接！", ERR_LOG);
        return false;
    }

    if (!dbus_bus_name_has_owner(conn, sendInfo.dbusName.c_str(), 0)) {
        WRITE_LOG(pLog, "指定的dbus连接未启动！", ERR_LOG);
        WRITE_LOG(pLog, sendInfo.dbusName.c_str(), WARN_LOG);
        return false;
    }

    DBusMessage* msg = dbus_message_new_signal(sendInfo.interName.c_str(), "dbus.send.signal", sendInfo.funcSigName.c_str());
    if (msg == nullptr) {
        WRITE_LOG(pLog, "建立一个信号失败！无法发送信号！", ERR_LOG);
        return false;
    }
    
    DBusMessageIter args;
    dbus_message_iter_init_append(msg, &args);

    list<int64_t>::iterator it = sendInfo.numParam.begin();
    while (it != sendInfo.numParam.end()) {
        if (!dbus_message_iter_append_basic(&args, DBUS_TYPE_INT64, &(*it))) {
            WRITE_LOG(pLog, "绑定int参数时，内存不足，信号发送失败！", ERR_LOG);
            return false;
        }
    }

    list<string>::iterator it1 = sendInfo.strParam.begin();
    const char* pArg = nullptr;
    while (it1 != sendInfo.strParam.end()) {
        pArg = (*it1).c_str();
        if (!dbus_message_iter_append_basic(&args, DBUS_TYPE_STRING, &pArg)) {
            WRITE_LOG(pLog, "绑定int参数时，内存不足，信号发送失败！", ERR_LOG);
            return false;
        }
    }

    dbus_uint32_t serial = 0;
    if (!dbus_connection_send(conn, msg, &serial)) {
        WRITE_LOG(pLog, "发送消息失败！内存不足...", ERR_LOG);
        return false;
    }
    dbus_message_unref(msg);

    string logInfo("Dbus客户端发送一条消息. ");
    WRITE_LOG(pLog, logInfo.c_str(), DEBUG_LOG);

    return true;
}


