#!/bin/bash

set -e

# 如果没有 build 目录，创建该目录
if [ ! -d `pwd`/build ]; then
  mkdir `pwd`/"build"
fi

rm -rf `pwd`/build/*

cd build &&
   cmake .. &&
   make

cd ..

# 把头文件拷贝到 /usr/include/luwu 下， so 库拷贝到 /usr/lib 下
if [ ! -d /usr/include/luwu ]; then
  mkdir /usr/include/luwu
fi

if [ ! -d /usr/include/luwu/http ]; then
  mkdir /usr/include/luwu/http
fi

if [ ! -d /usr/include/luwu/http/http-parser ]; then
  mkdir /usr/include/luwu/http/http-parser
fi

if [ ! -d /usr/include/luwu/utils ]; then
  mkdir /usr/include/luwu/utils
fi

for header in `ls ./luwu/*.h`
do
  cp $header /usr/include/luwu
done

for header in `ls ./luwu/http/*.h`
do
  cp $header /usr/include/luwu/http
done

for header in `ls ./luwu/http/http-parser/*.h`
do
  cp $header /usr/include/luwu/http/http-parser
done

for header in `ls ./luwu/utils/*.h`
do
  cp $header /usr/include/luwu/utils
done

cp `pwd`/lib/libluwu.so /usr/lib

ldconfig