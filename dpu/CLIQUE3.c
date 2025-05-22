#include <dpu_mine.h>

static ans_t partial_ans[NR_TASKLETS];
static uint64_t partial_cycle[NR_TASKLETS];
static perfcounter_cycles cycles[NR_TASKLETS];
static perfcounter_cycles dc_cycles[NR_TASKLETS];

#ifdef BITMAP
static ans_t __imp_clique3_bitmap(sysname_t tasklet_id, node_t second_index) {
    ans_t ans = 0;
    mram_read(mram_bitmap[second_index], bitmap[tasklet_id], sizeof(bitmap[tasklet_id]));
    for (node_t i = 0; i < bitmap_size; i++) {
        uint32_t tmp = bitmap[tasklet_id][i];
        if (tmp) for (node_t j = 0; j < 32; j++) {
            if (tmp & (1 << j)) ans++;
        }
    }
    return ans;
}
#endif

static ans_t __imp_clique3_2(sysname_t tasklet_id, node_t root, edge_ptr root_begin, edge_ptr root_end,node_t second_root) {

    node_t(*tasklet_buf)[BUF_SIZE] = buf[tasklet_id];

    edge_ptr second_root_begin = row_ptr[second_root];  // intended DMA
    edge_ptr second_root_end = row_ptr[second_root + 1];  // intended DMA
    ans_t ans = 0;


#ifdef DC
    timer_start(&dc_cycles[tasklet_id]);
#endif

#ifdef NO_RUN   //test cycle without Intersection operation 
    node_t common_size = intersect_seq_buf_thresh_not_run(tasklet_buf, &col_idx[root_begin], root_end - root_begin, &col_idx[second_root_begin], second_root_end - second_root_begin, mram_buf[tasklet_id], second_root);
#else
    node_t common_size = intersect_seq_buf_thresh(tasklet_buf, &col_idx[root_begin], root_end - root_begin, &col_idx[second_root_begin], second_root_end - second_root_begin, mram_buf[tasklet_id], second_root);
#endif

#ifdef DC
    dc_cycle_ct[1][tasklet_id] +=timer_stop(&dc_cycles[tasklet_id]);  // intended DMA
#endif  

ans += common_size;
  
    return ans;
}

static ans_t __imp_clique3(sysname_t tasklet_id, node_t root) {
    edge_ptr root_begin = row_ptr[root];  // intended DMA
    edge_ptr root_end = row_ptr[root + 1];  // intended DMA
    ans_t ans = 0;

    for (edge_ptr i = root_begin; i < root_end; i++) {
        node_t second_root = col_idx[i];  // intended DMA
        if (second_root >= root) break;
        ans += __imp_clique3_2(tasklet_id, root,root_begin,root_end,second_root);
    }

    return ans;
}

extern void clique3(sysname_t tasklet_id) {

    node_t i = 0;
    large_degree_num = root_num; //if all node is large_degree
    while (i < root_num) {
        node_t root = roots[i];  // intended DMA
        node_t root_begin = row_ptr[root];  // intended DMA
        node_t root_end = row_ptr[root + 1];  // intended DMA
        if (root_end - root_begin < BRANCH_LEVEL_THRESHOLD) {
            large_degree_num = i;
            break;
        }
#ifdef PERF
        timer_start(&cycles[tasklet_id]);
#endif
#ifdef BITMAP
        build_bitmap(root, root_begin, root_end, tasklet_id);
#endif

        barrier_wait(&co_barrier);
        partial_ans[tasklet_id] = 0;
        for (edge_ptr j = root_begin + tasklet_id; j < root_end; j += NR_TASKLETS) {
            node_t second_root = col_idx[j];  // intended DMA
            if (second_root >= root) break;
#ifdef BITMAP
            partial_ans[tasklet_id] += __imp_clique3_bitmap(tasklet_id, j - root_begin);
#else
            partial_ans[tasklet_id] += __imp_clique3_2(tasklet_id, root,root_begin,root_end,second_root);
#endif
        }
#ifdef PERF
        partial_cycle[tasklet_id] = timer_stop(&cycles[tasklet_id]);
#endif
        barrier_wait(&co_barrier);
        if (tasklet_id == 0) {
            ans_t total_ans = 0;
#ifdef PERF
            uint64_t total_cycle = 0;
#endif
            for (uint32_t j = 0; j < NR_TASKLETS; j++) {
                total_ans += partial_ans[j];
#ifdef PERF
                total_cycle += partial_cycle[j];
#endif
            }
            ans[i] = total_ans;  // intended DMA
#ifdef PERF
            cycle_ct[i] = total_cycle;  // intended DMA
#endif
        }
        i++;
    }

    for (i += tasklet_id; i < root_num; i += NR_TASKLETS) {
        node_t root = roots[i];  // intended DMA

#ifdef PERF
        timer_start(&cycles[tasklet_id]);
#endif


        ans[i] = __imp_clique3(tasklet_id, root);  // intended DMA

#ifdef PERF
        cycle_ct[i] = timer_stop(&cycles[tasklet_id]);  // intended DMA
#endif

    }
}