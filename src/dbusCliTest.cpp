#include "dbusBaseClass.h"
#include <iostream>


using namespace std;


int main(int argc, char** argv)
{
    DBusParam param;
    param.dbusName = "china.dbus.connect";
    param.interName = "/china/dbus/server";
    param.funcSigName = "testSignal";
    param.msgType = DBUS_MESSAGE_TYPE_SIGNAL;
    param.numParam.push_back(100);
    param.numParam.push_back(200);
    param.numParam.push_back(300);
    param.numParam.push_back(400);
    param.strParam.push_back("参数1");
    param.strParam.push_back("参数2");
    param.strParam.push_back("参数3");
    param.strParam.push_back("参数4");
    param.strParam.push_back("参数5");
    param.strParam.push_back("参数6");
    param.strParam.push_back("参数111");
    param.strParam.push_back("参数222222222");

    DBusBaseClien cli;
    if (!cli.DbusSendSignal(param)) {
        cout << "发送失败！" << endl;
    }

    string end;
    cin >> end;
    return 0;
}


// dbusCliTest.cpp



