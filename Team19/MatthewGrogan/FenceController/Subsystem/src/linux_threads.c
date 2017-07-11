#include "linux_threads.h"

int     gNumLoops = 0;
HPLoop  **gHPLoopArray = NULL;


void *HPLoopHandler(void *cbData)
{
    HPLoop  *theLoop;
    int i=1;

    theLoop = (HPLoop *) cbData;

    while (i==1) {
	i = theLoop->LoopFunc(theLoop->cbData);
	if (theLoop->loopT == 0)
	    break;
	usleep(theLoop->loopT * 1000);
    }
}


int StartHPLoop(int (* LoopFunc) (void *cbData), void *cbData, double loopT)
{
    HPLoop              *theLoop;
    int                 i;
	
    if(gHPLoopArray == NULL) {
        gHPLoopArray = (HPLoop **) malloc(sizeof(HPLoop *));
        if(gHPLoopArray == NULL)
            return HPL_MALLOC_ERROR;
        theLoop = gHPLoopArray[0] = (HPLoop *) malloc(sizeof(HPLoop));
        if(theLoop == NULL)
            return HPL_MALLOC_ERROR;
        gNumLoops = 1;
    }
    else {
        gHPLoopArray = (HPLoop **) realloc(gHPLoopArray, (gNumLoops + 1) * sizeof(HPLoop *));
        if(gHPLoopArray == NULL)
            return HPL_MALLOC_ERROR;
        theLoop = gHPLoopArray[gNumLoops] = (HPLoop *) malloc(sizeof(HPLoop));
        if(theLoop == NULL)
            return HPL_MALLOC_ERROR;
        gNumLoops += 1;
    }

    theLoop->loopID = gNumLoops;
    theLoop->LoopFunc = LoopFunc;
    theLoop->cbData = cbData;
    theLoop->loopT = loopT;

    i = pthread_create(&(theLoop->loopTID), NULL, &HPLoopHandler, theLoop);
    if (i != 0)
        return HPL_THREAD_ERROR;

    return theLoop->loopID;
}


int CheckHPLoopState(int theLoopID)
{
    HPLoop  *theLoop;

    theLoop = gHPLoopArray[theLoopID - 1];
    if(pthread_tryjoin_np(theLoop->loopTID, NULL) == EBUSY)
        return HPL_STATE_BUSY;
    else
        return HPL_STATE_DONE;
}
