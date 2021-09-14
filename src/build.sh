#!/bin/bash
date

ccflag="g++ -g "
includePath="-I../include -I/usr/include/dbus-1.0 "
midFlag="-fPIC -shared "

sourceFiles="dbusBaseClass.cpp "
objPro="-o ../lib/libdbusBaseClass.so"

buildLine="$ccflag $midFlag $includePath $sourceFiles $objPro"
echo $buildLine && $buildLine




sourceFiles="dbusSrvTest.cpp "
objPro="-o ../bin/dbusSrvTest"
libs="-L../lib -ldbusBaseClass -ldbus-1 -lpthread"
buildLine="$ccflag $includePath $sourceFiles $objPro $libs "
echo $buildLine && $buildLine




