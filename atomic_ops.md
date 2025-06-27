# mram_unaligned.h

APIs for unaligned MRAM access (address/size) and tasklet-safe write/update of less than 8-byte values.

Defines
mram_update_int_atomic
Update an integer in MRAM atomically (i.e., multi-tasklet safe)

Parameters
dest : the integer address in MRAM

update_func : the pointer to the update function

args : a void* pointer, context passed to the update function

#define mram_update_int_atomic(dest, update_func, args)
mram_write_int_atomic
Write an integer in MRAM atomically (i.e., multi-tasklet safe)

Parameters
dest : the integer address in MRAM

val : the new integer value

#define mram_write_int_atomic(dest, val)
mram_update_byte_atomic
Update a byte in MRAM atomically (i.e., multi-tasklet safe)

Parameters
dest : the byte address in MRAM

update_func : the pointer to the update function

args : a void* pointer, context passed to the update function

#define mram_update_byte_atomic(dest, update_func, args)
mram_write_byte_atomic
Write a byte in MRAM atomically (i.e., multi-tasklet safe)

Parameters
dest : the byte address in MRAM

val : the new byte value

#define mram_write_byte_atomic(dest, val)

# mutex.h
Mutual exclusions.

A mutex ensures mutual exclusion between threads: only one thread can have the mutex at a time, blocking all the other threads trying to take the mutex.

Defines
MUTEX_GET
Return the symbol to use when using the mutex associated to the given name.

#define MUTEX_GET(_name)
MUTEX_INIT
Declare and initialize a mutex associated to the given name.

#define MUTEX_INIT(_name)
Types
mutex_id_t
A mutex object reference, as declared by MUTEX_INIT.

typedef uint8_t *mutex_id_t;
Functions
mutex_lock
Takes the lock on the given mutex.

Parameters
mutex : the mutex we want to lock

void mutex_lock(mutex_id_t mutex);
mutex_trylock
Tries to take the lock on the given mutex. If the lock is already taken, returns immediately.

Parameters
mutex : the mutex we want to lock

Returns
Whether the mutex has been successfully locked.

bool mutex_trylock(mutex_id_t mutex);
mutex_unlock
Releases the lock on the given mutex.

Parameters
mutex : the mutex we want to unlock

void mutex_unlock(mutex_id_t mutex);
mutex_pool.h
Mutual exclusions extension (hardware mutex pool).

A mutex ensures mutual exclusion between threads: only one thread can have the mutex at a time, blocking all the other threads trying to take the mutex. This file defines pools of mutexes. A pool of mutexes can be used to protect a large number of elements. When provided with a specific id to lock, the mutex_pool_lock function locks the hardware mutex whose id is equal to the log(N) least significant bits of the id to lock, N being the number of hardware mutexes in the pool.

Defines
MUTEX_POOL_INIT
Initialize a pool using a given number of hardware mutexes

The number of mutexes should be a power of 2

#define MUTEX_POOL_INIT(NAME, NB_MUTEXES)
Data types
struct mutex_pool
Structure holding a pool of hardware mutexes

struct mutex_pool {

uint8_t *hw_mutexes;

uint8_t hw_mutexes_mask;

};
Functions
mutex_pool_lock
Takes the lock for the given element id.

Parameters
mp : the mutex pool structure

id : the id of the element to lock

void mutex_pool_lock(struct mutex_pool *mp, uint16_t id);
mutex_pool_unlock
Releases the lock for the given element id.

Parameters
mp : the mutex pool structure

id : the id of the element to unlock

void mutex_pool_unlock(struct mutex_pool *mp, uint16_t id);