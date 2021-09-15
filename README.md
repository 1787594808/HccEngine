# HccEngine
### 安装手册

#### 1.安装   tensorflow

1. 在https://storage.googleapis.com/tensorflow/libtensorflow/libtensorflow-cpu-linux-x86_64-1.10.1.tar.gz下载tensorflow1.10.1版本 Linux 下 cpu 版本安装包。

2. 执行下列代码解压

   ```shell
   sudo tar -C /usr/local -xzf libtensorflow-cpu-linux-x86_64-	1.10.1.tar.gz
   ```

3. 配置链接器：

   ```shell
   sudo ldconfig
   ```

#### 2.安装 qt

https://www.cnblogs.com/tweechalice/p/13406608.html

#### 3.安装 opencv

1.安装 cmake

```shell
sudo apt-get install cmake
```

2.安装依赖

```shell
sudo apt-get install build-essential libgtk2.0-dev libavcodec-dev libavformat-dev libjpeg-dev libswscale-dev libtiff5-dev
sudo apt-get install libgtk2.0-dev
sudo apt-get install pkg-config
```

3.下载opencv

下载地址：https://opencv.org/releases/
点击Sources进行下载3.4.14的版本

![image-20210628190229885](C:\Users\dell\AppData\Roaming\Typora\typora-user-images\image-20210628190229885.png)

4.安装opencv。解压进入对应文件夹，创建build文件夹，进入build文件夹。

```shell
mkdir build
cd build
sudo cmake -D CMAKE_BUILD_TYPE=Release -D CMAKE_INSTALL_PREFIX=/usr/local ..
sudo make或sudo make -j 8
sudo make install
```

5.配置环境

(1)打开opencv.conf 

```shell
sudo gedit /etc/ld.so.conf.d/opencv.conf 
```

(2)在文末添加 

```shell
/usr/local/lib
```

(3)保存后，执行如下命令使得刚才的配置路径生效

```shell
sudo ldconfig
```

(4)配置 bash

```shell
sudo gedit /etc/bash.bashrc
```

(5)文件末尾添加

```shell
PKG_CONFIG_PATH=$PKG_CONFIG_PATH:/usr/local/lib/pkgconfig  
export PKG_CONFIG_PATH  
```

(6)保存，执行如下命令使得配置生效

```
source /etc/bash.bashrc
```

(7)更新

```shell
sudo updatedb 
```

6.测试

（1）转到 opencv-3.4.14/sample/cpp/example_cmake 目录下，打开终端运行

```shell
cmake .
make
./opencv_example
```

如果摄像头打开，左上角出现 Hello OpenCV 则证明安装成功

#### 4. 安装libibus-1.0-dev

```shell
sudo apt-get install libibus-1.0-dev
```

![image-20210628173113709](C:\Users\dell\AppData\Roaming\Typora\typora-user-images\image-20210628173113709.png)



#### 5.安装输入法

（1）进入HccEngine文件夹

（2）进行编译

```shell
./ish.sh
```

 (3)重启ibus

```shell
ibus restart
ibus-setup
```

选择MyEngine进行添加

进入setting-Region&Language中input添加Myengine

Manage installed Language中设置keyboard input mothod system为IBUS框架
