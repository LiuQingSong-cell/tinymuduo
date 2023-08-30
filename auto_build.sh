#！/bin/bsh

set -e

if [ ! -d `pwd`/build ]
then
    mkdir `pwd`/build
fi

rm -rf `pwd`/build/*

cd `pwd`/build &&
    cmake .. && 
    make

cd .. # 回到项目根目录

# include -> /usr/include/tinymuduo 
# libtinymuduo.so -> /usr/lib

if [ ! -d /usr/include/tinymuduo ]
then
    mkdir /usr/include/tinymuduo
fi

for header in `ls ./include/*.h`
do
    cp $header /usr/include/tinymuduo
done

cp `pwd`/lib/libtinymuduo.so /usr/lib

ldconfig