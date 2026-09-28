#include "DSW_Node.h"

void DSW_NodeInit(DSW_NodeTypeDef *node,
                  DSW_NodeIdType nodeId,
                  DSW_NodeType nodeType)
{
    uint8_t i;
    if (node == NULL) { return; }

    node->nodeId = nodeId;
    node->nodeType = nodeType;
    node->parent = NULL;
    node->childCount = 0u;
    node->faultCount = 0u;

    for (i = 0u; i < DSW_MAX_CHILD_PER_NODE; ++i)
    {
        node->children[i] = NULL;
    }

    for (i = 0u; i < DSW_MAX_FW_PER_NODE; ++i)
    {
        node->faultWords[i] = NULL;
    }
}

bool DSW_NodeAddChild(DSW_NodeTypeDef *parent,
                      DSW_NodeTypeDef *child)
{
    if ((parent == NULL) || (child == NULL)) { return false; }
    if (parent->childCount >= DSW_MAX_CHILD_PER_NODE) { return false; }
    if (child->parent != NULL) { return false; }

    parent->children[parent->childCount] = child;
    parent->childCount++;
    child->parent = parent;
    return true;
}

bool DSW_NodeAddFault(DSW_NodeTypeDef *node,
                      DSW_FaultWordType *fw)
{
    uint8_t i;
    if ((node == NULL) || (fw == NULL)) { return false; }
    if (node->faultCount >= DSW_MAX_FW_PER_NODE) { return false; }

    for (i = 0u; i < node->faultCount; ++i)
    {
        if (node->faultWords[i] == fw) { return false; }
    }

    node->faultWords[node->faultCount] = fw;
    node->faultCount++;
    return true;
}

bool DSW_NodeHasCurrentFault(const DSW_NodeTypeDef *node)
{
    uint8_t i;
    if (node == NULL) { return false; }

    for (i = 0u; i < node->faultCount; ++i)
    {
        if (DSW_FwIsCurrentFault(node->faultWords[i]))
        {
            return true;
        }
    }
    return false;
}

bool DSW_NodeHasParentFault(const DSW_NodeTypeDef *node)
{
    return (DSW_NodeGetFirstFaultParent(node) != NULL);
}

const DSW_NodeTypeDef *DSW_NodeGetFirstFaultParent(const DSW_NodeTypeDef *node)
{
    const DSW_NodeTypeDef *parent;

    if (node == NULL) { return NULL; }
    parent = node->parent;

    while (parent != NULL)
    {
        if (DSW_NodeHasCurrentFault(parent))
        {
            return parent;
        }
        parent = parent->parent;
    }
    return NULL;
}

const DSW_FaultWordType *DSW_NodeGetFirstFaultParentFw(const DSW_NodeTypeDef *node)
{
    const DSW_NodeTypeDef *parent;
    uint8_t i;

    if (node == NULL) { return NULL; }
    parent = node->parent;

    while (parent != NULL)
    {
        for (i = 0u; i < parent->faultCount; ++i)
        {
            if (DSW_FwIsCurrentFault(parent->faultWords[i]))
            {
                return parent->faultWords[i];
            }
        }
        parent = parent->parent;
    }
    return NULL;
}
