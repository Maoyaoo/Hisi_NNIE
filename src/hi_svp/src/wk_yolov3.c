#include "yolov3_core.h"

#include "wk_yolov3.h"




/*yolov3 para*/
static SAMPLE_SVP_NNIE_MODEL_S s_stYolov3Model = {0};
static SAMPLE_SVP_NNIE_PARAM_S s_stYolov3NnieParam = {0};
static SAMPLE_SVP_NNIE_YOLOV3_SOFTWARE_PARAM_S s_stYolov3SoftwareParam = {0};


SAMPLE_SVP_NNIE_CFG_S   stNnieCfg = {0};



int wk_yolov3_load_model(const char *model_path)
{
    HI_S32 s32Ret = HI_SUCCESS;
    memset(&s_stYolov3Model,0,sizeof(s_stYolov3Model));
    memset(&s_stYolov3NnieParam,0,sizeof(s_stYolov3NnieParam));
    memset(&s_stYolov3SoftwareParam,0,sizeof(s_stYolov3SoftwareParam));

    /*Yolov3 Load model*/
    s32Ret = _yolov3_load_model(model_path, &s_stYolov3Model);
    if(HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Error(%#x),_yolov3_load_model failed!\n", s32Ret);
        return s32Ret;
    }

     /*Init Param*/
    s32Ret = yolov3_param_init(&s_stYolov3Model, &stNnieCfg, &s_stYolov3NnieParam, &s_stYolov3SoftwareParam);
    if(HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Error(%#x),yolov3_param_init failed!\n", s32Ret);
        return s32Ret;
    }

    return s32Ret;
}


int wk_yolov3_run(VIDEO_FRAME_INFO_S *pstExtFrmInfo,WK_YOLO_RECT_ARRAY_S *pstRect)
{
    HI_S32 s32Ret = HI_SUCCESS;
 

    s32Ret = yolov3_inference(&s_stYolov3NnieParam,&s_stYolov3SoftwareParam,pstExtFrmInfo,pstRect);
    SAMPLE_SVP_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret,YOLOV3_FAIL_0,SAMPLE_SVP_ERR_LEVEL_ERROR,
        "Error,yolov3_inference failed!\n");

    // s32Ret = yolov3_return_result(&s_stYolov3SoftwareParam);
    // SAMPLE_SVP_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret,YOLOV3_FAIL_0,SAMPLE_SVP_ERR_LEVEL_ERROR,
    //     "Error,yolov3_return_result failed!\n");

    return s32Ret;
    YOLOV3_FAIL_0:
        wk_yolov3_clear();
        return s32Ret;
}


int wk_yolov3_clear(void)
{
    HI_S32 s32Ret = HI_SUCCESS;    
    s32Ret = yolov3_param_deinit(&s_stYolov3NnieParam,&s_stYolov3SoftwareParam,&s_stYolov3Model);
    if(HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Error(%#x),yolov3_param_init failed!\n", s32Ret);
        return s32Ret;
    }
    return s32Ret;
}



