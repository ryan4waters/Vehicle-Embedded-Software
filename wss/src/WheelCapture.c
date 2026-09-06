#include "WheelCapture.h"

/*
 * Hardware-independent ring buffer.
 * The real project should fill this buffer from eMIOS/GTM capture + DMA.
 */
#define CAPTURE_BUFFER_SIZE (128U)

typedef struct
{
    WheelCaptureEvent_t buf[CAPTURE_BUFFER_SIZE];
    volatile uint16_t wr;
    volatile uint16_t rd;
    volatile uint32_t overflow;
} CaptureQueue_t;

static CaptureQueue_t g_capture[WHEEL_COUNT];

void WheelCapture_Init(void)
{
    for (uint8_t i = 0U; i < WHEEL_COUNT; ++i)
    {
        g_capture[i].wr = 0U;
        g_capture[i].rd = 0U;
        g_capture[i].overflow = 0U;
    }

    WheelCapture_HwInit();
    WheelCapture_HwStart();
}

bool WheelCapture_GetEvent(uint8_t wheel, WheelCaptureEvent_t *event)
{
    CaptureQueue_t *q;

    if ((wheel >= WHEEL_COUNT) || (event == NULL))
    {
        return false;
    }

    q = &g_capture[wheel];

    if (q->rd == q->wr)
    {
        return false;
    }

    *event = q->buf[q->rd];
    q->rd = (uint16_t)((q->rd + 1U) % CAPTURE_BUFFER_SIZE);

    return true;
}

/*
 * Called by the platform-specific DMA/capture layer after decoding a
 * timestamp and edge polarity from the DMA buffer.
 */
void WheelCapture_PushEvent(uint8_t wheel,
                            uint32_t timestamp,
                            WheelEdgeType_t edge)
{
    CaptureQueue_t *q;
    uint16_t next;

    if (wheel >= WHEEL_COUNT)
    {
        return;
    }

    q = &g_capture[wheel];
    next = (uint16_t)((q->wr + 1U) % CAPTURE_BUFFER_SIZE);

    if (next == q->rd)
    {
        q->overflow++;
        return;
    }

    q->buf[q->wr].timestamp = timestamp;
    q->buf[q->wr].edge = edge;
    q->wr = next;
}
