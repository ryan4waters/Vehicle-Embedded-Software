#include "powerscope.h"

/* 这些变量实际项目中来自PFC/CLLC/DCDC控制软件 */
volatile float g_Vin;
volatile float g_Vout;
volatile float g_Iin;
volatile float g_Iout;
volatile float g_Duty;
volatile float g_VloopErr;
volatile float g_IloopErr;
volatile float g_PwmFreq;
volatile float g_Temperature;

void PowerScope_AppInit(void)
{
    PowerScope_Init(50u); /* 示例：50us */

    PowerScope_AddChannel(0, &(PS_ChannelConfig){
        .name="Vin", .type=PS_TYPE_FLOAT32, .scale=1.0f, .offset=0.0f,
        .source=&g_Vin, .enabled=true});

    PowerScope_AddChannel(1, &(PS_ChannelConfig){
        .name="Vout", .type=PS_TYPE_FLOAT32, .scale=1.0f, .offset=0.0f,
        .source=&g_Vout, .enabled=true});

    PowerScope_AddChannel(2, &(PS_ChannelConfig){
        .name="Iin", .type=PS_TYPE_FLOAT32, .source=&g_Iin, .enabled=true});

    PowerScope_AddChannel(3, &(PS_ChannelConfig){
        .name="Iout", .type=PS_TYPE_FLOAT32, .source=&g_Iout, .enabled=true});

    PowerScope_AddChannel(4, &(PS_ChannelConfig){
        .name="Duty", .type=PS_TYPE_FLOAT32, .source=&g_Duty, .enabled=true});

    PowerScope_AddChannel(5, &(PS_ChannelConfig){
        .name="VloopErr", .type=PS_TYPE_FLOAT32, .source=&g_VloopErr, .enabled=true});

    PowerScope_AddChannel(6, &(PS_ChannelConfig){
        .name="IloopErr", .type=PS_TYPE_FLOAT32, .source=&g_IloopErr, .enabled=true});

    PowerScope_AddChannel(7, &(PS_ChannelConfig){
        .name="PwmFreq", .type=PS_TYPE_FLOAT32, .source=&g_PwmFreq, .enabled=true});

    PowerScope_AddChannel(8, &(PS_ChannelConfig){
        .name="Temp", .type=PS_TYPE_FLOAT32, .source=&g_Temperature, .enabled=true});

    PS_TriggerConfig trig = {
        .channel = 1,
        .mode = PS_TRIG_RISING,
        .level = 400.0f,
        .pre_samples = 500,
        .post_samples = 1000,
        .single_shot = true
    };
    PowerScope_ConfigTrigger(&trig);
}

/* 推荐在ADC同步ISR中调用，而不是普通while(1) */
void PowerScope_ADC_ISR_Hook(void)
{
    PowerScope_SampleISR();
}

/* 10ms/20ms后台任务 */
void PowerScope_BackgroundTask(void)
{
    PowerScope_Task();

    if (PowerScope_IsCaptured()) {
        static uint8_t frame[PS_MAX_SAMPLES * PS_MAX_CHANNELS * 4u + 32u];
        uint16_t len = PowerScope_BuildFrame(frame, sizeof(frame));
        if (len) {
            /* 实际工程这里调用CAN-FD/UART/Ethernet */
            /* PowerScope_Platform_Send(frame, len); */
            PowerScope_Stop();
        }
    }
}
