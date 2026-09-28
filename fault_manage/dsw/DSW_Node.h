#ifndef DSW_NODE_H
#define DSW_NODE_H

#include <stdint.h>
#include <stdbool.h>
#include "DSW_Fault.h"

#define DSW_MAX_FW_PER_NODE       (8u)
#define DSW_MAX_CHILD_PER_NODE    (8u)

typedef enum
{
    DSW_NODE_TYPE_ROOT = 0,
    DSW_NODE_TYPE_HSW,
    DSW_NODE_TYPE_ASW
} DSW_NodeType;

typedef uint16_t DSW_NodeIdType;

typedef struct DSW_NodeTag DSW_NodeTypeDef;

struct DSW_NodeTag
{
    DSW_NodeIdType       nodeId;
    DSW_NodeType         nodeType;
    DSW_NodeTypeDef     *parent;
    DSW_NodeTypeDef     *children[DSW_MAX_CHILD_PER_NODE];
    uint8_t              childCount;
    DSW_FaultWordType   *faultWords[DSW_MAX_FW_PER_NODE];
    uint8_t              faultCount;
};

void DSW_NodeInit(DSW_NodeTypeDef *node,
                  DSW_NodeIdType nodeId,
                  DSW_NodeType nodeType);

bool DSW_NodeAddChild(DSW_NodeTypeDef *parent,
                      DSW_NodeTypeDef *child);

bool DSW_NodeAddFault(DSW_NodeTypeDef *node,
                      DSW_FaultWordType *fw);

bool DSW_NodeHasCurrentFault(const DSW_NodeTypeDef *node);
bool DSW_NodeHasParentFault(const DSW_NodeTypeDef *node);
const DSW_NodeTypeDef *DSW_NodeGetFirstFaultParent(const DSW_NodeTypeDef *node);
const DSW_FaultWordType *DSW_NodeGetFirstFaultParentFw(const DSW_NodeTypeDef *node);

#endif
