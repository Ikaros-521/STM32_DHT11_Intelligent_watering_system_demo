# 前言

**注意：浇水由LED1的亮灭进行模拟**

源码参考：

&nbsp;&nbsp;&nbsp;&nbsp;正点原子定时器中断实验

&nbsp;&nbsp;&nbsp;&nbsp;正点原子RTC实验

&nbsp;&nbsp;&nbsp;&nbsp;正点原子RTC实验

&nbsp;&nbsp;&nbsp;&nbsp;正点原子TFTLCD显示实验

&nbsp;&nbsp;&nbsp;&nbsp;正点原子按键实验

&nbsp;&nbsp;&nbsp;&nbsp;正点原子蜂鸣器实验

开发板：正点原子 STM32F103 精英版

语言：C语言

开发环境：Keil5

**开发板**使用了 LED KEY BEEP  TFTLCD TIM3 RTC USART DHT11模块 

程序仅供学习参考，会有一些bug（比如：报警时长、休息时长的问题）。

## 代码下载：

[码云](https://gitee.com/ikaros-521/STM32_DHT11_Intelligent_watering_system_demo) [GitHub](https://github.com/Ikaros-521/
STM32_DHT11_Intelligent_watering_system_demo)

## 功能介绍：

 1. LCD显示当前时间（时:分:秒）、当前温度（CEL）、当前湿度（%RH）、温度上限（CEL）、湿度上限（%RH）、湿度下限（%RH）、浇水时长（min）、休息时长（min）、报警时长（sec）。已经当前设备处于的工作模式（working浇水中 resting休息中 running正常运行中）。
 2. LED0约2秒反转一次。**LED1亮表示浇水，灭表示关水**（浇水由LED1来模拟）。
 3. 按键功能；KEY0 进入修改模式，分别针对当前时间、当前温度、当前湿度、温度上限、湿度上限、湿度下限、浇水时长、休息时长，报警时长的修改（处于修改下的数值会变红），最后退出修改模式。
在修改模式下：KEY1数值+1（封顶循环），KEY_UP数值-1（封底循环）
在普通模式下：KEY1进入休息，KEY_UP进入工作。（提供了手动控制）
 4. 当前湿度低于下限或湿度正常但温度高于上限时，报警设置的时长（这里时间<=设定），开始进入浇水（LED1点亮模拟浇水），浇水过程中如果湿度高于上限，可以打断浇水（直接进入休息，休息时长是工作时长+休息时长-已工作时长），否则浇水完毕后会进入休息（时长为休息时长）。休息完毕后会重新进行条件判断。
 5. 配置的数据存储于flash中（首地址0X08070000）
 6. 串口打印相应的信息，具体参考效果图。
 7. 不接入DHT11模块是不行的，会等待模块接入。
 8. TIM4对独立看门狗进行投食，（100ms一投食，125ms的等待投食）

# 接线

DHT11 DATA —> PG11 3.3V供电

![在这里插入图片描述](https://img-blog.csdnimg.cn/20210510091720369.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L0lrYXJvc181MjE=,size_16,color_FFFFFF,t_70)

# 效果图

开始运行（我已经设置好了一些配置），现在温湿度都正常。（ps：时间不是很准）

![在这里插入图片描述](https://img-blog.csdnimg.cn/2021051009401923.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L0lrYXJvc181MjE=,size_16,color_FFFFFF,t_70)
![在这里插入图片描述](https://img-blog.csdnimg.cn/20210510094224800.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L0lrYXJvc181MjE=,size_16,color_FFFFFF,t_70)
![在这里插入图片描述](https://img-blog.csdnimg.cn/20210510094251346.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L0lrYXJvc181MjE=,size_16,color_FFFFFF,t_70)
![在这里插入图片描述](https://img-blog.csdnimg.cn/20210510094348247.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L0lrYXJvc181MjE=,size_16,color_FFFFFF,t_70)
![在这里插入图片描述](https://img-blog.csdnimg.cn/2021051009475827.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L0lrYXJvc181MjE=,size_16,color_FFFFFF,t_70)
![在这里插入图片描述](https://img-blog.csdnimg.cn/20210510094842904.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L0lrYXJvc181MjE=,size_16,color_FFFFFF,t_70)
![在这里插入图片描述](https://img-blog.csdnimg.cn/20210510094918968.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L0lrYXJvc181MjE=,size_16,color_FFFFFF,t_70)
![在这里插入图片描述](https://img-blog.csdnimg.cn/20210510094949155.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L0lrYXJvc181MjE=,size_16,color_FFFFFF,t_70)
![在这里插入图片描述](https://img-blog.csdnimg.cn/20210510095010523.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L0lrYXJvc181MjE=,size_16,color_FFFFFF,t_70)
![在这里插入图片描述](https://img-blog.csdnimg.cn/20210510095255992.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L0lrYXJvc181MjE=,size_16,color_FFFFFF,t_70)

漏拍LED1了，补充

![在这里插入图片描述](https://img-blog.csdnimg.cn/2021051009542535.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L0lrYXJvc181MjE=,size_16,color_FFFFFF,t_70)
![在这里插入图片描述](https://img-blog.csdnimg.cn/2021051009543964.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L0lrYXJvc181MjE=,size_16,color_FFFFFF,t_70)
