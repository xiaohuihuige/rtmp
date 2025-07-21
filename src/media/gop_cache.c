#include "gop_cache.h"

GopCache *createGopCache(int idr_count)
{
    if (idr_count <= 0 )
        return NULL;

    GopCache *gop = CALLOC(1, GopCache);
    if (!gop)
        return NULL;

    gop->frame = NULL;
    gop->max_frame_count = idr_count;
    gop->haved_idr_count = 0;
    INIT_LIST_HEAD(&gop->list);

    return gop;
}

void destroyGopCache(GopCache *gop)
{
    if (!gop)
        return;

    GopCache *task_node = NULL;                                           
    GopCache *temp_pos = NULL;                                                         
    list_for_each_entry_safe(task_node, temp_pos, &gop->list, list)      
    {   
        if (!task_node)
            continue;

        list_del(&task_node->list); 
        bufferReleaseSpace(task_node->frame);                                                   
        FREE(task_node);                                                                 
    }

    FREE(gop);     
}

static void _resetTheGopCache(GopCache *gop)
{
    if (!gop)
        return;

    GopCache *task_node = NULL;                                           
    GopCache *temp_pos = NULL;                                                         
    list_for_each_entry_safe(task_node, temp_pos, &gop->list, list)      
    {   
        if (!task_node)
            continue;

        list_del(&task_node->list); 
        bufferReleaseSpace(task_node->frame);                                                   
        FREE(task_node);                                                                 
    }          
}

void pullFrameToGopCache(GopCache *gop, Buffer *frame)
{
    if (!gop || !frame)
        return;

    if (frame->frame_type == NAL_UNIT_TYPE_CODED_SLICE_IDR)
    {
        if (gop->haved_idr_count >= gop->max_frame_count)
        {
            _resetTheGopCache(gop);
            gop->haved_idr_count = 0;
        }
        gop->haved_idr_count++;
    }

    GopCache *new_gop = CALLOC(1, GopCache);
    if (!new_gop)
        return;

    new_gop->frame = frame;

    list_add_tail(&new_gop->list, &gop->list);
}

void sendGopCacheToClient(GopCache *gop, sendFrameToClient func, void *args)
{
    if (!gop || !args)
        return;

    GopCache *task_node = NULL;                                                                                                  
    list_for_each_entry(task_node, &gop->list, list)      
    {     
        if (!task_node || !task_node->frame)
            continue;

        bufferReferenceCount(task_node->frame);
        func(args, task_node->frame);                                                                                                                                            
    }  
}