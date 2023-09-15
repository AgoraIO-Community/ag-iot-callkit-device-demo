开始本项目之前，请按[wiki说明](https://github.com/AgoraIO-Community/ag-iot-callkit-device-demo/wiki)前往声网控制台完成申请测试所用一机一密（也称激活码）

1、构建Demo应用前须先修改example/src/app_config.h中的APP ID并联系销售(sales@agora.io)获取对应测试一机一密(node_id/node_secret)；

- CONFIG_AGORA_APP_ID需要通过[声网控制台](https://console.shengwang.cn/)创建项目ID；

2、在example目录下执行make命令可编译生成Demo

3、将hello_link拷贝到Ubuntu 18.04以上Linux系统中，确保所在路径下有运行权限和网络访问。

4、运行Demo
- 主动呼叫

`$ ./bin/test device_node_id device_node_secret app_node_id`

`#### Input your command: "c", "h", "a" or "q"`

运行成功后，有上述提示信息，分别对应：“呼叫”，“挂断”，“接听”，“退出”命令

比如，输入“c” + 回车，则会呼叫对端ID(运行程序时指定的app_node_id)。

- 被动呼叫

当Demo运行成功后，如果从其他方式来呼叫该Demo，则会收到呼叫请求，提示信息如下：

`#### Get call from peer "100***000-704***400", attach message: ***`

此时，如果不想接听，输入`h`+ 回车来拒绝呼叫请求；如果需要接听，输入`a` + 回车来接受呼叫并进入视频通话。

5、API集成说明（纯呼叫模式）

参考hello_doorbell.c示例程序。
