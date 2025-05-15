# 一个轻量型的rtmp服务器
# 运行
```
./build.sh
```

# 拉流
1. 下载VLC播放器
2. 编译的结果打印里有播放地址
3. 打开VLC, 打开媒体->打开网络串流
4. 点击播放
![](./resources/1.png)
![](./resources/2.png)
![](./resources/3.png)
![](./resources/4.png)

# 调试
```
echo "/tmp/core.%e.%p" | sudo tee /proc/sys/kernel/core_pattern

ulimit -c unlimited
```
https://rtmp.veriskope.com/docs/spec#53chunking
https://zhuanlan.zhihu.com/p/645648373


## 问题
### 1.出现send资源不可用的时候，大概率是发送缓冲满了
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
## 观察和学习rtmp协议
1. 克隆nginx-rtmp-module
```
git clone https://github.com/arut/nginx-rtmp-module
```
2. 安装nginx
```
wget https://nginx.org/download/nginx-1.24.0.tar.gz
tar -zxvf nginx-1.24.0.tar.gz
cd nginx-1.24.0

./configure \
	--with-threads \
 	--with-http_stub_status_module \
 	--with-http_ssl_module \
 	--with-http_realip_module \
 	--with-stream \
 	--with-stream_ssl_module \
 	--add-module=../nginx-rtmp-module

make -j8

sudo make install

sudo ln -s /usr/local/nginx/sbin/nginx /usr/local/bin/nginx

```
3. 操作nginx
```
#查看配置是否成功配置
sudo nginx -t
#启动nginx
sudo nginx

sudo nginx -s reload
sudo nginx -s stop

sudo vim /usr/local/nginx/conf/nginx.conf

rtmp {
    server {                   # 标识为一个服务
        listen 1935            # rtmp流服务器监听的端口号
        so_keepalive=2s:1:2;   # 
        chunk_size 4000;       # 流复用块的大小，值越大cpu消耗越低
        application live {     # live是推拉流的路径名字
            live on;           # 开始实时直播
        }
    }
}
```

4. ffmpeg推流
```
ffmpeg -re -stream_loop -1 -i mountain.h264 -vcodec copy -f flv  rtmp://192.168.181.128:8890/live
ffmpeg -re -stream_loop -1  -i suiyueruge.aac -c:a aac -f flv rtmp://192.168.181.128:8890/live

tcpdump -i eth0 port 1935 -w rtmp.pcap  # 捕获RTMP流量
rtmp://192.168.181.128:8890/live
```

