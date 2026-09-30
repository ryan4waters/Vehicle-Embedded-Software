#include "Hsm_Ipc.h"
#include "Tc377_Hsm_Hal.h"

typedef struct {
    Hsm_U8 used;
    Hsm_JobType job;
    Hsm_JobRequestType req;
    Hsm_JobCallbackType cb;
    Hsm_U32 context;
} Entry;

static Entry q[HSM_QUEUE_DEPTH];
static Hsm_U32 rd, wr;

Hsm_ReturnType Hsm_IpcInit(void)
{
    rd = wr = 0u;
    Tc377_HsmHal_Init();
    return HSM_OK;
}

static Hsm_U8 Full(void)
{
    return ((wr + 1u) % HSM_QUEUE_DEPTH) == rd;
}

Hsm_ReturnType Hsm_IpcSubmit(
    Hsm_JobType job,
    const Hsm_JobRequestType *req,
    Hsm_JobCallbackType cb,
    Hsm_U32 context)
{
    if (!req) return HSM_E_PARAM;
    if (Full()) return HSM_E_BUSY;

    q[wr].used = 1u;
    q[wr].job = job;
    q[wr].req = *req;
    q[wr].cb = cb;
    q[wr].context = context;
    wr = (wr + 1u) % HSM_QUEUE_DEPTH;
    return HSM_OK;
}

Hsm_ReturnType Hsm_IpcProcess(void)
{
    Hsm_ReturnType ret;
    Entry *e;

    if (rd == wr) return HSM_OK;

    e = &q[rd];
    ret = Tc377_HsmHal_Execute(e->job, &e->req);

    if (ret == HSM_E_BUSY)
        return ret;

    if (e->cb)
        e->cb(e->job, ret, e->context);

    e->used = 0u;
    rd = (rd + 1u) % HSM_QUEUE_DEPTH;
    return ret;
}

void Hsm_Ipc_CompletionIsr(void)
{
    /*
     * TC377 HSM completion interrupt adapter.
     * A real implementation should acknowledge the interrupt,
     * read completion status and let the Crypto/CSM callback path
     * consume the result.
     */
}
