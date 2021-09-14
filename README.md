# dbus
linux下使用dbus进行进程通讯，实现类似Windows下的消息机制


只需要安装dbus，然后使用C的API，无需安装其他任何软件。

deepin下只需要安装以下包就可以通过测试：
apt install libdbus-1-dev


编译时缺少dbus-arch-deps.h头文件，要去/usr/lib目录下查找，x86和arm的机器放的位置不一样，然后加一个软链接：
ln -s  文件全名   /usr/include/dbus-1.0/dbus/dbus-arch-deps.h
或者直接把头文件复制过来






