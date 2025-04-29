
#!/bin/bash

# 克隆 Git 仓库
git clone https://github.com/xiaohuihuige/schudule.git

# 进入目录
cd schudule || { echo "进入目录失败"; exit 1; }

# 编译项目
make || { echo "编译失败"; exit 1; }

# 安装项目
sudo make install || { echo "安装失败"; exit 1; }

echo "安装schudule成功！"

cd -

make 
