# 2020-DBMS-project

## 项目架构

```
|-- project
    |-- 04_proj_task2_report.md
    |-- 项目运行说明.md
    |-- makefile
    |-- pm_test.c
    |-- src
        |-- data_page.cpp
        |-- main.cpp
        |-- makefile
        |-- pm_ehash.cpp
        |-- ycsb.cpp
        |-- ycsb_map.cpp
    |-- asset
        |-- PmEHash.bmp
    |-- commandLineDB
        |-- CLDB.cpp
        |-- makefile
    |-- data
    |-- gtest
    |-- images
        |-- gtest.png
        |-- ycsb(persist).png
        |-- ycsb(not persist).png
    |-- include 
        |-- data_page.h
        |-- pm_ehash.h
    |-- test
        |-- ehash_test.cpp
        |-- makefile
    |-- workloads
        |-- data_generator.cpp
        |-- makefile

```

**请注意，所有的操作请在root模式下进行。**

## 持久化数据路径配置
持久化数据路径默认为 `./data`，但是可以通过环境变量 `PERSIST_PATH` 进行配置。

例如，将持久化路径设置为 `/mnt/pmemdir/data`:
```bash
export PERSIST_PATH=/mnt/pmemdir/data/
```

如果设置的数据路径为磁盘上的路径，程序也可以正常运行，但会发出警告。

如果程序没有数据路径的访问与修改权限，程序会提示错误信息并结束运行。

## gtest运行说明

首先将 `PERSIST_PATH` 设置为持久化数据目录并创建该目录(如果整个项目文件夹就存储于NVM下，也可以不设置直接使用相对路径)，如：
``` bash
export PERSIST_PATH=/mnt/pmemdir/data/
mkdir $PERSIST_PATH
```

如果数据目录不存在，程序一般会自动创建。如果程序缺少权限创建失败的话，可以手动创建数据目录。进入**test**文件夹下,执行

``` bash
make
./bin/ehash_test
```

即可看到gtest运行结果

## ycsb运行说明

### 生成测试数据集

**workloads**目录下，执行

``` 
./data_generator 生成的测试数据文件名前缀 初始数据数量 操作次数 数据最大值
```

例如执行    

```
./data_generator test 1000000 2000000 10000
```
会生成    
1000000个初始数据到**test-load.txt**    
2000000个操作到**test-run.txt**    
数据范围都是1-10000    

### 运行ycsb测试

在根目录执行

```bash
make ycsb
./ycsb 220w-rw-50-50
```

就会将 **./workloads/220w-rw-50-50-load.txt** 和 **./workloads/220w-rw-50-50-run.txt** 的数据进行测试 。

构建 ycsb 时可以给 make 传入参数 `ARG=-DSINGLE_STEP` 启用单个操作的时间统计，如：
``` bash
make ARG=-DSINGLE_STEP ycsb
./ycsb 220w-rw-50-50
```

由于该统计会带来额外开销，因此默认不开启。

### ycsb测试时启用map对比操作结果，验证可扩展哈希的正确性

在根目录的Shell执行

```bash
make ycsb_map
./ycsb_map 220w-rw-50-50
```

就会将 **./workloads/220w-rw-50-50-load.txt** 和 **./workloads/220w-rw-50-50-run.txt** 的数据进行测试 ,测试过程中会同时调用一个C++ STL库的map来同步处理这些数据。并将可扩展哈希的计算结果与map的计算结果相比较，如果不同就会发出错误提示并终止程序。如果没有错误提示且程序正常结束，就说明可扩展哈希的运行结果和map的运行结果完全一致。

(由于map运行时也有时间开销，所以使用ycsb_map测试的运行速度相比ycsb会较慢)

## 命令行数据库

从根目录可以进入`commandLineDB`子目录。该子目录下有一个命令行数据库`./CLDB`。可以选择该命令行数据库使用的数据目录路径(可以类似前面一样用`PERSIST_PATH`指定路径，不设置`PERSIST_PATH`环境变量时默认数据目录路径为`./data/`)。

在`commandLineDB`的Shell执行

```bash
make CLDB
./CLDB 
```

就可以通过命令行使用可扩展哈希实现的键值对数据库。数据库支持插入(Insert)、删除(Remove)、修改(Update)、搜索(Search)、清空数据(Clean)等几种操作。

## 项目报告

[Project Report.md](04_proj_task2_report.md)
