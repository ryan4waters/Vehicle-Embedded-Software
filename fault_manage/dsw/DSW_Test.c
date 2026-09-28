#include <assert.h>
#include <stdio.h>
#include "DSW_Config.h"
#include "DSW_Tree.h"

static void test_parent_block(void)
{
    DSW_ReportInfoType info;

    assert(DSW_ReportFault(&g_DswPowerNode, &g_FwPower12vUv) == DSW_RESULT_OK);
    assert(DSW_IsNodeFault(&g_DswPowerNode));
    assert(DSW_IsParentFault(&g_DswImuHswNode));
    assert(DSW_IsParentFault(&g_DswImuAswNode));

    assert(DSW_ReportFaultEx(&g_DswImuAswNode,
                             &g_FwImuDataInvalid,
                             &info) == DSW_RESULT_PARENT_FAULT);
    assert(info.blockingNode == &g_DswPowerNode);
    assert(info.blockingFw == &g_FwPower12vUv);

    (void)DSW_ClearFault(&g_FwPower12vUv);
}

static void test_same_node_mutex(void)
{
    DSW_ReportInfoType info;

    assert(DSW_ReportFault(&g_DswImuHswNode, &g_FwImuSpi) == DSW_RESULT_OK);
    assert(DSW_ReportFaultEx(&g_DswImuHswNode,
                             &g_FwImuTimeout,
                             &info) == DSW_RESULT_NODE_ALREADY_FAULT);
    assert(info.blockingNode == &g_DswImuHswNode);
    assert(info.blockingFw == &g_FwImuSpi);

    (void)DSW_ClearFault(&g_FwImuSpi);
    (void)DSW_ClearFault(&g_FwImuTimeout);
}

static void test_history_not_blocking(void)
{
    assert(DSW_FwIsHistoryFault(&g_FwImuSpi) == true);
    assert(DSW_FwIsCurrentFault(&g_FwImuSpi) == false);
    assert(DSW_IsParentFault(&g_DswImuAswNode) == false);
}

int main(void)
{
    DSW_ConfigInit();
    DSW_Init();

    test_parent_block();
    test_same_node_mutex();
    test_history_not_blocking();

    printf("DSW test passed.\n");
    return 0;
}
