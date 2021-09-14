#ifndef DBUS_SRV_TEST_h_
#define DBUS_SRV_TEST_h_
#include "dbusBaseClass.h"

class DBusSrvTest : public DBusBaseServer
{
public:
    DBusSrvTest();
    ~DBusSrvTest();
public:
    virtual void Run(DBusParam& recvInfo);

};




#endif