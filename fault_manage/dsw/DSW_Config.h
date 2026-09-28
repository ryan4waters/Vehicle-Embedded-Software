#ifndef DSW_CONFIG_H
#define DSW_CONFIG_H

#include "DSW_Node.h"

typedef enum
{
    DSW_NODE_ID_ROOT = 0x0000u,
    DSW_NODE_ID_POWER = 0x0100u,
    DSW_NODE_ID_CAN = 0x0200u,
    DSW_NODE_ID_SENSOR = 0x0300u,
    DSW_NODE_ID_IMU_HSW = 0x0110u,
    DSW_NODE_ID_ADC_HSW = 0x0120u,
    DSW_NODE_ID_IMU_ASW = 0x0111u
} DSW_ConfigNodeIdType;

typedef enum
{
    DSW_FW_ID_POWER_12V_UV = 0x0101u,
    DSW_FW_ID_IMU_SPI = 0x0111u,
    DSW_FW_ID_IMU_RESET = 0x0112u,
    DSW_FW_ID_IMU_TIMEOUT = 0x0113u,
    DSW_FW_ID_IMU_DATA_INVALID = 0x0114u,
    DSW_FW_ID_ADC_REFERENCE = 0x0121u,
    DSW_FW_ID_CAN_BUS_OFF = 0x0201u
} DSW_ConfigFwIdType;

extern DSW_NodeTypeDef g_DswRootNode;
extern DSW_NodeTypeDef g_DswPowerNode;
extern DSW_NodeTypeDef g_DswCanNode;
extern DSW_NodeTypeDef g_DswSensorNode;
extern DSW_NodeTypeDef g_DswImuHswNode;
extern DSW_NodeTypeDef g_DswAdcHswNode;
extern DSW_NodeTypeDef g_DswImuAswNode;

extern DSW_FaultWordType g_FwPower12vUv;
extern DSW_FaultWordType g_FwImuSpi;
extern DSW_FaultWordType g_FwImuReset;
extern DSW_FaultWordType g_FwImuTimeout;
extern DSW_FaultWordType g_FwImuDataInvalid;
extern DSW_FaultWordType g_FwAdcReference;
extern DSW_FaultWordType g_FwCanBusOff;

void DSW_ConfigInit(void);

#endif
