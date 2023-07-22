# 																				思考题

1. > • 在前述已初始化的 ~/learnGit 目录下，创建一个名为 README.txt 的文件。 执行命令 git status > Untracked.txt。 
   >
   > • 在 README.txt 文件中添加任意文件内容，然后使用 add 命令，再执行命令 git status > Stage.txt。 
   >
   > • 提交 README.txt，并在提交说明里写入自己的学号。 • 执行命令 cat Untracked.txt 和 cat Stage.txt，对比两次运行的结果，体 会 README.txt 两次所处位置的不同。
   >
   >  • 修改 README.txt 文件，再执行命令 git status > Modified.txt。
   >
   >  • 执行命令 cat Modified.txt，观察其结果和第一次执行 add 命令之前的 status 是否一样，并思考原因。

   不一样。因为在之后的add，commit等动作中，将文件上传到了版本库，处于modifiled状态。这里Modifiled.txt中显示了尚未暂存以备提交的变更；而Stage.txt中显示尚无提交。

2. > 仔细看看0.10，思考一下箭头中的 add the file 、stage the file 和 commit 分别对应的是 Git 里的哪些命令呢？

   + add the file 是`git add`命令。
   + stage the file也是`git add`命令。
   + commit 是 `git commit -m "<message>"`命令。

3. > 1. 代码文件 print.c 被错误删除时，应当使用什么命令将其恢复？
   > 2. 代码文件 print.c 被错误删除后，执行了 git rm print.c 命令，此时应当 使用什么命令将其恢复？ 
   > 3. 无关文件 hello.txt 已经被添加到暂存区时，如何在不删除此文件的前提下 将其移出暂存区？
   
   1. 使用 `git checkoyt -- print.c`恢复文件。
   2. 首先 `git reset HEAD printf.c` 重写暂存区的目录树，再用`git checkoyt -- print.c`恢复文件。
   3. 直接使用 `git rm --cached printf.c` 文件从暂存区删除。


4. > • 找到在/home/21xxxxxx/learnGit 下刚刚创建的 README.txt 文件，若不存 在则新建该文件。 • 在文件里加入 Testing 1，git add，git commit，提交说明记为 1。
   >
   >  • 模仿上述做法，把 1 分别改为 2 和 3，再提交两次。
   >
   >  • 使用 git log 命令查看提交日志，看是否已经有三次提交，记下提交说明为 3 的哈希值a。
   >
   >  • 进行版本回退。执行命令 git reset --hard HEAD^后，再执行 git log，观 察其变化。
   >
   >  • 找到提交说明为 1 的哈希值，执行命令 git reset --hard  后，再执 行 git log，观察其变化。
   >
   >  • 现在已经回到了旧版本，为了再次回到新版本，执行 git reset --hard  ，再执行 git log，观察其变化。
   
   - 第三次提交的哈希值为c0dc6fbd24a585b881cbea1bdf812a7f88180cd6。
   
   - 执行`git reset --hard HEAD^`后第三次提交的log消失了。
   - 执行`git reset --hard`后第二次提交的log消失了。
   - 再次执行`git reset --hard`后，又返回到了最开始的版本，有记录为1，2，3的三条记录。


5. >  执行如下命令, 并查看结果 
   >
   > ```shell
   > echo first 
   > echo second > output.txt 
   > echo third > output.txt 
   > echo forth >> output.txt
   > ```
   
   - 第一条指令再窗口输出了first。
   - 第二条指令再output文件中写入了second。
   - 第三条指令写入了third将之前的内容覆盖。
   - 第四条指令再文件后接着写入forth最后的文件内容为third和forth。


6. >  使用你知道的方法（包括重定向）创建下图内容的文件（文件命名为 test）， 将创建该文件的命令序列保存在 command 文件中，并将 test 文件作为批处理文件运行， 将运行结果输出至 result 文件中。给出 command 文件和 result 文件的内容，并对最 后的结果进行解释说明（可以从 test 文件的内容入手）. 具体实现的过程中思考下列问 题: echo echo Shell Start 与 echo `echo Shell Start`效果是否有区别; echo echo $c>file1 与 echo `echo $c>file1`效果是否有区别.
   >
   >  ![image-20230305190156274](C:\Users\az\AppData\Roaming\Typora\typora-user-images\image-20230305190156274.png)
   
   #### command
   
   ```shell
   echo 'echo Shell Start...' >> test
   echo 'echo set a = 1' >> test
   echo 'a=1' >> test
   echo 'echo set b = 2' >> test
   echo 'b=2' >> test
   echo 'echo set c = a+b' >> test
   echo 'c=$[$a+$b]' >> test
   echo 'echo c = $c' >> test
   echo 'echo save c to' ./file1 >> test
   echo 'echo $c>file1' >> test
   echo 'echo save b to ./file2' >> test
   echo 'echo $b>file2' >> test
   echo 'echo save a to ./file3' >> test
   echo 'echo $a>file3' >> test
   echo 'echo save file1 file2 file3 to file4' >> test
   echo 'cat file1>file4' >> test
   echo 'cat file2>>file4' >> test
   echo 'cat file3>>file4' >> test
   echo 'echo save file4 to' ./result >> test
   echo 'cat file4>>result' >> test
   ```
   
   #### result
   
   ```
   3
   2
   1
   ```
   
   echo echo Shell Start 和 echo echo Shell Start没有区别。echo echo $c>file1是将c所 echo $c(c所代表的数据) 输入导file1，echo echo $c>file1 是将字符串echo $c>file1输出的窗口(这里\$c就是字符串\$c)。

# 											实验难点

实验难点主要在于我第一次学习os相关的知识，对linux命令行，shell和Makefile的语法完全不熟悉，导致我花了10个多小时学习相关知识。而且在写作业的时候也经常利用搜索引擎在菜鸟教程或者csdn等网站上寻找类似的教程，花了很多时间。在这次的pre和lab0作业中，exercise 0.4 的第一题我花的时间最多，当然这也是源于我对shell语法和特性不熟悉所导致的。

这道题目很简单，只要用sed命令将文件中的char改成int就可以了，我最初写的代码如下

```shell
#!/bin/bash
first=$2
secend=$3
sed -i 's/${first}/${secend}/g' $1
```

由于shell中单引号的特性，这串代码实现的是用\${first}\$替换${secend}\$字符串，而非用第二个输入的字符串替换第三个输入的字符串。后来通过我搜索资料，了解了相应的特性。将代码修改为了下面这样，成功实现了替换功能。

```shell
#!/bin/bash
first=$2
secend=$3
sed -i "s/${first}/${secend}/g" $1
```

# 											实验体会

本次实验我花费了很多时间，但是我所写的代码并不是很多，而且要实现的功能也是很基础的，这很大程度上是照顾了我这种对git，linux，shell，Makefile完全不了解的学生。即使做完了lab0实验，我对这些内容依旧感到有点陌生，对于自己所写的代码能否实现自己想要实现的功能常常会抱有疑惑；在完成实验作业的过程，我常常是带着代码真的能跑的吗的心态来运行的，同时也在网上寻找相关资料，来辅助自己完成作业。同时，也有一些文件已经给出了部分代码，通过这些代码，我也很好的理解了语法和命令行，对我帮助很大。

对我来说，很多命令行和语法，很难通过单纯阅读来接受，即使接受了也很快会忘记。所以，我要通过不断试验来强化我对命令行和语法的记忆和理解，强化认识。

直到如今，我对sed，awk等命令依旧不是很理解，还需要我在日后的实验和阅读中，加强对语法的学习，多看代码，提高自己的水平。