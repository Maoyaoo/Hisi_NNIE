#ifndef __WK_SOT_QUEUE_H__
#define __WK_SOT_QUEUE_H__


#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

#include "wk_svp_type.h"
#include "wk_sot_thread.h"


// typedef struct hiSAMPLE_IVE_NODE_S
// {
//     VIDEO_FRAME_INFO_S stFrameInfo;
//     struct hiSAMPLE_IVE_NODE_S *next;
// }SAMPLE_IVE_NODE_S;

// typedef struct hiSAMPLE_IVE_QUEUE_S
// {
// 	SAMPLE_IVE_NODE_S *front, *rear;
// }SAMPLE_IVE_QUEUE_S;


typedef struct wk_SOT_NODE_S
{
    SotFrameInfo stFrameInfo;
    struct wk_SOT_NODE_S *prve;
    struct wk_SOT_NODE_S *next;    
}SOT_NODE_S;


typedef struct wk_SOT_QUEUE_S
{
	SOT_NODE_S *front, *rear;
}SOT_QUEUE_S;

#define QUEUE_CORE_ERROR_BASE     (1)
#define QUEUE_CORE_FRAMEWORK_ERROR_BASE (QUEUE_CORE_ERROR_BASE + 10000)

#define QUEUE_NULL_POINTER        (QUEUE_CORE_FRAMEWORK_ERROR_BASE + 1)
#define QUEUE_ILLEGAL_STATE       (QUEUE_CORE_FRAMEWORK_ERROR_BASE + 2)
#define QUEUE_OUT_OF_MEMORY       (QUEUE_CORE_FRAMEWORK_ERROR_BASE + 3)


SOT_QUEUE_S* SOT_QueueCreate(HI_S32 s32Len);
HI_VOID SOT_QueueDestory(SOT_QUEUE_S* pstQueueHead);
HI_VOID SOT_QueueClear(SOT_QUEUE_S* pstQueueHead);
HI_BOOL SOT_QueueIsEmpty(SOT_QUEUE_S* pstQueueHead);
HI_S32 SOT_QueueSize(SOT_QUEUE_S* pstQueueHead);
HI_S32 SOT_QueueAddNode(SOT_QUEUE_S* pstQueueHead, SotFrameInfo *pstFrameInfo);
SOT_NODE_S* SOT_QueueGetHeadNode(SOT_QUEUE_S* pstQueueHead);
SOT_NODE_S* SOT_QueueGetNode(SOT_QUEUE_S* pstQueueHead);
HI_VOID SOT_QueueFreeNode(SOT_NODE_S *pstNode);





#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __SAMPLE_IVE_QUEUE_H__ */

