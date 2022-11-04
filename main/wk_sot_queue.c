#include <malloc.h>
#include "wk_sot_queue.h"

static HI_S32 s_s32MaxQueuelen = 0;
static HI_S32 s_s32CurQueueLen = 0;

SOT_QUEUE_S *SOT_QueueCreate(HI_S32 s32Len)
{
	SOT_QUEUE_S *pstQueueHead = NULL;

	if (s32Len < -1 || s32Len == 0)
	{
		return NULL;
	}
	s_s32CurQueueLen = 0;
	pstQueueHead = (SOT_QUEUE_S *)malloc(sizeof(SOT_QUEUE_S));
	if (NULL == pstQueueHead)
	{
		return NULL;
	}
	pstQueueHead->front = NULL;
	pstQueueHead->rear = NULL;
	s_s32MaxQueuelen = s32Len;

	// printf("###s_s32CurQueueLen: %d\n",s_s32CurQueueLen);
	// printf("###s_s32MaxQueuelen: %d\n",s_s32MaxQueuelen);

	return pstQueueHead;
}

HI_VOID SOT_QueueDestory(SOT_QUEUE_S *pstQueueHead)
{
	SOT_NODE_S *pstQueueTmp = NULL;

	if (NULL == pstQueueHead)
	{
		return;
	}

	pstQueueTmp = pstQueueHead->front;
	while (NULL != pstQueueTmp)
	{
		pstQueueHead->front = pstQueueTmp->next;
		free(pstQueueTmp);
		pstQueueTmp = pstQueueHead->front;
	}
	pstQueueHead->rear = pstQueueHead->front;
	s_s32CurQueueLen = 0;
	free(pstQueueHead);
	pstQueueHead = NULL;

	return;
}

HI_VOID SOT_QueueClear(SOT_QUEUE_S *pstQueueHead)
{
	SOT_NODE_S *pstQueueTmp = NULL;

	if (NULL == pstQueueHead)
	{
		return;
	}

	pstQueueTmp = pstQueueHead->front;
	while (NULL != pstQueueTmp)
	{
		pstQueueHead->front = pstQueueTmp->next;
		free(pstQueueTmp);
		pstQueueTmp = pstQueueHead->front;
	}
	pstQueueHead->rear = pstQueueHead->front;
	s_s32CurQueueLen = 0;

	return;
}

HI_BOOL SOT_QueueIsEmpty(SOT_QUEUE_S *pstQueueHead)
{
	if (NULL == pstQueueHead)
	{
		return HI_TRUE;
	}

	if (NULL != pstQueueHead->front)
	{
		return HI_FALSE;
	}

	return HI_TRUE;
}

HI_S32 SOT_QueueSize(SOT_QUEUE_S *pstQueueHead)
{
	if (NULL == pstQueueHead)
	{
		return 0;
	}

	return s_s32CurQueueLen;
}

HI_S32 SOT_QueueAddNode(SOT_QUEUE_S *pstQueueHead, SotFrameInfo *pstFrameInfo)
{
	SOT_NODE_S *pstQueueNode = NULL;

	if ((NULL == pstQueueHead) || (NULL == pstFrameInfo))
	{
		printf("1111111\n");
		return QUEUE_NULL_POINTER;
	}

	if ((s_s32MaxQueuelen != -1) && (s_s32CurQueueLen >= s_s32MaxQueuelen))
	{
		printf("2222222\n");
		printf("s_s32CurQueueLen: %d\n", s_s32CurQueueLen);
		printf("s_s32MaxQueuelen: %d\n", s_s32MaxQueuelen);
		return QUEUE_ILLEGAL_STATE;
	}

	pstQueueNode = (SOT_NODE_S *)malloc(sizeof(SOT_NODE_S));
	if (NULL == pstQueueNode)
	{
		printf("333333\n");
		return QUEUE_OUT_OF_MEMORY;
	}

	memcpy(&pstQueueNode->stFrameInfo, pstFrameInfo, sizeof(SotFrameInfo));
	pstQueueNode->next = NULL;
	if (SOT_QueueIsEmpty(pstQueueHead))
	{
		pstQueueHead->front = pstQueueNode;
		pstQueueHead->rear = pstQueueNode;
		pstQueueNode->prve = NULL;
	}
	else
	{
		pstQueueHead->rear->next = pstQueueNode; //队列后指针指向的元素的后驱指针等于新加入的元素地址
		pstQueueNode->prve = pstQueueHead->rear; //节点前驱指针指向队列后指针地址
		pstQueueHead->rear = pstQueueNode;   //队列后指针指向插入元素
	}

	s_s32CurQueueLen++;
	printf("add_____s_s32CurQueueLen: %d\n", s_s32CurQueueLen);

	return HI_SUCCESS;
}

SOT_NODE_S *SOT_QueueGetHeadNode(SOT_QUEUE_S *pstQueueHead)
{
	if ((NULL == pstQueueHead) || (NULL == pstQueueHead->front))
	{
		return NULL;
	}

	return pstQueueHead->front;
}

SOT_NODE_S *SOT_QueueGetNode(SOT_QUEUE_S *pstQueueHead)
{
	SOT_NODE_S *pstQueueTmp = NULL;

	if ((NULL == pstQueueHead) || (NULL == pstQueueHead->front))
	{
		return NULL;
	}

	pstQueueTmp = pstQueueHead->front;
	pstQueueHead->front = pstQueueTmp->next;
	pstQueueHead->front->prve = NULL;//add

	pstQueueTmp->next = NULL;//add
	pstQueueTmp->prve = NULL;//add
	if (NULL == pstQueueHead->front)
	{
		pstQueueHead->rear = pstQueueHead->front;
	}
	s_s32CurQueueLen--;

	return pstQueueTmp;
}

HI_VOID SOT_QueueFreeNode(SOT_NODE_S *pstNode)
{
	if (NULL != pstNode)
	{
		free(pstNode);
		pstNode = NULL;
	}

	return;
}



