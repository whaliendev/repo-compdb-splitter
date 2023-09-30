#!/bin/bash

# 设置默认值
BASE_IMAGE=${1:-ubuntu:16.04}
BUILD_TYPE=${2:-Release}
TARGET_DIR="./release/${BASE_IMAGE}-${BUILD_TYPE}"

echo "BASE_IMAGE: $BASE_IMAGE"

# 构建Docker镜像
docker build --build-arg BASE_IMAGE=$BASE_IMAGE --build-arg BUILD_TYPE=$BUILD_TYPE -t db-splitter-image .

# 创建一个临时容器但不运行它
TEMP_CONTAINER=$(docker create db-splitter-image)

# 创建目标目录
mkdir -p $TARGET_DIR

# 从临时容器中复制文件
docker cp $TEMP_CONTAINER:/app/release/db-splitter $TARGET_DIR/

# 从当前目录复制README.md文件
cp ./README.md $TARGET_DIR/

# 删除临时容器
docker rm $TEMP_CONTAINER
