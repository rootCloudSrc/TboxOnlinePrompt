# TboxOnlinePrompt
盒子上线提醒工具

# V00.01
1.只对pm客户有效，代码里面写死了其账户相关信息
2.脚本实现了盒子上线，托盘小图标闪烁提示功能
--------------存在问题-------------
  .tablewidget设置背景绿色后（即盒子上线），删掉单元格内容，点击刷新，背景颜色未被消除

  .长时间运行8小时左右，程序卡死崩溃，怀疑是日志显示控件内容过多了
	---解决方案初步设想：加入日志功能，就减少日志显示控件输出内容量，或者设置日志量上限，超量删除第一条（反正有日志文件可查） 
  .断网/缺少ssl库支持 情况下，会在获取token流程死循环并且频率极高导致输出日志到tablewidget过于频繁而程序卡死




******************************************************************************************************************************
一、开发环境
	1.qt 5.14 ，编译器选择msvc 2017 64bit
	2.依赖库libssl 版本OpenSSL 1.1.1d (实际上我只找到了OpenSSL 1.1.1t版本也能用),可通过打印输出支持的版本和当前环境是否已有对应库
		qDebug()<<"QSslSocket="<<QSslSocket::sslLibraryBuildVersionString(); //输出当前QT支持的openSSL版本
    qDebug() << "OpenSSL支持情况:" << QSslSocket::supportsSsl();//如果此平台支持SSL，则返回true; 否则，返回false。如果平台不支持SSL，则套接字将在连接阶段失败:qt.network.ssl: QSslSocket::connectToHostEncrypted: TLS initialization failed

		如果当前开发环境缺少openssl库，现成库下载地址：http://slproweb.com/products/Win32OpenSSL.html
		.将下载的安装包进行安装，安装路径可自定义
		.安装之后，找到安装目录下的两个文件(libcrypto-1_1.dll 和libssl-1_1.dll)，拷贝到QT编译器目录下即可(C:\Qt\Qt5.14.2\5.14.2\msvc2017_64\bin)。

******************************************************************************************************************************
![image](https://github.com/rootCloudSrc/TboxOnlinePrompt/assets/36293079/a667180d-e8e8-4850-8420-4162e6b11687)
![image](https://github.com/rootCloudSrc/TboxOnlinePrompt/assets/36293079/b4a8de89-87f1-4f7b-b878-2359d9c27902)



