**TC377 + AUTOSAR Classic**，理解 HSM 最关键的一点是：**HSM 不是 AUTOSAR 的一个普通 BSW 软件模块，而是“硬件安全执行环境”；AUTOSAR 的 CSM / CryIf / Crypto Driver 是把应用层的密码学需求接到 HSM 上的软件链路。**

TC37x 本身带有独立的 HSM 子系统；Infineon 文档明确描述它是一个独立处理器子系统，并通过 SPB 总线与系统连接。TC3xx HSM 还提供 AES-128、TRNG，以及 TC3xx HSM 上的 ECC-256、SHA-256 等硬件加速能力。([Infineon Technologies](https://www.infineon.com/dgdl/Infineon-AURIX_TC37x-UserManual-v02_00-EN.pdf?fileId=5546d4627506bb32017530759f185ed8&utm_source=chatgpt.com))

下面按照车载 ECU 软件开发的方式来讲，不只讲概念，而是把它落到 **TC377 + AUTOSAR + C代码 + SecOC/安全诊断/安全启动**。



# 1. 先建立一个完整认识

先把整个体系理解成：

```text
                 Application / SW-C
                        │
             ┌──────────┴──────────┐
             │                     │
          SecOC                  Diag
             │                     │
             └──────────┬──────────┘
                        │
                       CSM
             Crypto Service Manager
                        │
                       CryIf
                Crypto Interface
                        │
              Crypto Driver
          ┌─────────────┴─────────────┐
          │                           │
     SW Crypto                   HSM Driver
                                      │
                                HSM Interface
                                      │
                         ┌────────────┴────────────┐
                         │                         │
                    TC377 HSM CPU             HSM Crypto HW
                         │                 AES / SHA / ECC / TRNG
                         │
                  Secure Key Storage
```

AUTOSAR 对 CSM 的定位非常明确：**CSM 为 SW-C/BSW 提供密码服务，并通过 CryIf 访问一个或多个 Crypto Driver；Crypto Driver 最终可以依赖软件密码库或者硬件安全模块。** ([AUTOSAR](https://www.autosar.org/fileadmin/standards/R24-11/CP/AUTOSAR_CP_SWS_CryptoServiceManager.pdf?utm_source=chatgpt.com))

所以：**CSM ≠ HSM**、**CryIf ≠ HSM**、**Crypto Driver ≠ HSM**

而是：**CSM → CryIf → Crypto Driver → HSM**



# 2. HSM到底解决什么问题？

先不要从 AUTOSAR 看，从攻击场景看。

假设 ECU 里面有：

```text
Master Key
AES Key
MAC Key
ECC Private Key
Boot Key
SecOC Key
```

如果这些 Key 放在普通：

```text
Application RAM
Application PFlash
```

那么理论上应用软件如果存在漏洞，就可能：

```text
Application
     │
     ├── Read Key
     │
     ├── Copy Key
     │
     └── Use Key
```

这是很危险的。

HSM 的目的就是把：

```text
普通CPU世界
─────────────────────────
Application
AUTOSAR
BSW
OS
Drivers

          │
          │ Firewall / Protection
          ▼

安全世界
─────────────────────────
HSM CPU
Secure RAM
Secure Flash
Crypto Accelerator
TRNG
Key Storage
```

隔离开。

Infineon 对 AURIX HSM 的描述也是一个独立的安全计算平台/信任锚，通过独立保护域实现隔离，并提供受保护的密钥存储。([Infineon Technologies](https://www.infineon.com/product-information/aurix-security-solutions?utm_source=chatgpt.com))



# 3. TC377里的HSM是什么？

以 **TC377** 为例。

可以粗略理解成：

```text
                         TC377
┌──────────────────────────────────────────────┐
│                                              │
│          TriCore Host Domain                 │
│                                              │
│   CPU0 / CPU1 / CPU2                         │
│        │                                     │
│        ├── AUTOSAR OS                        │
│        ├── RTE                               │
│        ├── SWC                               │
│        ├── BSW                               │
│        ├── MCAL                              │
│        └── Crypto Driver Interface           │
│                       │                      │
│                       │                      │
│                  HSM Interface               │
│                       │                      │
├───────────────────────┼──────────────────────┤
│                       ▼                      │
│                                              │
│                HSM Domain                    │
│                                              │
│             HSM CPU / Firmware               │
│                       │                      │
│          ┌────────────┼────────────┐         │
│          │            │            │         │
│         AES          SHA          ECC        │
│          │            │            │         │
│          └────────────┼────────────┘         │
│                       │                      │
│                     TRNG                     │
│                       │                      │
│                 Secure Storage               │
│                                              │
└──────────────────────────────────────────────┘
```

TC37x User Manual 对 HSM 的定义就是独立的 processor subsystem dedicated for security tasks。([Infineon Technologies](https://www.infineon.com/dgdl/Infineon-AURIX_TC37x-UserManual-v02_00-EN.pdf?fileId=5546d4627506bb32017530759f185ed8&utm_source=chatgpt.com))



# 4. 为什么HSM需要自己的CPU？

这是理解 HSM 最关键的地方之一。

普通 CPU：

```c
uint8 key[16];
```

理论上：

```text
CPU
 ↓
RAM
 ↓
Key
```

而 HSM：

```text
Host CPU
    │
    │ Request:
    │ "用Key 3计算CMAC"
    ▼
HSM
    │
    ├── 找到 Key 3
    │
    ├── Key不暴露给Host
    │
    ├── AES/CMAC
    │
    └── 返回MAC
```

Host只得到：

```text
MAC = 0x123456...
```

而不是：

```text
Key = 0xABCDEF...
```

所以 HSM 最重要的原则可以记成：**Host使用Key，但Host不应该直接拥有Key。**

这也是为什么安全 ECU 中经常强调：Key never leaves secure domain



# 5. AUTOSAR为什么又搞出CSM？

因为 AUTOSAR 不希望应用程序知道底层到底是什么硬件。

例如：

```c
SecOC
  ↓
CSM
```

SecOC只关心：

> “我要计算 CMAC。”

它不应该关心：

```text
TC377？
HSM？
AES accelerator？
Software AES？
另一个外部HSM？
```

所以：

```text
Application
     ↓
    CSM
     ↓
   CryIf
     ↓
Crypto Driver
     ↓
  ┌───────┬─────────┐
  ↓       ↓         ↓
 SW AES  TC377 HSM  External HSM
```

这就是 AUTOSAR Crypto Stack 的价值。



# 6. CSM、CryIf、Crypto Driver到底分别干什么？

这是实际项目最容易混淆的地方。

## 6.1 CSM

CSM：**Crypto Service Manager**

主要负责：

```text
密码服务管理
Job
Queue
Priority
Primitive
Key Reference
```

例如：

```c
Csm_MacGenerate(...)
```

或者：

```c
Csm_MacVerify(...)
```

CSM是应用/BSW比较靠近的一层。

AUTOSAR规定 CSM 可以维护多个队列，并按照优先级调度 Crypto Job，再通过 CryIf 把 Job 转发到具体 Crypto Driver。([AUTOSAR](https://www.autosar.org/fileadmin/standards/R24-11/CP/AUTOSAR_CP_SWS_CryptoServiceManager.pdf?utm_source=chatgpt.com))



## 6.2 CryIf

CryIf：**Crypto Interface**

它相当于：

```text
CSM
 │
 ▼
CryIf
 │
 ├── Crypto Driver 0
 │
 ├── Crypto Driver 1
 │
 └── Crypto Driver 2
```

比如：

```text
Crypto Driver 0
→ TC377 HSM

Crypto Driver 1
→ Software Crypto

Crypto Driver 2
→ External HSM
```

CryIf负责把：

```text
CsmKeyId
```

映射到：

```text
CryptoDriverKeyId
```

AUTOSAR规范也明确规定 CryIf负责把 CSM 使用的 Key ID 转换成对应 Crypto Driver 的 Key ID。([Scribd](https://www.scribd.com/document/903904373/AUTOSAR-CP-SWS-CryptoServiceManager?utm_source=chatgpt.com))



## 6.3 Crypto Driver

Crypto Driver才真正负责：

```text
AES
CMAC
SHA
ECC
Random
Key management
```

它可以：

```text
Software
```

也可以：

```text
Hardware
```

所以：

```text
Crypto Driver
       │
       ├── SW AES
       │
       ├── TC377 HSM
       │
       └── HW Accelerator
```

这也是 AUTOSAR Crypto Driver specification 中的核心设计思想。([AUTOSAR](https://www.autosar.org/fileadmin/standards/R23-11/CP/AUTOSAR_CP_SWS_CryptoDriver.pdf?utm_source=chatgpt.com))



# 7. TC377项目里的真实链路

如果做的是：**TC377 + AUTOSAR + HSM + SecOC**

那么非常典型的结构就是：

```text
SecOC
 │
 │ Csm_MacGenerate()
 ▼
CSM
 │
 │ Crypto Job
 ▼
CryIf
 │
 ▼
Crypto Driver
 │
 ▼
TC377 HSM Driver
 │
 │ IPC / shared memory / interrupt
 ▼
HSM Firmware
 │
 ├── Key
 ├── AES
 ├── CMAC
 └── TRNG
 │
 ▼
MAC
 │
 ▼
Crypto Driver
 │
 ▼
CryIf
 │
 ▼
CSM
 │
 ▼
SecOC
```



# 8. 举一个最实际的例子：CAN SecOC

假设有一个 CAN 报文：

```text
CAN ID = 0x180

Payload:

Byte0 = VehicleSpeed
Byte1 = SteeringAngle
Byte2 = ...
Byte3 = ...
```

现在要做 SecOC。

原始：

```text
CAN Frame
│
├── Data
└── Counter
```

需要计算：

```text
MAC = CMAC(Key, Data + Counter + DataID)
```

最终：

```text
CAN Frame

| Data | Counter | Authenticator |
```



# 9. 应用层不会直接调用HSM

错误的做法：

```c
Hsm_AES_CMAC(
    key,
    data,
    len,
    mac
);
```

这会把 HSM 的具体实现暴露给应用。

AUTOSAR应该是：

```c
Csm_MacGenerate(...)
```

例如概念代码：

```c
uint8 data[16];
uint8 mac[16];

Csm_MacGenerate(
    CSM_MAC_JOB_ID,
    data,
    sizeof(data),
    mac
);
```

然后：

```text
Csm
 ↓
CryIf
 ↓
Crypto Driver
 ↓
HSM
```



# 10. 一个更接近AUTOSAR的Job结构

实际工程里，通常不会只是简单：

```c
Csm_MacGenerate(data, len, mac);
```

而是：

```c
Crypto_JobType job;

job.jobId = 10;

job.jobPrimitiveInfo =
    &CsmJobPrimitiveInfo;

job.jobPrimitiveInputOutput =
    &JobInputOutput;

job.jobPrimitiveInputOutput->inputPtr = data;

job.jobPrimitiveInputOutput->inputLength =
    dataLen;

job.jobPrimitiveInputOutput->outputPtr =
    mac;

job.jobPrimitiveInputOutput->outputLengthPtr =
    &macLen;

CryIf_ProcessJob(
    channelId,
    &job
);
```

实际结构名称和字段会根据供应商 AUTOSAR Stack 版本有所差异，但思想就是这个结构。

AUTOSAR CSM 的 API 最终会形成 `Crypto_JobType`，其中携带 primitive、输入输出、Key reference 等信息，再交给 CryIf。([Scribd](https://www.scribd.com/document/903904373/AUTOSAR-CP-SWS-CryptoServiceManager?utm_source=chatgpt.com))



# 11. 为什么这里经常出现异步？

这是 HSM工程中特别重要的问题。

假设：

```text
10 ms Task
```

调用：

```c
Csm_MacGenerate()
```

并不是一定：

```text
调用
 ↓
等待HSM
 ↓
返回
```

更常见的是：

```text
Application
    │
    │ Request
    ▼
CSM
    │
    ▼
CryIf
    │
    ▼
Crypto Driver
    │
    ▼
HSM
    │
    │ processing
    │
    ▼
Interrupt / callback
    │
    ▼
Crypto Driver
    │
    ▼
CSM
    │
    ▼
Callback
```

所以：

```c
Csm_MacGenerate();
```

可能只是**提交Job**，而不是**MAC已经计算完成**



# 12. 为什么汽车项目特别适合异步？

因为：

```text
HSM计算
```

可能需要时间。

如果：

```text
10ms task
```

里面：

```c
Csm_MacGenerate();

while(!done)
{
}
```

那就非常糟糕。

因为：

```text
CPU
│
├── Wait HSM
├── Wait HSM
├── Wait HSM
└── ...
```

反而失去了 AUTOSAR OS 的调度能力。

更合理：

```text
t0:
提交Job

t1:
继续执行其他任务

t2:
HSM完成

t3:
Callback

t4:
获取MAC
```



# 13. HSM Job Queue是什么？

假设同时有：

```text
SecOC CAN1
SecOC CAN2
Diag Authentication
Secure Boot
Key update
```

全部需要：

```text
CMAC
```

就会出现：

```text
               ┌── CAN1 MAC
               │
               ├── CAN2 MAC
CSM Queue ─────┼── Diag MAC
               │
               └── Other MAC
                       │
                       ▼
                    CryIf
                       │
                       ▼
                     HSM
```

CSM可以配置：

```text
Priority
Queue depth
Channel
Crypto Driver Object
```

AUTOSAR也明确把从 CSM Queue 经 CryIf 到 Crypto Driver Object 的路径定义为一个 channel。([AUTOSAR](https://www.autosar.org/fileadmin/standards/R24-11/CP/AUTOSAR_CP_SWS_CryptoServiceManager.pdf?utm_source=chatgpt.com))



# 14. Crypto Driver Object又是什么？

这是 AUTOSAR Crypto 非常容易让人迷糊的概念。

可以理解：

```text
Crypto Driver
│
├── Crypto Driver Object 0
│       └── AES/CMAC
│
├── Crypto Driver Object 1
│       └── SHA
│
└── Crypto Driver Object 2
        └── ECC
```

一个 Driver Object可以对应一个硬件加速器，或者一个独立的crypto执行资源

AUTOSAR定义中，一个 Crypto Driver 可以包含多个独立的 Crypto Driver Object，每个 Object 有自己的 workspace，并可以独立执行 crypto primitive。([AUTOSAR](https://www.autosar.org/fileadmin/standards/R20-11/CP/AUTOSAR_SWS_CryptoServiceManager.pdf?utm_source=chatgpt.com))



# 15. 再进入TC377：HSM里面到底有什么？

对于 TC3xx HSM，Infineon公开资料列出了：

```text
HSM CPU
AES-128
TRNG
ECC-256
SHA-256
Secure storage
```

其中 ECC-256 和 SHA2-256 是 TC3xx HSM 的能力。([Infineon Technologies](https://www.infineon.com/product-information/aurix-security-solutions?utm_source=chatgpt.com))

可以抽象成：

```text
             TC377 HSM
                 │
      ┌──────────┼──────────┐
      │          │          │
     AES        SHA        ECC
      │          │          │
      └──────────┼──────────┘
                 │
                TRNG
                 │
          Secure Key Storage
```



## 15.1 AES有什么用？

例如：

```text
AES-128
```

可以用于：

```text
AES Encrypt
AES Decrypt
CMAC
Key derivation
SecOC
Secure Boot
Secure update
```

Infineon HSM training资料中列出了 AES-128，以及 ECB/CBC/CTR/OFB/CFB 等模式，并说明 TRNG 可用于密钥和 challenge 等随机数场景。([Infineon Technologies](https://www.infineon.com/assets/row/public/documents/10/56/infineon-aurix-tc3xx-hardware-security-module-quick-training-en.pdf?fileId=5546d46274cf54d50174da4ebc3f2265&utm_source=chatgpt.com))



## 15.2 SHA有什么用？

例如：

```text
Firmware Image
```

计算：

```text
SHA256(Image)
```

得到：

```text
Hash
```

用于：

```text
Secure Boot
Secure Update
Firmware Verification
Certificate
Signature Verification
```



## 15.3 ECC有什么用？

ECC主要解决：

```text
非对称密码
```

例如：

```text
Private Key
Public Key
```

典型：

```text
ECDSA
```

用于：

```text
Firmware Signature
Certificate
Authentication
Secure Boot
```

例如：

```text
OEM Private Key
       │
       ▼
签名 Firmware
       │
       ▼
ECU
       │
       ▼
HSM
       │
       ├── Public Key
       │
       ├── SHA256
       │
       └── ECDSA Verify
       │
       ▼
PASS / FAIL
```



## 15.4 TRNG为什么重要？

安全系统不能随便：

```c
rand();
```

产生密钥。

例如：

```text
Session Key
Nonce
Challenge
IV
Random Seed
```

都需要可靠随机数。

TC3xx HSM具有 TRNG，并且 Infineon将其描述为符合 AIS31 的真随机数源。([Infineon Technologies](https://www.infineon.com/product-information/aurix-security-solutions?utm_source=chatgpt.com))



## 15.5 最重要的：Key到底放在哪里？

这是HSM项目真正的核心。

普通方案：

```text
uint8 AES_KEY[16];

AES_KEY =
{
    0x12,
    0x34,
    ...
};
```

非常不推荐。

HSM方案：

```text
             Key ID = 5
Application ────────────────┐
                            ▼
                         CSM
                            │
                         CryIf
                            │
                     Crypto Driver
                            │
                            ▼
                          HSM
                            │
                    ┌───────┴───────┐
                    │               │
                  Key 5            AES
                    │               │
                    └───────┬───────┘
                            ▼
                           MAC
```

Application只知道：

```c
KeyId = 5;
```

不知道：

```text
Key material
```



## 15.6 Key ID不等于Key本身

比如：

```c
#define SEC_OC_KEY_ID  0x1001
```

这个：

```text
0x1001
```

不是：

```text
AES key = 12 34 56 ...
```

而是：

> **Key Handle / Key Reference**

类似：

```text
Key ID
  ↓
Crypto Driver
  ↓
HSM Key Slot
  ↓
Actual Key
```

因此：

```c
Csm_MacGenerate(
    keyId,
    data,
    ...
);
```

并不意味着：

```c
Csm_MacGenerate(
    16-byte-secret-key,
    ...
);
```



## 15.7 AUTOSAR Key层级

可以这样理解：

```text
CSM
 │
 └── Key ID
       │
       ▼
CryIf
 │
 └── Crypto Driver Key ID
       │
       ▼
Crypto Driver
 │
 └── HSM Key Slot
       │
       ▼
     Key Element
       │
       ▼
   Actual Key Material
```

AUTOSAR还区分：

```text
Key
Key Type
Key Element
```

例如：

```text
Key
└── Key Type
      ├── KEY_VALUE
      ├── IV
      ├── MAC
      └── ...
```



## 15.8 KeyM又是什么？

如果继续深入 AUTOSAR Security，会遇到：KeyM

也就是：Key Manager

可以粗略理解：

```text
            KeyM
             │
      Key Provisioning
      Key Update
      Certificate
             │
             ▼
            CSM
             │
            CryIf
             │
        Crypto Driver
             │
             ▼
            HSM
```

因此：CSM解决怎么使用密码服务？

而：KeyM更偏怎么管理密钥和证书生命周期？



# 16. HSM和SecOC的关系

做 AUTOSAR CAN Security，很容易看到：

```text
SecOC
```

实际上：

```text
SecOC
   │
   │ Authentication
   ▼
 CSM
   │
   ▼
CryIf
   │
   ▼
Crypto Driver
   │
   ▼
 HSM
```

例如：

```text
CAN Data
+
Freshness Counter
+
Data ID
        │
        ▼
      CMAC
        │
        ▼
Authenticator
```

所以 SecOC本身并不是：

```text
自己实现 AES
```

而是：

```text
SecOC → CSM → CryIf → Crypto Driver → HSM
```



# 17. 一个简化版C代码

为了真正建立代码概念，在此提供一个**非供应商绑定**的 AUTOSAR 风格代码。

```c
// SecOC侧
#define SEC_OC_KEY_ID      0x1001u
#define SEC_OC_JOB_ID      0x01u

Std_ReturnType SecOC_CalculateAuthenticator(
    const uint8* data,
    uint32 dataLen,
    uint8* mac,
    uint32* macLen)
{
    return Csm_MacGenerate(
        SEC_OC_JOB_ID,
        data,
        dataLen,
        mac,
        macLen
    );
}
```

应用层只知道：

```text
JOB ID
KEY ID
Data
MAC
```

不知道 HSM。



## 17.1 CSM内部

概念上：

```c
Std_ReturnType Csm_MacGenerate(
    uint32 jobId,
    const uint8* data,
    uint32 dataLen,
    uint8* mac,
    uint32* macLen)
{
    Crypto_JobType job;

    job.jobId = jobId;

    job.jobPrimitiveInputOutput.inputPtr =
        data;

    job.jobPrimitiveInputOutput.inputLength =
        dataLen;

    job.jobPrimitiveInputOutput.outputPtr =
        mac;

    job.jobPrimitiveInputOutput.outputLengthPtr =
        macLen;

    job.jobPrimitiveInputOutput.cryIfKeyId =
        SEC_OC_KEY_ID;

    return CryIf_ProcessJob(
        CSM_CHANNEL_0,
        &job
    );
}
```

实际供应商生成代码的结构会比这里复杂很多。



## 17.2 CryIf

概念：

```c
Std_ReturnType CryIf_ProcessJob(
    uint32 channel,
    Crypto_JobType* job)
{
    uint32 cryptoKeyId;

    cryptoKeyId =
        CryIf_MapKey(
            job->jobPrimitiveInputOutput.cryIfKeyId
        );

    job->jobPrimitiveInputOutput.cryIfKeyId =
        cryptoKeyId;

    return Crypto_ProcessJob(
        channel,
        job
    );
}
```

这里体现一个很重要的事情：

```text
CSM Key ID
      ↓
CryIf
      ↓
Crypto Driver Key ID
```



## 17.3 Crypto Driver

概念：

```c
Std_ReturnType Crypto_ProcessJob(
    uint32 objectId,
    Crypto_JobType* job)
{
    switch(job->jobPrimitiveInfo->primitiveService)
    {
        case CRYPTO_MACGENERATE:

            return Hsm_MacGenerate(
                job
            );

        default:

            return E_NOT_OK;
    }
}
```

这里才开始接触：

```text
HSM Driver
```



## 17.4 HSM Driver

然后：

```c
Std_ReturnType Hsm_MacGenerate(
    Crypto_JobType* job)
{
    Hsm_RequestType request;

    request.command =
        HSM_CMD_MAC_GENERATE;

    request.keyId =
        job->jobPrimitiveInputOutput.cryIfKeyId;

    request.input =
        job->jobPrimitiveInputOutput.inputPtr;

    request.inputLength =
        job->jobPrimitiveInputOutput.inputLength;

    request.output =
        job->jobPrimitiveInputOutput.outputPtr;

    return Hsm_SendRequest(
        &request
    );
}
```

注意这里：

```c
request.keyId
```

依然只是：

```text
Key Handle
```

而不是：

```text
Actual AES Key
```



## 17.5 HSM内部

HSM侧可以抽象成：

```c
void Hsm_Main(void)
{
    Hsm_RequestType request;

    if(Hsm_GetRequest(&request) == TRUE)
    {
        switch(request.command)
        {
            case HSM_CMD_MAC_GENERATE:

                Hsm_CmacGenerate(
                    request.keyId,
                    request.input,
                    request.inputLength,
                    request.output
                );

                break;

            default:
                break;
        }
    }
}
```

真正项目中：

```text
HSM Main
```

可能是：

```text
HSM firmware
+
interrupt
+
mailbox
+
shared memory
+
hardware crypto accelerator
```

而不是这么简单。



# 18. Host和HSM怎么通信？

可以抽象成：

```text
Host CPU
   │
   │ Request
   ▼
Shared Memory / Mailbox
   │
   ▼
HSM
   │
   │ Result
   ▼
Shared Memory / Mailbox
   │
   ▼
Host CPU
```

然后：

```text
HSM完成
    │
    ▼
Interrupt
    │
    ▼
Crypto Driver
    │
    ▼
Callback
```

这就是 **TC377中断分类、CPU占用、DMA** 等知识在安全栈中的一个交汇点。



# 19. 这里和中断问题有直接关系

假设：

```text
HSM完成
```

触发：

```text
HSM interrupt
```

那么 Host CPU可能：

```text
CPU
│
├── Task
│
├── CAN ISR
│
├── ADC ISR
│
├── DMA ISR
│
├── HSM ISR
│
└── Task
```

所以调试：

```text
SecOC MAC
```

的时候，如果发现：

```c
Csm_MacGenerate()
```

返回：

```c
E_OK
```

但 callback很晚才来，

不要马上怀疑：

```text
AES计算慢
```

还应该检查：

```text
HSM queue
Crypto Driver queue
OS scheduling
HSM interrupt priority
CPU interrupt load
```



# 20. HSM和普通Crypto Accelerator有什么区别？

这个非常重要。

比如：

```text
AES Accelerator
```

只是：

```text
硬件加速器
```

而 HSM：

```text
HSM
│
├── CPU
├── Secure Memory
├── Key Protection
├── AES
├── SHA
├── ECC
├── TRNG
├── Secure Boot support
└── Security Isolation
```

所以：**AES硬件 ≠ HSM。**

HSM是完整的安全执行环境。



# 21. TC377 HSM还有一个非常关键的东西：Secure Boot

最终一定会遇到：

```text
Secure Boot
```

启动过程可以理解：

```text
Power ON
   │
   ▼
Reset
   │
   ▼
Boot ROM / SSW
   │
   ▼
HSM初始化
   │
   ▼
验证Application
   │
   ├── FAIL → 不允许启动
   │
   └── PASS
         │
         ▼
      Application
```

验证可能涉及：

```text
Hash
+
Signature
+
Public Key
```

例如：

```text
Firmware
   │
   ▼
SHA256
   │
   ▼
Hash
   │
   ▼
ECDSA Verify
   │
   ▼
PASS / FAIL
```

TC377 HSM 的安全能力正是用于这类汽车安全场景；Infineon也把 secure boot、secure onboard communication 等列为典型 HSM 应用。([Infineon Technologies](https://www.infineon.com/product-information/aurix-security-solutions?utm_source=chatgpt.com))



# 22. Secure Update

然后就是：

```text
OTA
```

例如：

```text
OEM Server
     │
     ▼
Signed Firmware
     │
     ▼
Vehicle
     │
     ▼
Bootloader
     │
     ▼
HSM
     │
     ├── Hash
     ├── Signature Verify
     └── Key verification
     │
     ▼
Firmware Valid
```

如果：

```text
Signature FAIL
```

则：

```text
Reject
```



# 23. Debug为什么也会被HSM影响？

HSM并不只是加密算法

还涉及：

```text
Debug protection
Flash protection
Secure boot
Key protection
```

TC3xx中 HSM相关配置与 UCB 等安全配置有关；公开资料也说明 HSM 的启用/保护配置涉及 User Configuration Block。([iSystem](https://www.isystem.com/downloads/winIDEA/help/tc3-hsm-programming.html?utm_source=chatgpt.com))

所以开发阶段经常出现：

```text
HSM打开
↓
Debugger突然不能正常访问
↓
Flash无法正常下载
↓
芯片被安全配置锁住
```

这是 HSM 项目非常现实的风险。



# 24. 所以开发阶段一定要区分三种状态

建议 TC377 项目这样管理：

```text
Development
│
├── HSM Disabled
│
├── HSM Development Mode
│
└── HSM Production Mode
```

尤其是：

```text
UCB
HSMCFG
Flash protection
Debug protection
Secure boot
```

不要一开始就全部打开。

否则刷死，会比普通 AUTOSAR ECU 麻烦很多。



# 25. HSM和功能安全是什么关系？

做 OBC / DCDC 的时候尤其要区分：

```text
Functional Safety
```

和：

```text
Cyber Security
```

例如：

### 功能安全

```text
ADC异常
CAN Timeout
OV
OC
OT
CPU Fault
RAM ECC
```

主要解决：ECU出现随机故障以后，如何进入安全状态。



### Cyber Security

```text
CAN Spoofing
Firmware Tampering
Key Theft
Replay Attack
Unauthorized Diagnostic
```

主要解决：恶意攻击者如何不能伪造、篡改、窃取。

HSM主要属于：Cyber Security

但是在实际 ECU 中，两者最终需要协同。



# 26. 对TC377项目，建议的软件架构

如果准备真正把 HSM 做成一个 ECU Security Package，建议：

```text
Security/
│
├── SecOC/
│   ├── SecOC_Cfg.c
│   ├── SecOC.c
│   └── SecOC.h
│
├── CSM/
│   ├── Csm_Cfg.c
│   ├── Csm.c
│   └── Csm.h
│
├── CryIf/
│   ├── CryIf_Cfg.c
│   ├── CryIf.c
│   └── CryIf.h
│
├── Crypto/
│   ├── Crypto.c
│   ├── Crypto.h
│   └── Crypto_Cfg.c
│
├── HSM/
│   ├── Hsm.c
│   ├── Hsm.h
│   ├── Hsm_Ipc.c
│   ├── Hsm_Ipc.h
│   ├── Hsm_Key.c
│   ├── Hsm_Key.h
│   └── Hsm_Cfg.c
│
└── KeyM/
    ├── KeyM.c
    ├── KeyM.h
    └── KeyM_Cfg.c
```



# 27. 再进一步，把芯片层隔离出来

考虑一直强调的：TC377 / F29P32 / SPC58NN 多芯片适配

那么 Security Package 也应该这么做：

```text
Security Service
        │
        ▼
AUTOSAR CSM
        │
        ▼
CryIf
        │
        ▼
Crypto Driver Interface
        │
   ┌────┼─────────┐
   │    │         │
 TC377 F29P32  SPC58NN
   │
   ▼
TC377 HSM
```

例如：

```text
Crypto_Hw.c
```

只提供：

```c
Crypto_Hw_MacGenerate()
Crypto_Hw_MacVerify()
Crypto_Hw_RandomGenerate()
Crypto_Hw_Hash()
Crypto_Hw_SignatureVerify()
```

然后：

```text
TC377
   ↓
TC377_Hsm.c

F29P32
   ↓
F29P32_Hsm.c

SPC58NN
   ↓
SPC58NN_Hsm.c
```

这样上层完全不用知道芯片。



# 28. TC377建议最终形成这张架构图

```text
                       Application
                            │
              ┌─────────────┴─────────────┐
              │                           │
            SecOC                       Diag
              │                           │
              └─────────────┬─────────────┘
                            │
                           CSM
                            │
                  ┌─────────┴─────────┐
                  │                   │
               Csm Job             KeyM
                  │
                  ▼
                 CryIf
                  │
                  ▼
             Crypto Driver
                  │
          ┌───────┴────────┐
          │                │
       SW Crypto       TC377 HSM Driver
                           │
                           │ IPC
                           ▼
                    ┌──────────────┐
                    │ TC377 HSM    │
                    │              │
                    │ HSM CPU      │
                    │ AES          │
                    │ SHA256       │
                    │ ECC256       │
                    │ TRNG         │
                    │ Key Storage  │
                    └──────────────┘
```



# 29. 可以把整个HSM记成四句话

### 第一层：CSM

> **我要做什么密码服务？**

例如：

```text
MAC
Encrypt
Decrypt
Hash
Sign
Verify
Random
```



### 第二层：CryIf

> **这个请求交给哪个 Crypto Driver？**



### 第三层：Crypto Driver

> **具体怎么调用底层密码硬件/软件？**



### 第四层：HSM

> **真正安全地执行密码运算，并保护Key。**



# 30. 最容易搞错的几个概念

| 概念            | 本质                      |
| --------------- | ------------------------- |
| HSM             | 独立安全执行环境          |
| AES Accelerator | 加密硬件加速器            |
| CSM             | AUTOSAR密码服务管理       |
| CryIf           | Crypto Driver路由/接口    |
| Crypto Driver   | 具体密码实现              |
| KeyM            | 密钥/证书管理             |
| SecOC           | 通信报文安全保护          |
| Secure Boot     | 启动阶段完整性/真实性保护 |
| SHE             | 汽车安全硬件规范/接口体系 |
| TRNG            | 真随机数发生器            |

Infineon 的 TC3xx HSM 还提供 SHE/SHE+相关能力；其公开资料说明 SHE+ driver 可以提供 AUTOSAR CRY 接口，并负责与 HSM 安全硬件交互。([Infineon Technologies](https://www.infineon.com/product-information/aurix-security-solutions?utm_source=chatgpt.com))



# 31 一个实际项目的调用链

例如：

> **TC377 收到一帧 CAN → SecOC验证 MAC**

实际逻辑：

```text
CAN Driver
    │
    ▼
CanIf
    │
    ▼
PduR
    │
    ▼
SecOC
    │
    ├── Data
    ├── Freshness Counter
    └── Authenticator
             │
             ▼
        Csm_MacVerify()
             │
             ▼
           CSM
             │
             ▼
          Crypto Job
             │
             ▼
           CryIf
             │
             ▼
       Crypto Driver
             │
             ▼
        TC377 HSM
             │
             ├── Key Slot
             │
             ├── AES
             │
             └── CMAC
             │
             ▼
          MAC Result
             │
             ▼
       CSM Callback
             │
             ▼
           SecOC
             │
       ┌─────┴─────┐
       │           │
     PASS         FAIL
       │           │
    Accept       Reject
```

这张链路真正吃透以后，**AUTOSAR HSM基本就入门了**。

结合 **TC377 + AUTOSAR + CAN + 安全/BSW**，建议不要停留在概念层，而是继续把它落成一个完整工程：

### ① TC377 HSM底层

重点讲：

```text
TC377 HSM CPU
HSM RAM/PFLASH/SFLASH
UCB_HSMCFG
HSM启动
Host ↔ HSM通信
Mailbox / Shared Memory
HSM Interrupt
HSM Driver
```

### ② AUTOSAR Crypto完整配置

直接按照：

```text
CSM
 ↓
CryIf
 ↓
Crypto Driver
 ↓
TC377 HSM
```

拆成：

```text
CryptoKey
CryptoKeyType
CryptoKeyElement
CryptoJob
CryptoPrimitive
CryptoChannel
CryptoDriverObject
Queue
Callback
```

以及 **EB Tresos / DaVinci中每一个配置项究竟对应代码里的什么东西**。

### ③ 最后做一个真正能用于项目的实例

建议直接做：

```text
TC377
+
AUTOSAR Classic
+
CAN
+
SecOC
+
CSM
+
CryIf
+
Crypto Driver
+
HSM
+
AES-CMAC
+
Freshness Counter
```