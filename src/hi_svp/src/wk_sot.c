#include "wk_yolov3.h"
#include "wk_kcf.h"
#include "wk_sot.h"



int wk_sot_init(const char *model_path)
{
    int ret = 0;

    ret = wk_kcf_prm_init();
    if(ret != 0)
    {
        return -1;
    }

    ret = wk_yolov3_load_model(model_path);
    if(ret != 0)
    {
        return -1;
    }
    return 0;
}

int wk_sot_deinit()
{
    int ret = 0;
    ret = wk_kcf_prm_deinit();
    if(ret != 0)
    {
        return -1;
    }

    ret = wk_yolov3_clear();
    if(ret != 0)
    {
        return -1;
    }
    return 0;
}


HI_BOOL bDo_Detection = HI_FALSE;

int wk_sot_start(WK_SOT_INPUT_PRM *pstSotPrm)
{
    
    // HI_U32 s32Bbox_X = stTrackBox.s24q8X / 256 & (~1);
    // HI_U32 s32Bbox_Y = stTrackBox.s24q8Y / 256 & (~1);
    // HI_U32 s32Bbox_W = stTrackBox.u32Width & (~1);
    // HI_U32 s32Bbox_H = stTrackBox.u32Height & (~1);
    // printNum++;
    // if (printNum % 30 == 0)
    // {
    //     printf("s32Bbox_X = %d \n", s32Bbox_X);
    //     printf("s32Bbox_Y = %d \n", s32Bbox_Y);
    //     printf("s32Bbox_W = %d \n", s32Bbox_W);
    //     printf("s32Bbox_H = %d \n", s32Bbox_H);
    //     printf("##\n");
    // }

    HI_S32 s32Ret;
    WK_KCF_INPUT_PRM pstKcf_prm = {0};
    pstKcf_prm.bFlashSelectBbox = pstSotPrm->bFlashSelectBbox;
    pstKcf_prm.stInputBboxInfo = pstSotPrm->stInputBboxInfo;
    pstKcf_prm.stKcfFrmInfo = pstSotPrm->stKcfFrmInfo;
    if (bDo_Detection)
    {
        WK_YOLO_RECT_ARRAY_S stYoloRect; /*Detect result*/

        printf("====== Detecting!!!!! ======\n");
        s32Ret = wk_yolov3_run(&pstSotPrm->stDetectFrmInfo,&stYoloRect);
        if (HI_SUCCESS != s32Ret)
        {
            printf("Error(%#x),wk_yolov3_run failed!\n", s32Ret);
            return s32Ret;
        }

        //TO Do
        if(stYoloRect.u32TotalNum!=0)
        {
            bDo_Detection = HI_FALSE;

            pstKcf_prm.bFlashSelectBbox = HI_TRUE;
            s32Ret = wk_kcf_proc(&pstKcf_prm);          /*reflash track bbox*/
        }
        else
        {
            bDo_Detection = HI_TRUE;
        }
        return s32Ret;
        
    }
    else
    {
        printf("====== Tracking!!!!! ======\n");
        
        s32Ret = wk_kcf_proc(&pstKcf_prm);             /*update track bbox*/
        if(pstKcf_prm.stKcfRes.bTrackState)
        {
            /*to do return res*/
            memcpy(&pstSotPrm->stResBbox,&pstKcf_prm.stKcfRes.stResBbox,sizeof(&pstKcf_prm.stKcfRes.stResBbox));

        }
        else
        {
            bDo_Detection = HI_TRUE;
            memcpy(&pstSotPrm->stResBbox,&pstKcf_prm.stKcfRes.stResBbox,sizeof(&pstKcf_prm.stKcfRes.stResBbox));/*reset 0?*/
        }

        return s32Ret;
    }
}