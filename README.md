# 2020-DBMS-project

## 项目架构

```
|-- project
    |-- 04_proj_task2_report.md
    |-- 项目运行说明.md
    |-- src
        |-- data_page.cpp
        |-- main.cpp
        |-- makefile
        |-- pm_ehash.cpp
        |-- ycsb.cpp
    |-- asset
        |-- PmEHash.bmp
    |-- data
    |-- gtest
    |-- images
        |-- gtest.png
        |-- ycsb.png
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

## gtest运行说明

如果项目根目录下没有**data**文件夹的话，请先在根目录执行    

``` bash
mkdir data
```

来创建本次实验需要的持久数据目录。  
进入**test**文件夹下,执行

``` bash
export PERSIST_PATH=../data/
make
./bin/ehash_test
```

即可看到运行结果

## ycsb运行说明

### 生成测试数据集

**workloads**目录下，执行

``` 
./data_generator 文件名前缀 初始数据数量 操作次数 数据最大值
```

例如执行    

```
./data_generator test 1000000 2000000 10000
```
会生成    
1000000个初始数据到**test-load.txt**    
2000000个操作到**test-run.txt**    
数据范围都是1-10000    

### 运行ycsb

在根目录执行

```bash
make ycsb
./ycsb test
```

就会将 **./workloads/test-load.txt** 和 **./workloads/test-run.txt** 的数据进行测试 。

构建 ycsb 时可以给 make 传入参数 `ARG=-DSINGLE_STEP` 启用单个操作的时间统计，如：
``` bash
make ARG=-DSINGLE_STEP ycsb
./ycsb test
```

由于该统计会带来额外开销，因此默认不开启。


