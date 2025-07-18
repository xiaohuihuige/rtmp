#include "gop_cache.h"

GopCache *createGopCache(int idr_count)
{
    GopCache *gop = CALLOC(1, GopCache);
    if (!gop)
        return NULL;

    gop->frame = NULL;
    gop->max_frame_count = idr_count;
    gop->haved_idr_count = 0;
    INIT_LIST_HEAD(&gop->list);
    return gop;
}

static void _resetTheGopCache(GopCache *gop)
{
    GopCache *task_node = NULL;                                           
    GopCache *temp_pos = NULL;                                                         
    list_for_each_entry_safe(task_node, temp_pos, &gop->list, list)      
    {                                                                                   
        list_del(&task_node->list); 
        bufferReleaseSpace(task_node->frame);                                                   
        FREE(task_node);                                                                 
    }          
}

void pullFrameToGopCache(GopCache *gop, Buffer *frame)
{
    if (frame->frame_type == NAL_UNIT_TYPE_CODED_SLICE_IDR)
    {
        gop->haved_idr_count++;
        if (gop->haved_idr_count >= gop->max_frame_count + 1)
        {
            _resetTheGopCache(gop);
            gop->haved_idr_count = 1;
        }
    }

    GopCache *new_gop = CALLOC(1, GopCache);
    if (!new_gop)
        return;

    new_gop->frame = frame;

    list_add_tail(&new_gop->list, &gop->list);
}

void sendGopCacheToClient(GopCache *gop, sendFrameToClient func, void *args)
{
    GopCache *task_node = NULL;                                                                                                  
    list_for_each_entry(task_node, &gop->list, list)      
    {     
        bufferReferenceCount(task_node->frame);
        func(args, task_node->frame);                                                                                                                                            
    }  
}