
#!/bin/bash

current_directory=$(pwd)

function git_clone_schudule()
{
	if [ ! -d "schudule" ]; then
    	# 克隆 Git 仓库
    	git clone https://github.com/xiaohuihuige/schudule.git
	fi	

    # 进入目录
    cd schudule || { echo "进入目录失败"; exit 1; }
	
	git pull origin main || { echo "update git"; exit 1; }

    make clean || { echo "失败"; exit 1; }

    # 编译项目
    make || { echo "编译失败"; exit 1; }

    # 安装项目
    sudo make install || { echo "安装失败"; exit 1; }

    echo "安装schudule成功！"
}

git_clone_schudule

sudo sysctl -w net.ipv4.tcp_wmem="4096  873800  12582912"
sudo sysctl -w net.core.wmem_max="12582912"
sudo sysctl -p

# 返回上级目录
cd $current_directory || { echo "返回上级目录失败"; exit 1; }

make clean || { echo "失败"; exit 1; }

# 运行 make 命令（如果需要）
make || { echo "rtmp 编译失败"; exit 1; }

echo "编译成功！"

echo "----------------开启RTMP服务----------------"
echo "*******************************************"
./out/bin/rtmp


