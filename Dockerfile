# 基于 Ubuntu 18.04 的基础镜像
FROM --platform=linux/amd64 ubuntu:18.04

# 设置环境变量
ENV DEBIAN_FRONTEND=noninteractive

# 更新包列表并安装必要的工具
RUN apt-get update && apt-get install -y \
    software-properties-common \
    wget \
    && add-apt-repository ppa:ubuntu-toolchain-r/test -y \
    && apt-get update \
    && apt-get install -y \
    g++-7 \
    cmake \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

# 设置 g++ 的默认版本
RUN update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-7 100

# 下载并安装 cmake 3.22.0
RUN wget https://github.com/Kitware/CMake/releases/download/v3.22.0/cmake-3.22.0-linux-x86_64.tar.gz \
    && tar -zxvf cmake-3.22.0-linux-x86_64.tar.gz \
    && cp -r cmake-3.22.0-linux-x86_64/* /usr/ \
    && rm -rf cmake-3.22.0-linux-x86_64 cmake-3.22.0-linux-x86_64.tar.gz

# 设置工作目录
WORKDIR /app

# 默认命令
CMD ["bash"]