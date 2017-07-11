#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <sys/types.h>

typedef struct
{
    int         loopID;
    pthread_t   loopTID;
    int         (* LoopFunc) (void *cbData);
    void        *cbData;
    double      loopT;      /* ms */
} HPLoop;

#define HPL_SUCCESS         1
#define HPL_MALLOC_ERROR    -1
#define HPL_THREAD_ERROR    -2

#define HPL_STATE_IDLE  0
#define HPL_STATE_BUSY  1
#define HPL_STATE_DONE  2

void *HPLoopHandler(void *cbData);
int StartHPLoop(int (* LoopFunc) (void *cbData), void *cbData, double loopT);
int CheckHPLoopState(int theLoopID);


