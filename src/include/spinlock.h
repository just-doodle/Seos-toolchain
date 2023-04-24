#ifndef __SPINLOCK_H__
#define __SPINLOCK_H__

void spinlock_lock(uint32_t *lock);
void spinlock_unlock(uint32_t *lock);

#endif /*__SPINLOCK_H__*/