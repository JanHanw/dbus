#include "dbusSrvTest.h"
#include <iostream>


using namespace std;

DBusSrvTest::DBusSrvTest() : DBusBaseServer()
{

}

DBusSrvTest::~DBusSrvTest()
{

}

void DBusSrvTest::Run(DBusParam& recvInfo)
{
    cout << "DBusSrvTest recv dbus message!" << endl;
}


int main(int argc, char** argv)
{
    DBusSrvTest srv;
    string reqName("china.dbus.connect");
    string rule("/china/dbus/server");
    if (!srv.DbusInit(reqName, rule)) {
        cout << "DBUS启动失败!" << endl;
        return -1;
    }

    cin >> reqName;
    srv.Stop();

    return 0;
}






