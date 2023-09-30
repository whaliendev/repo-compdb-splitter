# 使用ARG来定义我们可以在构建时传递的变量
ARG BASE_IMAGE=ubuntu:16.04
ARG BUILD_TYPE=Release

# 使用基础镜像
FROM ${BASE_IMAGE}

# 定义构建参数为环境变量，以便在后续的命令中使用
ENV BUILD_TYPE=${BUILD_TYPE}

# 更新并安装必要的软件包
RUN apt-get update && apt-get install -y software-properties-common wget curl

# 根据Ubuntu版本安装GCC和Python
RUN if [ "$(lsb_release -rs)" = "16.04" ]; then \
    echo "ubuntu 16.04" && \
    apt-get install -y build-essential software-properties-common && \
    add-apt-repository ppa:ubuntu-toolchain-r/test -y && \
    apt-get update && \
    apt-get install -y gcc-snapshot gcc-6 g++-6 gcc-4.8 g++-4.8 && \
    update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-6 60 --slave /usr/bin/g++ g++ /usr/bin/g++-6 && \
    update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-4.8 60 --slave /usr/bin/g++ g++ /usr/bin/g++-4.8 && \
    echo 1 | update-alternatives --config gcc; \
    add-apt-repository -y ppa:jblgf0/python && \
    apt-get update && \
    apt-get install -y python3.8; \
    update-alternatives --install /usr/bin/python3 python3 /usr/bin/python3.8 1; \
    else \
    echo "ubuntu 18.04" && \
    apt-get install -y python3.6 && \
    update-alternatives --install /usr/bin/python3 python3 /usr/bin/python3.6 1; \
    fi


# 更新pip并安装其他必要的依赖
RUN apt-get install -y python3-pip && \
    pip3 install --upgrade pip && \
    pip install conan cmake

# 设置工作目录
WORKDIR /app

# 复制项目文件到容器中
COPY . .

# 初始化conan并安装依赖
RUN conan profile detect --force && \
    sed -i 's/compiler.cppstd=[^ ]*/compiler.cppstd=17/' ~/.conan/profiles/default && \
    conan install . --build=missing -r=conancenter -s build_type=${BUILD_TYPE}

# 构建项目
RUN cmake --preset conan-${BUILD_TYPE} && \
    cmake --build build/${BUILD_TYPE}

# 将二进制文件移到release目录
RUN mkdir -p release && cp ./build/${BUILD_TYPE}/bin/db-splitter release/
