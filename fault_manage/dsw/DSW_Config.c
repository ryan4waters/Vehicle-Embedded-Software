#include "DSW_Config.h"
#include "DSW_Tree.h"

DSW_NodeTypeDef g_DswRootNode;
DSW_NodeTypeDef g_DswPowerNode;
DSW_NodeTypeDef g_DswCanNode;
DSW_NodeTypeDef g_DswSensorNode;
DSW_NodeTypeDef g_DswImuHswNode;
DSW_NodeTypeDef g_DswAdcHswNode;
DSW_NodeTypeDef g_DswImuAswNode;

DSW_FaultWordType g_FwPower12vUv;
DSW_FaultWordType g_FwImuSpi;
DSW_FaultWordType g_FwImuReset;
DSW_FaultWordType g_FwImuTimeout;
DSW_FaultWordType g_FwImuDataInvalid;
DSW_FaultWordType g_FwAdcReference;
DSW_FaultWordType g_FwCanBusOff;

static void DSW_ConfigInitFw(DSW_FaultWordType *fw)
{
    DSW_FwInit(fw);
    DSW_SetMonitorEnable(fw);
}

void DSW_ConfigInit(void)
{
    DSW_NodeInit(&g_DswRootNode, DSW_NODE_ID_ROOT, DSW_NODE_TYPE_ROOT);
    DSW_NodeInit(&g_DswPowerNode, DSW_NODE_ID_POWER, DSW_NODE_TYPE_HSW);
    DSW_NodeInit(&g_DswCanNode, DSW_NODE_ID_CAN, DSW_NODE_TYPE_HSW);
    DSW_NodeInit(&g_DswSensorNode, DSW_NODE_ID_SENSOR, DSW_NODE_TYPE_HSW);
    DSW_NodeInit(&g_DswImuHswNode, DSW_NODE_ID_IMU_HSW, DSW_NODE_TYPE_HSW);
    DSW_NodeInit(&g_DswAdcHswNode, DSW_NODE_ID_ADC_HSW, DSW_NODE_TYPE_HSW);
    DSW_NodeInit(&g_DswImuAswNode, DSW_NODE_ID_IMU_ASW, DSW_NODE_TYPE_ASW);

    (void)DSW_NodeAddChild(&g_DswRootNode, &g_DswPowerNode);
    (void)DSW_NodeAddChild(&g_DswRootNode, &g_DswCanNode);
    (void)DSW_NodeAddChild(&g_DswRootNode, &g_DswSensorNode);

    (void)DSW_NodeAddChild(&g_DswPowerNode, &g_DswImuHswNode);
    (void)DSW_NodeAddChild(&g_DswPowerNode, &g_DswAdcHswNode);

    (void)DSW_NodeAddChild(&g_DswImuHswNode, &g_DswImuAswNode);

    DSW_ConfigInitFw(&g_FwPower12vUv);
    DSW_ConfigInitFw(&g_FwImuSpi);
    DSW_ConfigInitFw(&g_FwImuReset);
    DSW_ConfigInitFw(&g_FwImuTimeout);
    DSW_ConfigInitFw(&g_FwImuDataInvalid);
    DSW_ConfigInitFw(&g_FwAdcReference);
    DSW_ConfigInitFw(&g_FwCanBusOff);

    (void)DSW_NodeAddFault(&g_DswPowerNode, &g_FwPower12vUv);

    (void)DSW_NodeAddFault(&g_DswImuHswNode, &g_FwImuSpi);
    (void)DSW_NodeAddFault(&g_DswImuHswNode, &g_FwImuReset);
    (void)DSW_NodeAddFault(&g_DswImuHswNode, &g_FwImuTimeout);

    (void)DSW_NodeAddFault(&g_DswImuAswNode, &g_FwImuDataInvalid);

    (void)DSW_NodeAddFault(&g_DswAdcHswNode, &g_FwAdcReference);
    (void)DSW_NodeAddFault(&g_DswCanNode, &g_FwCanBusOff);
}
