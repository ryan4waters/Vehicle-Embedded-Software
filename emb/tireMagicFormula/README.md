# Tire Magic Formula

按**汽车纵向/横向动力学 + 控制算法开发**的视角，把轮胎魔术公式（Pacejka Magic Formula）从物理意义、公式结构、参数辨识，一直到 Simulink 实现完整串起来。



## 1. 轮胎魔术公式到底解决什么问题？

车辆动力学里面，我们最终关心的是：**轮胎在某一个垂向载荷、纵向滑移和侧偏角下，到底能产生多大的纵向力和横向力？**

例如：

- 驱动时轮胎产生 $F_x$
- 制动时轮胎产生 $F_x$
- 转向时轮胎产生 $F_y$
- 驱动 + 转向同时发生时，同时存在 $F_x,F_y$

所以可以把轮胎模型理解成：$(F_z,\kappa,\alpha) \rightarrow (F_x,F_y)$

其中：

| 变量     | 含义         |
| -------- | ------------ |
| $F_z$    | 轮胎垂向载荷 |
| $\kappa$ | 纵向滑移率   |
| $\alpha$ | 轮胎侧偏角   |
| $F_x$    | 纵向轮胎力   |
| $F_y$    | 横向轮胎力   |

这就是 Magic Formula 最核心的作用。



## 2. 为什么不能简单用 $F=\mu F_z$？

最简单的轮胎模型：$F_x=\mu F_z$

这个模型只告诉你：最大摩擦力是多少。

但是车辆控制开发真正关心的是：**滑移率变化以后，轮胎力怎么变化？**

例如制动：

```text
轮胎力
 ^
 |                 ______ 最大附着
 |              __/
 |           __/
 |        __/
 |     __/
 |____/______________________> 滑移率
       ↑
       最佳滑移率
```

通常：

- 低滑移率：轮胎力快速增加
- 达到峰值：最佳附着区域
- 继续增加滑移：轮胎逐渐进入滑移状态
- 极端滑移：轮胎力可能下降

所以：$F_x=f(\kappa,F_z)$

而不是简单：$F_x=\mu F_z$

这就是 Magic Formula 存在的意义。



## 3. Magic Formula 最经典的形式

最经典的 Pacejka 形式：$\boxed{ Y=D\sin \left[ C\tan^{-1} \left( Bx-E(Bx-\tan^{-1}(Bx)) \right) \right] }$

完整一点：$\boxed{ Y=D\sin \left\{ C\tan^{-1} \left[ B(X+S_h) -E \left( B(X+S_h) -\tan^{-1}[B(X+S_h)] \right) \right] \right\} +S_v }$

这是理解 Magic Formula 最重要的一条公式。



## 4. B、C、D、E分别是什么？

这是理解 MF 的核心。

可以把它理解成：

```text
              D
              ↑
              │       ______
              │      /
              │    /
              │  /
              │ /
──────────────┼──────────────→ X
             /
            /
```

### 4.1. D：峰值

$D\approx \mu F_z$

所以 D 主要决定：**轮胎最多能产生多大的力。**

例如：$F_z=4000N$

如果：$\mu=1$

那么：$D=4000N$

如果：$\mu=1.2$

那么：$D=4800N$



### 4.2. C：曲线形状

C 是 shape factor。

它主要决定曲线整体形状。

一般可以粗略理解成：

- C 小 → 曲线更平缓
- C 大 → 峰值附近更加尖锐

在工程调参中：

```text
C
│
├── 决定曲线整体形状
│
└── 不是简单的“刚度”
```

所以不要把 C 直接理解成轮胎刚度。



### 4.3. B：刚度相关参数

B 是 stiffness factor。

最重要的是：$\boxed{B C D}$

它和曲线初始斜率高度相关。

例如：$K_0 \approx BCD$

因此：**B 本身不是轮胎刚度，而是与 C、D 一起决定初始斜率。**

这点在参数辨识中特别重要。



### 4.4. E：曲率因子

E 决定：峰值附近以及峰值之后曲线怎么弯。

简单理解：

```text
E
↓
改变峰值附近的弯曲程度
↓
改变进入饱和区之后的形态
```

所以：

- B → 初始变化速度
- C → 整体形状
- D → 峰值
- E → 曲率

这是最常用的记忆方式。



## 5. Magic Formula 和轮胎物理量之间的对应

可以记成：$\boxed{ B\rightarrow 初始斜率 }$$\boxed{ C\rightarrow 曲线形状 }$$\boxed{ D\rightarrow 峰值 }$$\boxed{ E\rightarrow 峰值附近曲率 }$

而：

$S_h$负责横向偏移。

$S_v$负责纵向偏移。



## 6. 纵向 Magic Formula

对于驱动/制动：$X=\kappa$

因此：$F_x= D_x \sin \left[ C_x \tan^{-1} \left( B_x\kappa- E_x (B_x\kappa-\tan^{-1}(B_x\kappa)) \right) \right]$

这里：$\kappa$就是纵向滑移率。



## 7. 滑移率到底怎么定义？

这里一定要注意**符号约定**。

我在代码中采用：$\kappa>0$：驱动滑移

一种常见定义：

驱动：$\boxed{ \kappa= \frac{R\omega-V_x}{V_x} }$

例如：

车速：$V_x=100km/h$

轮胎等效线速度：$R\omega=105km/h$

那么：$\kappa=5\%$

说明轮胎存在驱动滑移。



## 8. 制动滑移率

另一种常见定义：$\kappa= \frac{R\omega-V_x}{V_x}$

那么制动时：$R\omega < V_x$

所以：$\kappa<0$

因此：

```text
κ < 0       制动
κ = 0       纯滚动
κ > 0       驱动
```

但是不同 OEM / ABS / ESC 算法可能采用不同定义。

所以实际项目一定要先统一：**slip ratio sign convention**

否则很容易出现：

```text
模型认为在驱动
控制器认为在制动
```



## 9. 横向 Magic Formula

侧向：$X=\alpha$

因此：$F_y= D_y \sin \left[ C_y \tan^{-1} \left( B_y\alpha- E_y (B_y\alpha-\tan^{-1}(B_y\alpha)) \right) \right]$

这里：$\alpha$就是轮胎侧偏角。



## 10. 什么叫侧偏角？

例如：

```text
              车辆速度 V
                 ↗
                /
               /
--------------●----------→ 轮胎指向
             α
```

轮胎实际运动方向和轮胎指向之间存在夹角：$\alpha$

轮胎产生：$F_y$



## 11. 侧偏角越大，横向力不一定越大

大概：

```text
Fy
│
│              ______
│           __/
│        __/
│     __/
│  __/
│_/
└────────────────────→ α
       ↑
     峰值
```

这和纵向滑移率非常类似。

所以：$F_y=f(\alpha,F_z)$

而不是：$F_y=C_\alpha\alpha$



## 12. 线性轮胎模型其实是 Magic Formula 的局部近似

低侧偏角时：$F_y\approx C_\alpha\alpha$

这里：$C_\alpha= \left. \frac{\partial F_y}{\partial\alpha} \right|_{\alpha=0}$

也就是：**侧偏刚度。**

所以：

```text
线性轮胎模型
       ↓
小 α
       ↓
Magic Formula 局部线性化
```

而不是两个完全没有关系的模型。



## 13. 为什么车辆控制通常需要 Magic Formula？

因为车辆极限工况：

* 正常行驶

$\alpha\approx 0$

可以使用线性模型。

* 激烈转向

$\alpha\uparrow$

开始进入非线性。

* 极限制动/转向

轮胎进入饱和：$F_y\rightarrow F_{y,max}$

这时线性模型失效。

因此：

```text
普通控制
   ↓
线性模型可能够用

极限操稳
   ↓
非线性轮胎模型

ABS / ESC / Torque Vectoring
   ↓
Magic Formula 非常常见
```



## 14. 为什么 D 不是简单固定的？

这是汽车项目里非常重要的一点。

理论上：$D=\mu F_z$

但是实际：$\mu\neq constant$

例如：

```text
Fz
↑
│
│       实际 μ
│      /
│    /
│  /
│ /
└────────────→
```

轮胎存在**载荷敏感性**。

也就是：两个轮胎承受的总载荷一样，不代表平均摩擦系数一样。

例如：

左：$F_{z1}=3000N$

右：$F_{z2}=5000N$

并不一定满足：$\frac{F_{x1}}{3000} = \frac{F_{x2}}{5000}$

这也是车辆横向载荷转移非常重要的原因。



## 15. 代码里加入了一个简单的载荷敏感模型

代码中：$\mu_x= \mu_{x0} \left( \frac{F_z}{F_{z0}} \right)^{n_x}$$\mu_y= \mu_{y0} \left( \frac{F_z}{F_{z0}} \right)^{n_y}$

其中：

```matlab
p.muLoadExpX = -0.08;
p.muLoadExpY = -0.10;
```

负指数意味着：随着垂向载荷增加，单位载荷对应的附着系数略微下降。

这是一个**教学/算法开发用的简化模型**，不是某款真实轮胎的参数。



## 16. 联合滑移才是 Magic Formula 真正复杂的地方

例如汽车：一边刹车，一边转弯。

这时候：$\kappa\neq0$

同时：$\alpha\neq0$

那么：$F_x\neq F_x(\kappa)$

而应该是：$\boxed{ F_x=f(\kappa,\alpha,F_z) }$

同时：$\boxed{ F_y=f(\kappa,\alpha,F_z) }$



## 17. 为什么会出现“摩擦圆”？

简单模型可以理解成：

$\left( \frac{F_x}{\mu F_z} \right)^2+ \left( \frac{F_y}{\mu F_z} \right)^2 \leq1$

也就是：

```text
             Fy
              ↑
              │
          ●●●●●
       ●●       ●●
     ●             ●
    ●       ○       ●
     ●             ●
       ●●       ●●
          ●●●●●
              │
              └────────→ Fx
```

所以：

如果你已经用了大量纵向附着：$F_x\rightarrow\mu F_z$

那么剩余的：$F_y$ 就会下降。

这就是：**制动越狠，转向能力越差。**

也是 ABS/ESC 控制的重要物理基础。



## 18. 但要注意：真正的 Pacejka 联合滑移不是简单摩擦圆

工程中故意做了一个：**纯 MF + 简化 combined-slip coupling**

而没有伪装成完整 MF5.2/MF6.1。

真正工业级 Pacejka 模型还会出现：

- $G_{x\alpha}$
- $G_{y\kappa}$
- $S_{Hx}$
- $S_{Hy}$
- $S_{Vx}$
- $S_{Vy}$
- camber
- load dependency
- pressure
- turn slip
- combined slip coefficients

等等。

所以生产项目不能直接拿这套示例参数当真实轮胎模型。



## 19. Simulink 模型结构

工程生成以后：

```text
              Kappa
                │
                │
              ┌─▼──────────┐
Alpha_rad ───►│            │
              │   Magic    │
Fz_N ────────►│  Formula   │
              │            │
              └─┬──┬──┬──┬─┘
                │  │  │  │
                ▼  ▼  ▼  ▼
               Fx Fy μx μy
```

模型接口：

* 输入

```text
Kappa
Alpha_rad
Fz_N
```

* 输出

```text
Fx_N
Fy_N
MuX
MuY
```



## 20. 生成模型

在 MATLAB R2025b 当前目录进入该文件夹。

执行：

```matlab
build_TireMagicFormula_Simulink
```

它会自动生成：TireMagicFormula_R2025b.slx

核心 MATLAB Function Block 就是：

```text
kappa
alpha
Fz
   │
   ▼
Magic Formula
   │
   ├── Fx
   ├── Fy
   ├── MuX
   └── MuY
```



## 21. 直接运行曲线

运行：

```matlab
run_MF_Tire_Demo
```

会得到：

### 21.1 纵向力－滑移率

$F_x-\kappa$

### 21.2 横向力－侧偏角

$F_y-\alpha$

### 21.3 联合滑移

$F_x(\kappa,\alpha)$

以及：

$F_y(\kappa,\alpha)$



## 22. 参数怎么从实验数据得到？

这个对做汽车控制开发其实比公式本身更重要。

假设轮胎台架得到：

```text
Fz
κ
Fx
```

数据：

```text
κ       Fx
0       0
0.01    500
0.02    1050
0.04    1900
0.06    2500
0.10    2700
0.20    2400
```

就要拟合：$B,C,D,E$

使 $F_{x,model}(\kappa)$ 尽可能接近 $F_{x,test}(\kappa)$

可以使用 MATLAB：lsqcurvefit，或者：fmincon进行参数辨识。



## 23. 参数辨识的基本流程

实际可以建立：

```text
轮胎试验数据
     │
     ├── Fz
     ├── κ
     ├── α
     ├── Fx
     └── Fy
     │
     ▼
数据预处理
     │
     ├── 去异常点
     ├── 滤波
     ├── 单位统一
     └── 坐标系统一
     │
     ▼
初始参数
 B C D E
     │
     ▼
非线性最小二乘
     │
     ▼
Magic Formula
     │
     ▼
误差
     │
     └──────→ 继续优化
```

最终得到：$\theta= [B,C,D,E,S_h,S_v,\cdots]$



## 24. 工程内容

| 文件                                | 用途                |
| ----------------------------------- | ------------------- |
| `MF_TireParams.m`                   | Magic Formula 参数  |
| `magicFormulaPure.m`                | 基础 MF             |
| `tireMagicFormula.m`                | 纵向+横向+联合滑移  |
| `run_MF_Tire_Demo.m`                | 曲线演示            |
| `build_TireMagicFormula_Simulink.m` | **自动生成 `.slx`** |
| `TireMagicFormula_Validation.m`     | 基本模型验证        |
| `README_R2025b.txt`                 | 使用说明            |
