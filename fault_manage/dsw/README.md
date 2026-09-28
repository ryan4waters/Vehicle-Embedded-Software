# DSW Fault Tree Package

## 1. Architecture

Root -> HSW -> ASW -> Fault Word

A node is considered currently faulty when any Fault Word registered
under that node has Bit0 CurrentFault = 1.

## 2. Fault Word

Bit0: Current fault
Bit1: Diagnostic completed
Bit2: Historical fault
Bit3: Monitor enabled
Bit4: Reserved

## 3. Core constraints

### Parent fault blocking

If any ancestor node currently has a fault, a child Fault Word cannot
be reported as a current fault.

Historical fault does NOT participate in parent blocking.

### Same-node mutual exclusion

If one Fault Word under a node is already currently faulty, another
Fault Word under the same node cannot be reported.

### Monitor enable

A Fault Word must have MonitorEnable set before DSW_ReportFault() can
set the current fault.

## 4. Typical usage

DSW_ConfigInit();

if (ImuSpiErrorDetected())
{
    (void)DSW_ReportFault(&g_DswImuHswNode, &g_FwImuSpi);
}

if (ImuDataInvalid())
{
    (void)DSW_ReportFault(&g_DswImuAswNode, &g_FwImuDataInvalid);
}

## 5. Important design choice

DSW_ReportFault() is the unique entry point for setting CurrentFault.
Application/HSW/ASW software should not directly set Bit0.

Bit2 HistoryFault is deliberately independent from current propagation.

## 6. Test

The package contains DSW_Test.c.

Example GCC build:

gcc -std=c99 -Wall -Wextra -pedantic \
    DSW_Fault.c DSW_Node.c DSW_Tree.c DSW_Config.c DSW_Test.c \
    -o dsw_test

./dsw_test
