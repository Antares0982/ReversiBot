# ReversiBot
黑白棋QQ机器人插件，使用C++ 20标准

### 依赖

> Requirements: 
>
> * 支持MiraiCP插件的运行环境
> * [MiraiCP源代码](https://github.com/Nambers/MiraiCP)
> * opencv2

编译测试：

- [x] Linux x64
- [ ] Windows x64
- [ ] MacOS

##### Linux命令行

确保opencv2已安装，`JAVA_HOME`等环境变量已经正确设置。见https://github.com/Nambers/MiraiCP-template 的配置方法。

首先确保环境变量`MIRAI_CPP_PATH`已经设置为https://github.com/Nambers/MiraiCP/tree/main/cpp/include 文件夹下载在本地的文件夹路径（不需要编译MiraiCP），例如`/home/user/MiraiCP/cpp/include`。可以通过cmake输出观察是否正确设置了。如果不想设置环境变量请自行修改cmakelists.

```shell
mkdir build && cd build && cmake .. && make -j8
```

即可正确编译。

如果想要使用[single include](https://github.com/Nambers/MiraiCP-template)请自行修改源代码的include文件以适配。

##### Linux Clion

参考[该讨论](https://stackoverflow.com/questions/37662130/clion-or-cmake-does-not-see-environment-variable)或自行修改cmakelists.

##### Windows

请参考[template](https://github.com/Nambers/MiraiCP-template)的编译方法。

### 运行配置

在工作目录下新建文件夹`config`，在`config/config.json`中写入如下内容：（MASTERID为管理员QQ号）

```json
{
    "MASTERID": 987654321,
    "BOTID": 123456789,
    "reversi": {
        "reversiAiName": "/path/to/reversi/Ai/program",
        "reversiAiName2": "/path/to/another/reversi/Ai/program"
      }
}
```

### AI?

> 目前暂时没有公开的AI，之后我将考虑开源一个简单的AI。欢迎来我的[黑白棋在线对战页](https://reversi.chr.fan/)游玩体验。


* AI请自行实现，插件中调用AI方法是执行AI程序并获取其stdout. 向AI程序传递的参数为如下格式：

  ```shell
  /path/to/aiprogram 5 <firstlineofmap> <secondlineofmap> ... <8thlineofmap>
  ```

  共9个参数，第一个参数（约定）为ai运行的最大时间（默认5秒），自行实现ai时可以根据需要忽略该参数（插件会阻塞直到ai结束运行，尽量不要太花时间）。第2到第9个参数中，每个参数代表一行，1为ai的棋子，2为对手的棋子，0为空。参数示范：

  ```shell
  /path/to/program 5 00000000 00000000 00000000 00021000 00012000 00000000 00000000 00000000
  ```

  AI程序的stdout需要满足：经过python的`str.split()`后的最后两个字符串将作为结果，如`"... ... 0 7"`代表ai将下在第一行最后一列（从0开始计算行列）。左上角视为0行0列。

* 目前可以向`reversi.chr.fan:7685/ai_api`发送黑白棋AI api请求，请求将被如下两行python代码解析(aiIndex支持0-7的整数，不同index参考https://reversi.chr.fan:7685/ai_list，可以在https://reversi.chr.fan上尝试，board为二维数组仅包含0,1,2，current代表轮到board中的1还是2下，后三个参数可以忽略)：

  ```python
  data = request.json
  aiIndex, board, current, _, _, _ = data.values()
  ```


### 玩家输入

* 私聊中关键词"reversi"触发与AI的游戏。在群聊中第一个输入"reversi pvp"的人将作为黑棋，第二个人回复"reversi"作为白棋。输入"abort"关键字将放弃棋局。3分钟后无响应则群内所有人都可以使用abort（目前仅支持一个群内一场游戏）。
* 群聊中使用"reversi pve"开始一局与AI的游戏。

* 玩家输入的行、列遵循自然输入而非从0开始，即：`8 8`为下在右下角的位置，而不是`7 7`。

### 管理员输入

"disable"关闭在某个群中的黑白棋功能，之后用"enable"开启。
