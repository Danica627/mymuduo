# mymuduo
使用C++11重写muduo网络库，封装成自己的网络库，包含TcpServer/TcpConnection/Channel/Poller/Epollpoller/EventLoop/Thread/TcpThread/TcpThreadpool等模块，将依赖boost库的部分转为C++11语法，使可移植性更好

本项目添加了自动编译脚本autobuild.sh
使用chmod +x autobuild.sh 给自动编译脚本增加可执行权限
./autobuild.sh 运行

example文件夹中提供了一个测试网络库的示例，编写过makefile文件，直接make运行即可
