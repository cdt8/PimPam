// fifo.h - FIFO + WRAM缓冲池结构设计，支持异步Loader/Worker模式
#ifndef FIFO_H
#define FIFO_H

#include <stdint.h>
#include <stdbool.h>
#include <attributes.h>  // for __mram_ptr
#include "common.h"      // node_t 等基本定义
#include <stdatomic.h>
#include <stddef.h> 

#define WRAM_FIFO_CAPACITY 128   // Job缓冲上限，WRAM容量限制
#define WRAM_MAX_ROOT_BUF_SLOT 8
#define MRAM_MAX_SECOND_BUF_SLOT 16
#define MRAM_BUF_SIZE 256

// ---------------- Job 结构 ----------------
typedef struct {
    node_t root_id;
    uint8_t a_buf_index;   // root邻居槽位索引
    uint8_t b_buf_index;   // second邻居槽位索引
    uint32_t a_size;
    uint32_t b_size;
} job_t;

// ---------------- FIFO 定义 ----------------
typedef struct {
    job_t buffer[WRAM_FIFO_CAPACITY];
    volatile uint32_t head;
    volatile uint32_t tail;
    volatile uint8_t lock;
} fifo_t;

static inline void fifo_init(fifo_t *fifo) {
    fifo->head = 0;
    fifo->tail = 0;
    fifo->lock = 0;
}

inline bool fifo_is_empty(fifo_t *fifo) {
    return fifo->head == fifo->tail;
}

inline bool fifo_is_full(fifo_t *fifo) {
    return ((fifo->tail + 1) % WRAM_FIFO_CAPACITY) == fifo->head;
}

inline void fifo_lock_acquire(volatile uint8_t *lock) {
    while (__atomic_test_and_set(lock, __ATOMIC_ACQUIRE));
}

inline void fifo_lock_release(volatile uint8_t *lock) {
    __atomic_clear(lock, __ATOMIC_RELEASE);
}

bool fifo_enqueue(fifo_t *fifo, job_t job) {
    fifo_lock_acquire(&fifo->lock);
    if (fifo_is_full(fifo)) {
        fifo_lock_release(&fifo->lock);
        return false;
    }
    fifo->buffer[fifo->tail] = job;
    fifo->tail = (fifo->tail + 1) % WRAM_FIFO_CAPACITY;
    fifo_lock_release(&fifo->lock);
    return true;
}

bool fifo_dequeue(fifo_t *fifo, job_t *job) {
    fifo_lock_acquire(&fifo->lock);
    if (fifo_is_empty(fifo)) {
        fifo_lock_release(&fifo->lock);
        return false;
    }
    *job = fifo->buffer[fifo->head];
    fifo->head = (fifo->head + 1) % WRAM_FIFO_CAPACITY;
    fifo_lock_release(&fifo->lock);
    return true;
}

// ---------------- Root WRAM缓冲区元信息 ----------------
typedef struct {
    node_t root_id;
    node_t *ptr;          // 指向 WRAM 实际缓冲
    uint32_t size;
    uint32_t ref_count;
    bool in_use;
} a_buf_entry_t;

static inline int allocate_a_buf() {
    for (int i = 0; i < WRAM_MAX_ROOT_BUF_SLOT; i++) {
        if (!a_buf_table[i].in_use) {
            a_buf_table[i].in_use = true;
            a_buf_table[i].ref_count = 0;
            return i;
        }
    }
    return -1;  // 分配失败
}

static inline void release_a_buf(int index) {
    a_buf_table[index].in_use = false;
    a_buf_table[index].ref_count = 0;
}

// ---------------- Second WRAM缓冲区元信息 ----------------
typedef struct {
    node_t second_id;
    node_t *ptr;
    uint32_t size;
    bool in_use;
} b_buf_entry_t;

static inline int allocate_b_buf() {
    for (int i = 0; i < MRAM_MAX_SECOND_BUF_SLOT; i++) {
        if (!b_buf_table[i].in_use) {
            b_buf_table[i].in_use = true;
            return i;
        }
    }
    return -1;
}

static inline void release_b_buf(int index) {
    b_buf_table[index].in_use = false;
}

// ---------------- 全局共享资源（WRAM区域） ----------------
__host fifo_t global_fifo;
__host a_buf_entry_t a_buf_table[WRAM_MAX_ROOT_BUF_SLOT];
__host b_buf_entry_t b_buf_table[MRAM_MAX_SECOND_BUF_SLOT];
__host node_t a_buf_pool[WRAM_MAX_ROOT_BUF_SLOT][MRAM_BUF_SIZE];
__host node_t b_buf_pool[MRAM_MAX_SECOND_BUF_SLOT][MRAM_BUF_SIZE];


static inline void __atomic_add(volatile ans_t *addr, ans_t val) {
    ans_t old, new;
    do {
        old = *addr;
        new = old + val;
    } while (!__atomic_compare_exchange_n(addr, &old, new, false, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST));
}

#endif // FIFO_H
