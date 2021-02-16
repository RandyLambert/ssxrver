# ssxrver
```
     _______.     _______.___   ___ .______     ____    ____  _______ .______      
    /       |    /       |\  \ /  / |   _  \    \   \  /   / |   ____||   _  \     
   |   (----`   |   (----` \  V  /  |  |_)  |    \   \/   /  |  |__   |  |_)  |    
    \   \        \   \      >   <   |      /      \      /   |   __|  |      /     
.----)   |   .----)   |    /  .  \  |  |\  \----.  \    /    |  |____ |  |\  \----.
|_______/    |_______/    /__/ \__\ | _| `._____|   \__/     |_______|| _| `._____|
```

> ssxrver 是一个运行于 Linux 平台下的高性能高并发网络库，使用 C++17 进行编写，支持 TCP 和UDP 协议。

## 优势

1. 使用了优化版 one-loop-per-thread + 细粒度锁 thread-pool 模型.
2. 高性能高并发,压测数据高于 Nginx/1.14.2 与 Apache/2.4.28
3. 主线程只进行 accept 操作通过 eventfd 进行事件分发，避免锁的竞争，IO 线程进行数据读写.
4. 根据在不同场景下的分析,使用 epoll 的不同的多路复用方式,提高性能.
5. 使用 RAII 的机制进行对象生命周期控制,所有内存分配操作使用智能指针,避免了内存泄露.
6. 使用 Linux 内核提供的 timerfd 将定时事件和 IO 时间统一处理, 通过 C++11 标准库 std::chrono 和 std::priority_queue 实现定时器管理,实现纳秒级别定时任务.
7. 使用非阻塞套接字,避免线程被单个连接阻塞.
8. 封装 http 模块,简单操作就可配置出一个高性能 HTTP Server,采用 Ragel (有限状态机)进行 HTTP 请求解析,调高效率,支持 HTTP/1.0 , HTTP/1.1 的 GET、POST 请求，支持长连接.
9. 封装高性能 buffer 类进行数据发送和接收.
10. 文件发送使用 sendfile 零拷贝技术，提高文件发送性能.
11. 封装数据库操作模块，可简单解析生成 MySQL 数据库对应 sql 语句,可配合细粒度锁 thread-pool 实现数据库连接池.
12. 实现多缓冲区异步日志库,支持设立日志级别,日志滚动大小等功能.
13. 使用 std::make_shared , std::make_unique , std:string_view  , explicit , [[nodiscard]] , emplace_back 等 C++11 14 17语法新特性,提高性能.
14. 使用统一风格的代码风格和命名规范,同时添加 10 余个编译参数来规范代码实现,提高代码质量和编译器优化可能性.
15. 多处设计进行对象复用,减少某些对象频繁申请释放.
16. 使用基于对象的编程思想,项目代码结构清晰明白,互相调用频繁的函数尽量放在一起,增加 CPU Cache命中率,模块之间松耦合,极易添加新功能模块.
17. 使用单例模式,策略模式,适配器模式等设计模式,降低代码冗杂度,使实现代码实现更加优雅. 
18. 封装配置文件模块,使用 json 格式来快速进行配置.
19. 可以通过配置文件来配置 CPU 亲和度,从而减少线程直接上下文切换次数,提高性能.
20. 支持 UDP 协议.

## 开发环境

- 操作系统发型版本 : deepin v20.1 社区版(1030)
- 内核版本 : 5.4.70-amd64-desktop (64位)
- 编译器版本 : gcc 8.3
- 语言 : c++ 17
- cmake版本 : 3.11.2
- boost库版本 : 1.72
- 数据库版本 : MySQL 5.7.21-1

## 如何运行

1. 请尽量匹配与我相同的开发环境,如果不需要 数据库模块 请对应修改 CMakeLists.txt .

   - cmake 安装

     ```sh
     # debian/ubuntu
     sudo apt-get install cmake
     ```

   - boost 库安装

     ```sh
     wget http://sourceforge.net/projects/boost/files/boost/1.72.0/boost_1_72_0.tar.bz2
     tar -xvf boost_1_72_0.tar.bz2
     cd ./boost_1_72_0
     ./bootstrap.sh --prefix=/usr/local
     sudo ./b2 install --with=all
     ```

2. 在 ssxrver 目录下运行 ./build.sh , 可以修改 build.sh 选择生成 Debug 版本还是 Release 版本(默认 Release 版本)

   ```sh
   ./build.sh
   ```

3. 编译成功会生成 build/ 目录,可执行文件在对应版本的目录下,比如当你选择 Release 版本,可执行文件就在 /build/Release/ssxrver.

4. 模仿 conf/ssxrver.json.example 的格式去创建你的配置文件(注意配置文件中不能加注释,不能加注释,不能加注释),以下我对各个配置文件选项做一下解释,很多参数实际上我设定了默认值.如果不配置的话也不会有影响.

   ```sh
   {
     "port": 4507, # 端口号,不填的话默认4507
     "address": "127.0.0.1", # 绑定的地址
     "worker_processes": 4, # IO 线程数量,不填默认为 4 个
     "worker_connections": -1, # 一个 IO 线程最多支持多少连接, -1 表示最多能创建多少就创建多少,不做限制
     "task_processes": 0, # 任务线程,不填的话默认为 0 
     "cpu_affinity": "off", # cpu 亲和度 ,默认关闭
     "http": { # http 模块
       "max_body_size": 67108864, # 单个 http 包最大支持大小
       "root_path": "/home/randylambert/sunshouxun/ssxrver/html/" # 文件访问根路径
     },
     "log": { # log 模块
       "level": "INFO", # 输出等级,可填三种等级, DEBUG,INFO,WARN 不填默认为 INFO 等级
       "ansync_started": "off", # 是否打开异步日志线程,不填默认关闭
       "flush_second": 3, # 异步线程每隔多久持久化一次
       "roll_size": 67108864, # 日志文件滚动大小
       "path": "/home/randylambert/sunshouxun/ssxrver/logs/", # 日志文件存放路径
       "base_name": "ssxrver" # 日志文件基础名
     },
     "mysql": { # 数据库模块
       "mysql_started": "off", # 是否打开数据库模块,默认关闭
       "address": "127.0.0.1",# 以下是对应数据库连接信息
       "user": "root",
       "password": "123456",
       "database_name": "ttms",
       "port": 0,
       "unix_socket": null,
       "client_flag": 0
     },
     "blocks_ip": ["122.0.0.2","198.1.2.33"] # 可屏蔽部分恶意 IP
   }
   ```

5. 运行可执行文件.

```sh
./ssxrver -f /配置文件的路径
# 例如
./build/Release/ssxrver -f ./conf/ssxrver.json
```

## 压测

| 测试环境         | 数值                                    |
| ---------------- | --------------------------------------- |
| 操作系统发型版本 | deepin v20.1 社区版(1030)               |
| 内核版本         | 5.4.70-amd64-desktop (64位)             |
| 编译器版本       | gcc 8.3                                 |
| boost库版本      | 1.72                                    |
| 处理器           | Intel(R) Core(TM) i7-8750H CPU @2.20GHz |
| L1 Cache 大小    | 32K                                     |
| L2 Cache 大小    | 256K                                    |
| L3 Cache 大小    | 9216K                                   |
| 硬盘转速         | 1.8 TiB 机械硬盘 5400转                 |
| 硬盘读写速度     | 370 MB in  3.03 seconds = 122.27 MB/sec |
| 内存             | 7.6GB                                   |
| Swap分区         | 4.7GB                                   |
| 逻辑核数         | 12核                                    |

### 测试场景

1. 为控制变量,测试前重启电脑,保证测试环境没有高 CPU 负载和 高 IO 负载的其他应用.

2. 测试工具为 webbench1.5 ,去掉第一次热身数据,测试命令如下(100个客户端持续访问15秒).

   ```sh
   ./webbench -c 100 -t 15 http://127.0.0.1:8081/
   ```

3. 测试对象为 Apache/2.4.38 , nginx/1.14.2 , ssxrver.

   - Apache/2.4.38 采用默认配置
   - nginx/1.14.2 关闭 log 打印,开 4 个工作进程,打开 sendfile , 其余默认配置.
   - ssxrver LOG 级别设置为 INFO , 打开 异步日志线程,开 4 个 IO 线程.

**注: 无论是使用 webbench 还是 ab ,这种压测工具测出来的数据只能做一个简单参考,压测是一个需要全方位多角度的测试,而不是简单的运行一条命令而已,甚至在压测时数据根本没有经过网络传输,只是在内核里转了一圈.**

### 测试结果

| 网络库                              | Speed(pages/min) | Requests成功率 |
| ----------------------------------- | ---------------- | -------------- |
| ssxrver 返回在内存中生成的 response | 7107414          | 100%           |
| ssxrver 返回静态文件                | 5114376          | 100%           |
| Apache/2.4.28                       | 2884072          | 100%           |
| nginx/1.14.2                        | 4728748          | 100%           |

- ssxrver 返回在内存中生成的 response![在这里插入图片描述](https://img-blog.csdnimg.cn/20210217002106626.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3dlaXhpbl80MzU3NDk2Mg==,size_16,color_FFFFFF,t_70)
- ssxrver 返回静态文件 
![在这里插入图片描述](https://img-blog.csdnimg.cn/2021021700224129.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3dlaXhpbl80MzU3NDk2Mg==,size_16,color_FFFFFF,t_70)
- Apache/2.4.28
![在这里插入图片描述](https://img-blog.csdnimg.cn/20210217002343108.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3dlaXhpbl80MzU3NDk2Mg==,size_16,color_FFFFFF,t_70)
- nginx/1.14.2
![在这里插入图片描述](https://img-blog.csdnimg.cn/20210217002419309.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3dlaXhpbl80MzU3NDk2Mg==,size_16,color_FFFFFF,t_70)

ssxrver 的测试结果还不错,但是奇怪的是,我本以为数据会更高的,因为在我早期开发的时候,当时我很多优化还并没有做,返回直接在内存中生成的 response 时,测出来最多有接近 8000000 pages/min (接近  8000000 pages/min 的测试结果没截图,留下来一个7550778).
![在这里插入图片描述](https://img-blog.csdnimg.cn/20210217002722770.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3dlaXhpbl80MzU3NDk2Mg==,size_16,color_FFFFFF,t_70)
而当时 nginx/1.14.2 最多也超过 5000000 pages/min ,  不过无论是 ssxrver 还是 nginx/1.14.2 ,现在我怎么测也测不出来那么高的值了,我也不清楚是什么原因,导致最终结果能出现这么大的差距,(难道是我电脑老化了?￣□￣｜｜)

## 关于未来

1. 目前我个人如果有时间的话会修改 ssxrver 的 Buffer 模块和 Log 模块.

   - 首先, Buffer 模块最简单的改法是将其改成循环buffer ,从而有效的减少 Buffer 将数据前移的次数,或者直接放弃这种 Buffer 实现,重新实现一种高性能 Buffer .

   - 其次,目前的 Log 模块是模拟 C++ 的流形式写的,虽然在性能上肯定比直接用 C++ 的 iostream 要高,但是重载 << 符号形式的 Log 还是会出现 格式控制不方便的问题 和 函数调用链引起的性能问题 ,这两个问题都可以通过实现 printf 形式的 Log 来解决.

2. 由于时间原因, ssxrver 并没有实现内存管理模块,写一个通用的高性能内存管理模块是几乎不可能的 (不如直接上 jemalloc 或者 tcmalloc),但是,通过分析网络库这种场景,写出一个在这种场景下性能更高的内存管理模块还是有一点机会的,如果之有时间,我会去看看 nginx 中的实现,学习一下.  

3. 在我查询资料的时候我得到了一个结论是在 C++ 17 中,可以使用 std::string_view 替换 const string& ,会有一定的效率提升,因此我尝试将我项目中全部使用 const string& 的地方更换为 std::string_view ,但是当我在最终使用 `perf -top` 去查看更改后的负载时,意外的发现了部分函数在我使用了 std::string_view 替换之后,负载居然提高了,我很疑惑为什么会出现这种情况,由于时间原因我暂时不去追究这个问题的具体成因了,有机会去看看底层实现去查一下具体原因.
- 替换前
![在这里插入图片描述](https://img-blog.csdnimg.cn/2021021700303993.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3dlaXhpbl80MzU3NDk2Mg==,size_16,color_FFFFFF,t_70)
- 替换后
![在这里插入图片描述](https://img-blog.csdnimg.cn/20210217003044753.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3dlaXhpbl80MzU3NDk2Mg==,size_16,color_FFFFFF,t_70)
5. 在实现 http 解析模块时,第一版我采用的是直接匹配字符串的手写状态机,之后我替换为了 Ragel 实现的状态机,但是最近测试的时候我发现 http 解析函数的负载十分夸张,达到了 10% , 难道说使用 Ragel 之后反而导致了性能下降 ?(如果说解析 header 会出现如此高的系统负载,那么看来 HTTP/2.0 对性能的提高还是很可观的) 遗憾的是在之前我手写状态机的时候,我并没有测试对应解析函数的负载情况,现在我一下子我拿不出来两者的数据比较,有机会写一个 BenchMark 测测.
![在这里插入图片描述](https://img-blog.csdnimg.cn/20210217003013283.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3dlaXhpbl80MzU3NDk2Mg==,size_16,color_FFFFFF,t_70)

6. ssxrver 支持简单的 UDP 传输,但是我个人认为一个没有拥塞控制,流量控制,丢包重传功能的 UDP 框架基本可以说是没办法正常应用的,以后我有时间去学习学习 QUIC, KCP 这些协议,在补充补充 UDP 相关知识,相信更高效更灵活的 UDP 协议在未来的应用会越来越广泛的!

## 关于框架

实际上,我其实我认为目前最好的网络框架应该是,端口复用地址复用加多线程(多进程)绑定同一地址和端口,内核自动去做 accept 负载均衡 , 同时在通过协程框架 + hook 阻塞系统调用,这个框架使用之后可以在保证高性能的前提之下,同时不用主线程分发连接,还不用陷入异步回调地狱.

初此之外,如果能使用上在 Linux kernel 5.1 版本之后加入的异步 IO 机制 io_uring , 相信服务器的性能会更上一层楼,不过我目前对 io_uring 的了解并不多,暂时还没有功力去设计一个基于 io_uring 的异步 IO 网络库.