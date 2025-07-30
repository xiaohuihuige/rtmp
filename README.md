# 一个轻量型的rtmp服务器
## 项目概述
基于RK3588实现了一个RTMP流媒体服务器，主要特点包括：
- 完整的RTMP协议支持: 握手、命令处理、媒体数据传输
- 高性能网络处理，使用单线程Reactor模式
- GOP缓存优化: 实现快速首屏和低延迟播放
- 多路流支持: 支持多个视频源和多路拉流
- 完善的错误处理: 异常检测和恢复机制
- 媒体管理使用工厂模式，支持AAC, H264流，或者自定义流，增加扩展性
- 使用buffer引用计数，多通道共享一个buffer，减低内存消耗

## 技术栈
- 网络框架: schudule,单线程Reactor模式
- 协议支持: RTMP 1.0 规范
- 音视频格式: H.264 + AAC
- 编程语言: C
- 构建工具: makefile

## 依赖
```
libasound.so.2 
libfaac.so
librockchip_vpu.so
librockchip_mpp.so
libschudule.so

sudo apt install libfaac-dev
sudo apt install alsa-base alsa-utils
https://github.com/rockchip-linux/mpp.git
https://github.com/xiaohuihuige/schudule.git
```

## 编译
```
make clean
make 
```
## 打开日志
```
LOG_LEVEL_ALL
LOG_LEVEL_DEBUG
LOG_LEVEL_INFO
LOG_LEVEL_WAR
LOG_LEVEL_ERR

编辑config.mk文件
选择你的日志等级
-DLOG_LEVEL=LOG_LEVEL_INFO

```

## 调试手段
### GDB调试
```
gdb ./out/bin/rtmp_online
run


echo "/tmp/core.%e.%p" | sudo tee /proc/sys/kernel/core_pattern

ulimit -c unlimited

```

### 出现send资源不可用的时候，大概率是发送缓冲满了
```
##设置TCP发送缓存区
#查看缓冲信息
sysctl net.ipv4.tcp_wmem

#设置大小
sudo sysctl -w net.ipv4.tcp_wmem="4096  87380  12582912"
sudo sysctl -w net.core.wmem_max="12582912"

#生效
sudo sysctl -p
```

###  使用tcpdump抓包
```
sudo tcpdump -i lo -w rtmp.pcap port 1935
```

### 使用wireshark分析
```
wireshark rtmp.pcap
```
### 使用top监控CPU和内存
```
top-p$(pgrep rtmp_server)
```

### 使用perf分析性能热点
```
perf record -g ./bin/rtmp_server
perf report
```

### Valgrind 是一个强大的工具，用于检测内存泄漏、内存错误和性能问题
```
valgrind --leak-check=full ./your_program
```

### 进程内存使用情况
```
sudo watch -n 0.1  cat /proc/3870453/status
VmSize:   169744 kB
VmRSS:     20552 kB
```

## 后续优化
- 支持更多的流格式
- 支持HLS/DASH输出
- 加入握手时密钥验证
- 使用多线程Reactor模式