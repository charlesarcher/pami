#include "SaOnNodeSyncGroup.h"
#ifdef __BSR_P6__
#include "BsrP6.h"
#else
#include "Bsr.h"
#endif

#include "ShmArray.h"
#include <stdlib.h>
#include <new>
#include <string.h>
#include "lapi_itrace.h"
#include "util/common.h"

SyncGroup::RC SaOnNodeSyncGroup::Init(
        const unsigned int mem_cnt, const unsigned int g_id,
        const unsigned int job_key,
        const unsigned int mem_id, void* param)
{
    PAMI_assert (param != NULL);
    Param_t* in_param = (Param_t*) param;
    this->multi_load = in_param->multi_load;
    if(mem_cnt == 0) {
        ITRC(IT_BSR, "SaOnNodeSyncGroup: Invalid member_cnt %d\n", member_cnt);
        return FAILED;
    }

    this->member_cnt = mem_cnt;
    this->group_id   = g_id;
    this->member_id  = mem_id;
    /* if there is only one member in the group, we do nothing. */
    if (mem_cnt == 1) {
        initialized = true;
        return SUCCESS;
    }

    // Initialize masks: 
    //      seq = 0, use mask[0] 
    //      seq = 1, use mask[1]
    memset(mask[0], 0, 8);
    memset(mask[1], (unsigned char)0x01, 8);

    // initialize SharedArray object

    const bool         is_leader  = (mem_id == 0);

    // modify seq, the init value is 0 for all tasks
    // seq of leader remains 0 with seq of the rest of tasks turned to 1
    if (!is_leader) {
        seq = !seq;
    }

    // temporary fix end
    SharedArray::RC sa_rc;
    try {
        // Started with BSR P7 first
#ifdef __BSR_P6__
        sa = new BsrP6;
#else
        sa = new Bsr;
#endif
        if (!in_param->use_shm_only) {
            sa_rc = sa->Init(mem_cnt, g_id, job_key, is_leader, mem_id, seq);
        } else {
            sa_rc = SharedArray::NOT_AVAILABLE;
        }

        bool priming_done = false;
        if (SharedArray::SUCCESS == sa_rc) {
            ITRC(IT_BSR, "(%d)SaOnNodeSyncGroup: Using Bsr\n", mem_id);
            this->group_desc = "SharedArray:Bsr";

            // show_bsr("After BSR Init "); 
        } else {
            ITRC(IT_BSR, "(%d)SaOnNodeSyncGroup: BSR setup failed with (%d)\n",
                    sa_rc);

            // If BSRs failed, try shm
            delete sa;
            sa = NULL;
            sa = new ShmArray;
            if (SharedArray::SUCCESS ==
                    sa->Init(mem_cnt, g_id, job_key, is_leader, mem_id, seq)) {
                ITRC(IT_BSR, "(%d)SaOnNodeSyncGroup: Using ShmArray\n",
                        mem_id);
                this->group_desc = "SharedArray:ShmArray";
            } else {
                delete sa;
                sa = NULL;
                // Cannot create a ShmArray object. No SharedArray object
                // available for using.
                ITRC(IT_BSR,
                        "(%d)SaOnNodeSyncGroup: Cannot create SharedArray obj\n",
                        mem_id);
                return FAILED;
            }
        }
    } catch (std::bad_alloc e) {
        sa = NULL;
        // TODO: if both BsrP7 and ShmArray failed, we should set sa=NULL
        // and use on node FIFO flow instead.
        ITRC(IT_BSR, "(%d)SaOnNodeSyncGroup: Out of memory.\n", mem_id);
        return FAILED;
    } catch (...) {
        ITRC(IT_BSR, "SaOnNodeSyncGroup: Unexpected exception caught.\n");
        return FAILED;
    }

    ITRC(IT_BSR, "SaOnNodeSyncGroup: Initialized with member_cnt=%d\n", mem_cnt);
    initialized = true;
    return SUCCESS;
}

/*!
 * \brief Default destructor.
 */
SaOnNodeSyncGroup::~SaOnNodeSyncGroup()
{
    delete sa;
}

void SaOnNodeSyncGroup::BarrierEnter()
{
    ITRC(IT_BSR, "SaOnNodeSyncGroup: Entering BarrierEnter()\n");
    PAMI_assert(this->initialized);
    // if only one member in group
    if (this->member_cnt == 1) {
        ITRC(IT_BSR, "SaOnNodeSyncGroup: Leaving BarrierEnter()\n");
        return;
    }

    if (member_id == 0) {
        if (multi_load) {
            ITRC(IT_BSR,
                    "SaOnNodeSyncGroup: Wait respons from follower (multi_load)\n");
            // assume multi_load is always supported
            unsigned int i=0;
            if (progress_cb) {
                for (i = 0; i + 8 <= member_cnt; i += 8)
                    while (sa->Load8(i) != *(unsigned long long*)(mask[seq]))
                        (*progress_cb)(progress_cb_info);

                for (; i + 4 <= member_cnt; i+= 4)
                    while (sa->Load4(i) != *(unsigned int*)(mask[seq]))
                        (*progress_cb)(progress_cb_info);

                for (; i + 2 <= member_cnt; i+= 2)
                    while (sa->Load2(i) != *(unsigned short*)(mask[seq]))
                        (*progress_cb)(progress_cb_info);

                // use 1-byte loads to finish the rest
                if (i == 0) i = 1;
                for (; i < member_cnt; i++)
                    while (sa->Load1(i) != seq)
                        (*progress_cb)(progress_cb_info);
            } else {
                for (i = 0; i + 8 <= member_cnt; i += 8)
                    while (sa->Load8(i) != *(unsigned long long*)(mask[seq]));

                for (; i + 4 <= member_cnt; i+= 4)
                    while (sa->Load4(i) != *(unsigned int*)(mask[seq]));

                for (; i + 2 <= member_cnt; i+= 2)
                    while (sa->Load2(i) != *(unsigned short*)(mask[seq]));

                // use 1-byte loads to finish the rest
                if (i == 0) i = 1;
                for (; i < member_cnt; i++)
                    while (sa->Load1(i) != seq);
            }
            ITRC(IT_BSR,
                    "SaOnNodeSyncGroup: Got respons from follower (multi_load)\n");
        } else {
            // use 1 byte load
            ITRC(IT_BSR,
                    "SaOnNodeSyncGroup: Wait respons from follower (single_load)\n");
            if (progress_cb) {
                for (unsigned int i = 1; i < member_cnt; i++) {
                    while (sa->Load1(i) != seq)
                        (*progress_cb)(progress_cb_info);
                }
            } else {
                for (unsigned int i = 1; i < member_cnt; i++) {
                    while (sa->Load1(i) != seq);
                }
            }
            ITRC(IT_BSR,
                    "SaOnNodeSyncGroup: Got respons from follower (single_load)\n");
        }
    } else {
        ITRC(IT_BSR, "SaOnNodeSyncGroup: Store1(%d, %d) called\n",
                member_id, seq);
        sa->Store1(member_id, !seq);
    }
    ITRC(IT_BSR, "SaOnNodeSyncGroup: Leaving BarrierEnter()\n");
}

void SaOnNodeSyncGroup::BarrierExit()
{
    ITRC(IT_BSR, "SaOnNodeSyncGroup: Entering BarrierExit()\n");
    PAMI_assert(this->initialized);
    // if only one member in group
    if (this->member_cnt == 1) {
        ITRC(IT_BSR, "SaOnNodeSyncGroup: Leaving BarrierExit()\n");
        return;
    }
    if (member_id == 0) {
        // notify others about barrier exit
        sa->Store1(member_id, !seq);
        ITRC(IT_BSR, "SaOnNodeSyncGroup: Store1(%d, %d) called\n",
                member_id, seq);
    } else {
        // wait for barrier exit
        ITRC(IT_BSR, "SaOnNodeSyncGroup: Wait response from leader\n");
        if (progress_cb) {
            while (sa->Load1(0) != seq)
                (*progress_cb)(progress_cb_info);
        } else {
            while (sa->Load1(0) != seq);
        }
        ITRC(IT_BSR, "SaOnNodeSyncGroup: Got response from leader\n");
    }

    // flip seq after barrier is done
    seq = !seq;
    ITRC(IT_BSR, "SaOnNodeSyncGroup: Leaving BarrierExit()\n");
}

void SaOnNodeSyncGroup::show_bsr(char* msg) {
    printf("BSR MEM at %s <", msg);
    for (int i = 0; i < member_cnt; i ++) {
        if (i == member_cnt - 1)
            printf("%d", sa->Load1(i));
        else
            printf("%d ", sa->Load1(i));
    }
    printf("> with seq %d \n", seq);
}

bool SaOnNodeSyncGroup::IsNbBarrierDone()
{
    // if only one member in group
    if (this->member_cnt == 1) {
        ITRC(IT_BSR, "SaOnNodeSyncGroup::IsNbBarrierDone() returns with member id: %d of %d, seq: %d\n", 
                member_id, member_cnt, seq);
        return true;
    }

    if(nb_barrier_stage == 0) { /* entering reduce stage */
//        show_bsr("stage 0");
        if (member_id == 0) {
            if (multi_load) {
                unsigned int i=0;
                for (i = 0; i + 8 <= member_cnt; i += 8)
                    if (sa->Load8(i) != *(unsigned long long*)(mask[seq]))
                        return false;

                for (; i + 4 <= member_cnt; i+= 4)
                    if (sa->Load4(i) != *(unsigned int*)(mask[seq]))
                        return false;

                for (; i + 2 <= member_cnt; i+= 2)
                    if (sa->Load2(i) != *(unsigned short*)(mask[seq]))
                        return false;

                // use 1-byte loads to finish the rest
                if (i == 0) i = 1;
                for (; i < member_cnt; i++)
                    if (sa->Load1(i) != seq)
                        return false;
            } else {
                // use 1 byte load
                for (unsigned int i = 1; i < member_cnt; i++) {
                    if (sa->Load1(i) != seq)
                        return false;
                }
            }
        } else {
            sa->Store1(member_id, !seq);
        }
        nb_barrier_stage = 1; /* reduce done */
    }

    if (nb_barrier_stage == 1) { /* enter broadcast stage */
//        show_bsr("stage 1");
        if (member_id == 0) {
            // notify others about barrier exit
            sa->Store1(member_id, !seq);
        } else {
            // wait for barrier exit
            if (sa->Load1(0) != seq)
                return false;
        }
        nb_barrier_stage = 2;
//        show_bsr("stage 2");
        seq = !seq;
    }

    return true;
}

void SaOnNodeSyncGroup::_Dump() const {
    // call base class function first
    SyncGroup::_Dump();
    printf("\tthis->group_desc      = \"%s\"\n", this->group_desc.c_str());
    printf("\tthis->seq             = %d\n", this->seq);
    printf("\tthis->sa              = %p\n", this->sa);
    printf("\tthis->multi_load      = %s\n", (this->multi_load)?"true":"false");
    printf("\tthis->mask[0]         = %p\n", this->mask[0]);
    for (unsigned int i = 0; this->mask[0] != NULL && i < this->member_cnt; ++i) {
        if (i == 0) printf("\t");
        printf("%d ", this->mask[0][i]);
        if (i == this->member_cnt - 1) printf("\n");
    }
    printf("\tthis->mask[1]         = %p\n", this->mask[1]);
    for (unsigned int i = 0; this->mask[1] != NULL && i < this->member_cnt; ++i) {
        if (i == 0) printf("\t");
        printf("%d ", this->mask[1][i]);
        if (i == this->member_cnt - 1) printf("\n");
    }
}
