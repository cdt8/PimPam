#include <dpu_mine.h>
#include <common.h>
#include <fifo.h>

#define NR_LOADER 1
#define NR_WORKER (NR_TASKLETS - NR_LOADER)

#ifdef WRAM_ASYNC
extern void clique3(sysname_t tasklet_id)
{
    printf("clique3");
    if (tasklet_id == 0)
    {
        printf("initing\n");
        fifo_init(&global_fifo);
    }
    barrier_wait(&co_barrier);

    if (tasklet_id < NR_LOADER)
    {
        // === Loader Tasklet ===
        for (node_t root_id = tasklet_id; root_id < root_num; root_id += NR_LOADER)
        {
            printf("[INFO] Loader %d processing root %d\n", tasklet_id, root_id);

            node_t root = roots[root_id];
            edge_ptr rb = row_ptr[root];
            edge_ptr re = row_ptr[root + 1];
            node_t root_size = re - rb;
            int a_idx = allocate_a_buf();
            if (a_idx < 0)
                continue;

            mram_read(&col_idx[rb], a_buf_pool[a_idx], ALIGN8(root_size << SIZE_NODE_T_LOG));
            a_buf_table[a_idx].ref_count = root_size;

            for (edge_ptr j = rb; j < re; j++)
            {
                node_t second = col_idx[j];
                edge_ptr sb = row_ptr[second];
                edge_ptr se = row_ptr[second + 1];
                node_t b_size = se - sb;

                int b_idx = allocate_b_buf();
                if (b_idx < 0)
                    continue;

                mram_read(&col_idx[sb], b_buf_pool[b_idx], ALIGN8(b_size << SIZE_NODE_T_LOG));
                b_buf_table[b_idx].in_use = true;

                job_t job = {
                    .job_type = JOB_TYPE_NORMAL, // 标记为普通作业
                    .root_id = root_id,
                    .a_index = a_idx,
                    .b_index = b_idx,
                    .a_size = root_size,
                    .b_size = b_size,
                    .threshold = UINT32_MAX};

                while (!fifo_enqueue(&global_fifo, job))
                {
                    if (tasklet_id == 0)
                        printf("[INFO] FIFO full. Waiting...\n");
                }
            }
        }
        job_t terminate_job = {
            .job_type = JOB_TYPE_TERMINATE,
            .root_id = 0,
            .a_index = -1,
            .b_index = -1,
            .a_size = 0,
            .b_size = 0,
            .threshold = 0};

        while (!fifo_enqueue(&global_fifo, terminate_job))
        {
            // 等待队列有空间
        }
    }
    else
    {

        // === Worker Tasklet ===
        while (1)
        {
            job_t job;
            if (!fifo_dequeue(&global_fifo, &job))
                continue;
            if (job.job_type == JOB_TYPE_TERMINATE)
            {
                printf("[INFO] Worker %d received terminate signal, exiting\n", tasklet_id);
                break;
            }
            node_t *a = a_buf_pool[job.a_index];
            node_t *b = b_buf_pool[job.b_index];
            node_t res = intersect_from_buf(a, job.a_size, b, job.b_size, job.threshold);

            ans[job.root_id] += res;

            release_b_buf(job.b_index);
            a_buf_table[job.a_index].ref_count--;
            if (a_buf_table[job.a_index].ref_count == 0)
            {
                release_a_buf(job.a_index);
            }
        }
    }
}
#else // WRAM_ASYNC

static ans_t partial_ans[NR_TASKLETS];
static uint64_t partial_cycle[NR_TASKLETS];
static perfcounter_cycles cycles[NR_TASKLETS];

#ifdef BITMAP
static ans_t __imp_clique3_bitmap(sysname_t tasklet_id, node_t second_index)
{
    ans_t ans = 0;
    mram_read(mram_bitmap[second_index], bitmap[tasklet_id], sizeof(bitmap[tasklet_id]));
    for (node_t i = 0; i < bitmap_size; i++)
    {
        uint32_t tmp = bitmap[tasklet_id][i];
        if (tmp)
            for (node_t j = 0; j < 32; j++)
            {
                if (tmp & (1 << j))
                    ans++;
            }
    }
    return ans;
}
#endif

static ans_t __imp_clique3_2(sysname_t tasklet_id, node_t __mram_ptr *root_col, node_t root_size, node_t __mram_ptr *second_col, node_t second_size)
{

    if (!second_size)
        return 0;

    node_t(*tasklet_buf)[BUF_SIZE] = buf[tasklet_id];

#ifdef NO_RUN // test cycle without Intersection operation
    // node_t ans =  intersect_seq_buf_thresh_no_run(tasklet_buf, root_col, root_size, second_col, second_size);
    // node_t ans = 1;
#else
    node_t ans = intersect_seq_buf_thresh(tasklet_buf, root_col, root_size, second_col, second_size);
#endif

    return ans;
}

static ans_t __imp_clique3(sysname_t tasklet_id, node_t root)
{
    edge_ptr root_begin = row_ptr[root];   // intended DMA
    edge_ptr root_end = row_ptr[root + 1]; // intended DMA
    node_t root_size = root_end - root_begin;
    if (!root_size)
        return 0;
    ans_t ans = 0;

    mram_read(&col_idx[edge_offset + 2 * root_begin], col_buf[tasklet_id], MIN(16, root_end - root_begin) << (SIZE_EDGE_PTR_LOG + 1));

    for (edge_ptr i = 1; i < root_size; i++)
    {
        ans += __imp_clique3_2(tasklet_id, &col_idx[root_begin], i, &col_idx[col_buf[tasklet_id][2 * i]], col_buf[tasklet_id][2 * i + 1] - col_buf[tasklet_id][2 * i]);
    }

    return ans;
}

static ans_t __imp_clique3_partition(sysname_t tasklet_id, node_t root)
{
    edge_ptr root_begin = row_ptr[root];   // intended DMA
    edge_ptr root_end = row_ptr[root + 1]; // intended DMA
    node_t root_size = root_end - root_begin;
    if (!root_size)
        return 0;
    ans_t ans = 0;
    for (edge_ptr i = root_begin + 1; i < root_end; i++)
    {
        node_t second_root = col_idx[i];                // intended DMA
        edge_ptr second_begin = row_ptr[second_root];   // intended DMA
        edge_ptr second_end = row_ptr[second_root + 1]; // intended DMA
        ans += __imp_clique3_2(tasklet_id, &col_idx[root_begin], i - root_begin, &col_idx[second_begin], second_end - second_begin);
    }
    return ans;
}
extern void clique3(sysname_t tasklet_id)
{
    node_t i = 0;
    large_degree_num = root_num; /* if all node is large_degree */
    while (i < root_num)
    {
        node_t root = roots[i];              /* intended DMA */
        node_t root_begin = row_ptr[root];   /* intended DMA */
        node_t root_end = row_ptr[root + 1]; /* intended DMA */
        node_t root_size = root_end - root_begin;
        if (root_size < BRANCH_LEVEL_THRESHOLD)
        {
            large_degree_num = i;
            break;
        }

#ifdef PERF
        timer_start(&cycles[tasklet_id]);
#endif
        partial_ans[tasklet_id] = 0;

        if (no_partition_flag)
        {

            // without prefetch
            for (edge_ptr j = root_begin + tasklet_id + 1; j < root_end; j += NR_TASKLETS)
            {
                partial_ans[tasklet_id] += __imp_clique3_2(tasklet_id, &col_idx[root_begin], j - root_begin, &col_idx[col_idx[edge_offset + 2 * j]], col_idx[edge_offset + 2 * j + 1] - col_idx[edge_offset + 2 * j]);
            }
        }
        else
        {
            for (edge_ptr j = root_begin + tasklet_id + 1; j < root_end; j += NR_TASKLETS)
            {
                node_t second_root = col_idx[j];                /* intended DMA */
                edge_ptr second_begin = row_ptr[second_root];   /* intended DMA */
                edge_ptr second_end = row_ptr[second_root + 1]; /* intended DMA */
                partial_ans[tasklet_id] += __imp_clique3_2(tasklet_id, &col_idx[root_begin], j - root_begin, &col_idx[second_begin], second_end - second_begin);
            }
        }

#ifdef PERF
        partial_cycle[tasklet_id] = timer_stop(&cycles[tasklet_id]);
#endif
        barrier_wait(&co_barrier);
        if (tasklet_id == 0)
        {
            ans_t total_ans = 0;
#ifdef PERF
            uint64_t total_cycle = 0;
#endif
            for (uint32_t j = 0; j < NR_TASKLETS; j++)
            {
                total_ans += partial_ans[j];
#ifdef PERF
                total_cycle += partial_cycle[j];
#endif
            }
            ans[i] = total_ans; /* intended DMA */
#ifdef PERF
            cycle_ct[i] = total_cycle; /* intended DMA */
#endif
        }
        i++;

        barrier_wait(&co_barrier);
    }

    for (i += tasklet_id; i < root_num; i += NR_TASKLETS)
    {
        node_t root = roots[i]; /* intended DMA */

#ifdef PERF
        timer_start(&cycles[tasklet_id]);
#endif
        if (no_partition_flag)
            ans[i] = __imp_clique3(tasklet_id, root); /* intended DMA */
        else
            ans[i] = __imp_clique3_partition(tasklet_id, root); /* intended DMA */

#ifdef PERF
        cycle_ct[i] = timer_stop(&cycles[tasklet_id]); /* intended DMA */
#endif
    }
}
#endif // WRAM_ASYNC
