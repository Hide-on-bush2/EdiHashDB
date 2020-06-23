# 2020-DBMS-project

**小组成员**

| 学号 | 姓名 |
| :-------:| :---:|
| 18308008 | 车春江|
| 18353070 | 谭嘉伟|
| 17364025 | 贺恩泽|
| 18324061 | 文君逸|

## 项目内容

&emsp;&emsp;实验项目完成的目标是完成基于针对NVM优化的可扩展哈希的数据结构，实现一个简单的键值存储引擎PmEHash。我们用data_page来实现数据页表的相关操作实现。    
&emsp;&emsp;此项目对外可用的对数据的基本操作就增删改查：

+ Insert增
+ Remove删
+ Update改
+ Find查
+ 删除所有数据
+ 恢复状态
  

&emsp;&emsp;在上述的每一次操作后都能立刻将数据持久化，不仅在程序关闭重新运行后能够重新恢复状态，而且可以防止程序运行过程中崩溃或掉电导致的数据崩溃。  
&emsp;&emsp;此项目需要完成的任务有：

+ 用内存模拟NVM
+ 实现代码框架的功能并进行简单的Google test，运行并通过ehash_test.cpp中的简单测试
+ 编写main函数进行YCSB benchmark测试，读取workload中的数据文件进行增删改查操作，测试运行时间并截图性能结果。

## 项目实现方法

## gtest结果

![gtest](./images/gtest.png);

## ycsb结果

![ycsb](./images/ycsb.png)