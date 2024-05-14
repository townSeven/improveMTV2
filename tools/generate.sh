#!/usr/bin/env bash
set -euo pipefail

# 指定生成的表文件存储目录 （*** 输入该参数时请用绝对路径！***）
TABLE_DIR=$1
# 如果目录不存在，则创建该目录
mkdir -p "$TABLE_DIR"

# 创建临时目录
TMPDIR=`mktemp --directory`
echo $TMPDIR

# 将初始化 SQL 文件复制到临时目录
# cp data/initialize.sql $TMPDIR/initialize.sql

# 进入临时目录
pushd $TMPDIR

# 下载 dbgen 工具并解压
echo 'd37618c646a6918be8ccc4bc79704061  dbgen.zip' | md5sum --check --status 2>/dev/null || curl -OL https://db.in.tum.de/~fent/dbgen/ssb/dbgen.zip
echo 'd37618c646a6918be8ccc4bc79704061  dbgen.zip' | md5sum --check --status
unzip -u dbgen.zip
mv dbgen/* .
rm -rf dbgen
rm dbgen.zip
rm -rf ./*.tbl
sed -i 's/#define  MAXAGG_LEN    10/#define  MAXAGG_LEN    20/' shared.h
make dbgen

# 指定比例因子
SF=$2

# 生成表文件
./dbgen -T c -s "$SF" #-b "$TABLE_DIR"
./dbgen -T d -s "$SF" #-b "$TABLE_DIR"
./dbgen -T p -s "$SF" #-b "$TABLE_DIR"
./dbgen -T s -s "$SF" #-b "$TABLE_DIR"
./dbgen -T l -s "$SF" #-b "$TABLE_DIR"

mv *.tbl "$TABLE_DIR"
# 设定表文件只读权限
# chmod +r $TABLE_DIR/*.tbl

# 返回上一级目录
popd

echo "Table files generated and stored in: $TABLE_DIR"
