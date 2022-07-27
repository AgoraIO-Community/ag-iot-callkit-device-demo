1、构建Demo应用前须先修改example/src/app_config.h设置相关参数为你新创建的产品；[详情请参考官方文档](https://docs.agora.io/cn/iot-apaas/device_media_call?platform=All%20Platforms)

- CONFIG_PRODUCT_KEY需要修改为自己创建的产品ID；
- CONFIG_USER_ID不需要指定，纯呼叫不需要这个参数；
- CONFIG_DEVICE_ID可以修改自定义的设备名称，如果忘记修改app_config.h中的定义，后续运行Demo时，可以通过hello_agora_call [-d deviceID]来指定。

2、将hello_agora_call拷贝到Ubuntu 18.04以上Linux系统中，确保所在路径下有运行权限和网络访问。

3、运行Demo
- 主动呼叫对方

`$ ./bin/test -d dev_tst_001`

`#### Input your command: "call", "hangup", "answer", "alarm", or "quit"`

运行成功后，有上述提示信息，分别对应：“呼叫”，“挂断”，“接听”，“告警”，“退出”命令

比如，输入“call”，则继续提示输入呼叫对方的ID（如果该设备已有绑定用户，则会顺便提示绑定的用户ID信息，如果没有绑定则为空）。

`#### Input the peer's ID (ps. your binding user ID: 100***000-684***872):`

如果需要呼叫已绑定的用户，则直接输入提示的用户ID即可；如果需要呼叫其他设备或用户，则需要手动输入设备或用户ID。

这里我们先测试下自己绑定的用户ID，输入：`100***000-684***872`

对应手机端登录的绑定账号就会收到通知，在手机侧接听即可。

当需要结束通话时，输入命令`hangup`则自动挂断通话。

- 被动呼叫

当Demo运行成功后，如果从其他方式来呼叫该Demo，则会收到呼叫请求，提示信息如下：

`#### Get call from peer "100***000-704***400", attach message: ***`

此时，如果不想接听，输入`hangup`来拒绝呼叫请求；如果需要接听，输入`answer`来接受呼叫并进入视频通话。

4、API集成说明（纯呼叫模式）

纯呼叫模式下，设备和用户之间是不需要绑定关系的。所以可以实现设备与设备、设备与用户之间的呼叫。

`agora_iot_register_and_bind(CONFIG_MASTER_SERVER_URL, CONFIG_PRODUCT_KEY, device_id, NULL, NULL, &device_info);`

其中，参数`user_id`和`device_nickname`都不需要指定，直接设置为NULL即可。那么开发者需要自己来维护设备和用户之间的关系。