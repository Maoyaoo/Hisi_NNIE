#ifndef __SOT_THREAD_H__
#define __SOT_THREAD_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <sys/prctl.h>
#include <math.h>



#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif    /*__cplusplus*/


/*日志打印控制*/
#define SHOW_LOG 0


/*初始化跟踪线程*/
int Sot_Thread_Init(void);

/*去初始化跟踪线程*/
void Sot_Thread_DeInit(void);


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /*cplusplus*/


#endif