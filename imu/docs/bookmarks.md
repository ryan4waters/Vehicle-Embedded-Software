# 机器人、SLAM与飞控

## ROS / SLAM

- 古月居：https://www.guyuehome.com/
- SLAM小菜鸡一枚（知乎）：https://www.zhihu.com/people/xiao-hu-ge-ge-de-di-di/answers
- 第4章_机器人传感器_惯性测量单元（视频）：https://www.zhihu.com/zvideo/1598614216260124672
- xiihoo-机器人开发者社区：https://xiihoo.com/
- Books_Robot_SLAM_Navigation (GitHub)：https://github.com/xiihoo/Books_Robot_SLAM_Navigation
- 课件PDF：https://github.com/xiihoo/Books_Robot_SLAM_Navigation/blob/main/2.%E8%AF%BE%E4%BB%B6/1-%E7%AC%AC1%E5%AD%A3%EF%BC%9A%E7%AC%AC4%E7%AB%A0_%E6%9C%BA%E5%99%A8%E4%BA%BA%E4%BC%A0%E6%84%9F%E5%99%A8.pdf
- 小虎哥哥爱学习-CSDN博客：https://blog.csdn.net/hiram_zhang?type=blog
- 带自校准九轴数据融合IMU惯性传感器：https://www.cnblogs.com/hiram-zhang/p/10398959.html
- SLAM导航机器人零基础实战系列：（三）感知与大脑——2.带自校准九轴数据融合IMU惯性传感器：https://blog.csdn.net/hiram_zhang/article/details/88374518

## IMU / 惯性导航（基础、误差、滤波、融合）

- imu_声时刻的博客：https://blog.csdn.net/shenshikexmu/category_6446413.html
- IMU校正以及姿态融合：https://blog.csdn.net/shenshikexmu/article/details/80013444
- IMU 传感器：您需要了解的一切！：https://embeddedinventor.com/what-is-an-imu-sensor-a-complete-guide-for-beginners/
- 用普物方法解释科氏加速度（B站）：https://www.bilibili.com/video/BV1oU4y1b7N2/
- 3分钟搞懂科氏加速度（B站）：https://www.bilibili.com/video/BV1vf4y1u7zc/
- 关于MEMS IMU的那些事（知乎专栏）：https://www.zhihu.com/column/c_1685339396851720192
- 自动驾驶工具箱中的坐标系：https://ww2.mathworks.cn/help/driving/ug/coordinate-systems.html
- 多个坐标系下的速度和角速度分析：https://gaoyichao.com/Xiaotu/?book=math_physics_for_robotics&title=%E5%A4%9A%E4%B8%AA%E5%9D%90%E6%A0%87%E7%B3%BB%E4%B8%8B%E7%9A%84%E9%80%9F%E5%BA%A6%E5%92%8C%E8%A7%92%E9%80%9F%E5%BA%A6%E5%88%86%E6%9E%90
- 齐次坐标（维基百科）：https://zh.wikipedia.org/wiki/%E9%BD%90%E6%AC%A1%E5%9D%90%E6%A0%87
- L2范数归一化概念和优势：https://www.cnblogs.com/Yumeka/p/11180519.html
- 坐标系转换之三：欧拉角、四元数、旋转矩阵等：https://blog.csdn.net/yq_forever/article/details/79558790
- 惯性导航算法(二)-欧拉角法(下)+方向余弦矩阵(上)：https://blog.csdn.net/absll/article/details/124823867
- 方向余弦（维基百科）：https://zh.wikipedia.org/wiki/%E6%96%B9%E5%90%91%E4%BD%99%E5%BC%A6
- 方向角与方向余弦：https://zhuanlan.zhihu.com/p/163807527
- 方向余弦矩阵(DCM)简介：https://www.cnblogs.com/andychenforever/p/6298073.html
- DCM Tutorial – An Introduction to Orientation Kinematics：http://www.starlino.com/dcm_tutorial.html
- 方向余弦矩阵DCM、欧拉角、四元数：https://zhuanlan.zhihu.com/p/359738184
- 方向余弦矩阵 - 卫星百科：https://sat.huijiwiki.com/wiki/%E6%96%B9%E5%90%91%E4%BD%99%E5%BC%A6%E7%9F%A9%E9%98%B5
- 最小二乘（模型拟合）算法：https://ww2.mathworks.cn/help/optim/ug/least-squares-model-fitting-algorithms.html
- 旋转矩阵转化成四元数的三种算法：https://blog.csdn.net/shenshikexmu/article/details/53608224
- 四元数表示向量V1到V2的旋转的两种算法：https://blog.csdn.net/shenshikexmu/article/details/70991286
- AN-1057: 使用加速度计进行倾斜检测：https://www.analog.com/cn/resources/app-notes/an-1057.html
- 为啥一定用残差图检查你的回归分析？：https://iphysresearch.github.io/blog/post/ml_notes/residual-plots/
- 通过IMU的加速度计计算倾角：https://blog.csdn.net/qq_27865227/article/details/139482874
- 从零开始的 IMU 状态模型推导：https://fzheng.me/2016/11/20/imu_model_eq/
- IMU 测量模型：https://xiaotaoguo.com/p/imu-model/
- quaternion.pdf：https://krasjet.github.io/quaternion/quaternion.pdf
- 四元数和旋转(Quaternion & rotation)：https://zhuanlan.zhihu.com/p/78987582

**滤波与信号处理**

- 数字滤波器15分钟入门！：https://zhuanlan.zhihu.com/p/523565858
- 与信号处理有关的那些东东：https://www.zhihu.com/column/xinhao
- Mahony滤波器详解：https://blog.csdn.net/luoshi006/article/details/51513580
- IMU 互补滤波器：https://blog.csdn.net/weixin_41469272/article/details/113599414
- 滤波算法 - HongYi_Liang：https://www.cnblogs.com/HongYi-Liang/p/7002718.html
- RT-Thread互补滤波器：https://www.eet-china.com/mp/a234760.html
- IMU基于互补滤波的姿态解算探究：https://www.bilibili.com/opus/873809069158170657
- Keeping a Good Attitude: A Quaternion-Based Orientation Filter：https://www.mdpi.com/1424-8220/15/8/19302
- imu_tk/src/filters.cpp：https://github.com/Kyle-ak/imu_tk/blob/master/src/filters.cpp
- Choosing correct filter parameters for IMU (Signal Processing StackExchange)：https://dsp.stackexchange.com/questions/29906/choosing-correct-filter-parameters-for-imu-sensor-datas

**误差与标定**

- Debiasing 6-DOF IMU via Hierarchical Learning (arXiv)：https://arxiv.org/abs/2504.09495
- 自动驾驶融合定位：IMU内参模型及标定：https://blog.csdn.net/NEON7788/article/details/138475934
- IMU原理介绍及误差分析：https://zhuanlan.zhihu.com/p/544296512
- 多传感器融合感知 --传感器外参标定及在线标定：https://www.guyuehome.com/38051
- IMU Noise Model (kalibr wiki)：https://github.com/ethz-asl/kalibr/wiki/IMU-Noise-Model
- IMU 内参标定：https://xiaotaoguo.com/p/imu-calibration/
- IMU误差模型与校准：https://www.cnblogs.com/suibizatan/p/13272545.html
- 惯性测量单元Allan方差分析详解：https://zeal-up.github.io/2023/04/19/sensors/AllanVariance/
- imu_tools (noetic)：https://github.com/CCNYRoboticsLab/imu_tools/tree/noetic
- IMUCalibration-Gesture (GitHub)：https://github.com/shenshikexmu/IMUCalibration-Gesture
- 利用GNSS/INS数据的六自由度IMU偏移和失准补偿（专利）：https://patentimages.storage.googleapis.com/04/1d/16/579c75ad8c599e/CN107084743B.pdf
- IMU加速度、磁力计校正－－椭球拟合：https://blog.csdn.net/shenshikexmu/article/details/70143455
- IMU误差模型与校准（卜小乂）：https://www.cnblogs.com/buxiaoyi/p/7541974.html
- IMU标定（三）确定误差的标定：https://cloud.tencent.com/developer/article/1761392
- 加速度计与陀螺仪误差分析：确定性与随机误差：https://blog.csdn.net/qq_43134830/article/details/129455339
- IMU标定与解算：https://nuhuo08.github.io/imu/
- IMU误差标定方法：https://blog.csdn.net/u014430081/article/details/127175830
- 自动驾驶中的横摆角速度补偿算法：https://blog.csdn.net/qq_40875526/article/details/135047871

**融合**

- 加速度计、陀螺仪的工作原理和数据融合：https://c.miaowlabs.com/B07.html
- GTSAM之多传感器融合：https://zhuanlan.zhihu.com/p/614805045
- IMU姿态融合（MPU9250从校正到滤波步骤）：https://blog.ggrarea.cn/archives/18633.html
- IMU姿态融合算法_mahony-ekf：https://blog.csdn.net/shenshikexmu/article/details/55194836
- IMU姿态融合算法实践：https://blog.csdn.net/shenshikexmu/article/details/55194836?ops_request_misc=...
- 感知融合（六）：运动补偿算法计算目标绝对速度：https://blog.csdn.net/weixin_43877080/article/details/108059891
- Inertial Sensor Fusion — Examples (MATLAB)：https://ww2.mathworks.cn/help/fusion/examples.html
- IMU Fusion Algorithm -- Magdwick：https://longbin.tech/fusion%20algorithm/IMU-Fusion-Algorithm-Magdwick.html
- Hermit_Rabbit的博客（华为云）：https://bbs.huaweicloud.com/community/usersnew/id_1658584026524907
- xioTechnologies/Fusion (GitHub)：https://github.com/xioTechnologies/Fusion
- FusionMath.h：https://github.com/xioTechnologies/Fusion/blob/main/Fusion/FusionMath.h
- xioTechnologies 主页：https://github.com/xioTechnologies

## 飞控 (PX4)

- PX4-Autopilot (GitHub)：https://github.com/PX4/PX4-Autopilot
- PX4软件系统简介 (PDF)：https://rflysim.com/doc/zh/RflySimAPIs/2.RflySimUsage/0.ApiExps/e1_RflySimSoftwareReadme/Firmware/Readme.pdf
- PX4 Autopilot User Guide：https://docs.px4.io/main/en/
- PX4 Autopilot 官网：https://px4.io/
- PX4 Discussion Forum：https://discuss.px4.io/
- 无人机基础知识-四轴-开源飞行控制器：https://www.ncnynl.com/archives/201608/704.html
- 开源飞控PX4小结（知乎）：https://zhuanlan.zhihu.com/p/539598875
- PX4 Tutorial example code (NXP)：https://nxp.gitbook.io/hovergames/developerguide/px4-tutorial-example-code
- 一、PX4环境搭建和编译（Ubuntu 16.04、ROS kinetic）：https://www.guyuehome.com/wap/detail?id=1825474070666096641
- Kalman滤波器从原理到实现：https://www.cnblogs.com/zhoug2020/p/8376509.html
- Extended Kalman Filter (EKF) — Plane documentation：https://ardupilot.org/plane/docs/common-apm-navigation-extended-kalman-filter-overview.html
- Fusion of optical flow measurements into the Navigation EKF：https://groups.google.com/g/drones-discuss/c/SqgXeojbiZU
