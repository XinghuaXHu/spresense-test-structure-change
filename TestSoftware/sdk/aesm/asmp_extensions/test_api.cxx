/*
 * test_api.cxx
 *
 *  Created on: 2019年3月12日
 *      Author: uas
 */
#include "test_api.hpp"


bool ASMP_mpmq_init(mpmq_t *mq, key_t key, cpuid_t cpu)
{
    return mpmq_init(mq, key, cpu) == 0;
}

bool ASMP_mpmq_destroy(mpmq_t *mq)
{
    return mpmq_destroy(mq) == 0;
}

bool ASMP_mpmq_send(mpmq_t *mq, int8_t msgid, uint32_t data)
{
    return mpmq_send(mq,msgid,data) == 0;
}

int ASMP_mpmq_receive(mpmq_t *mq, uint32_t *data)
{
    return mpmq_receive(mq,data);
}

bool ASMP_mpmq_notify(mpmq_t *mq, int signo, void *sigdata)
{
    return mpmq_notify(mq, signo, sigdata) == 0;
}

bool ASMP_mpmutex_init(mpmutex_t *mutex, key_t key)
{
    return mpmutex_init(mutex, key) == 0;
}

bool ASMP_mpmutex_destroy(mpmutex_t *mutex)
{
    return mpmutex_destroy(mutex) == 0;
}

bool ASMP_mpmutex_lock(mpmutex_t *mutex)
{
    return mpmutex_lock(mutex) == 0;
}

bool ASMP_mpmutex_trylock(mpmutex_t *mutex)
{
    return mpmutex_trylock(mutex) == 0;
}

bool ASMP_mpmutex_unlock(mpmutex_t *mutex)
{
    return mpmutex_unlock(mutex) == 0;
}

bool ASMP_mpshm_init(mpshm_t *shm, key_t key, size_t size)
{
    return mpshm_init(shm, key, size) == 0;
}

bool ASMP_mpshm_destroy(mpshm_t *shm)
{
    return mpshm_destroy(shm) == 0;
}

void *ASMP_mpshm_attach(mpshm_t *shm, int shmflg)
{
    return mpshm_attach(shm, shmflg);
}

bool ASMP_mpshm_detach(mpshm_t *shm)
{
    return mpshm_detach(shm) == 0;
}

bool ASMP_mpshm_control(mpshm_t *shm, int cmd, void *buf)
{
    return mpshm_control(shm, cmd, buf) == 0;
}

uintptr_t ASMP_mpshm_virt2phys(mpshm_t *shm, void *vaddr)
{
    return mpshm_virt2phys(shm, vaddr);
}

void *ASMP_mpshm_phys2virt(mpshm_t *shm, uintptr_t paddr)
{
    return mpshm_phys2virt(shm, paddr);
}

bool ASMP_mpshm_remap(mpshm_t *shm, void *vaddr)
{
    return mpshm_remap(shm, vaddr) == 0;
}

bool ASMP_mptask_init(mptask_t *task, const char *filename)
{
    return mptask_init(task, filename) == 0;
}

bool ASMP_mptask_init_secure(mptask_t *task, const char *filename)
{
    return mptask_init_secure(task, filename) == 0;
}

bool ASMP_mptask_destroy(mptask_t *task)
{
    int exit_status;
    return mptask_destroy(task, false, &exit_status) == 0;
}

bool ASMP_mptask_assign(mptask_t *task)
{
    return mptask_assign(task) == 0;
}

cpuid_t ASMP_mptask_getcpuid(mptask_t *task)
{
    return mptask_getcpuid(task);
}

bool ASMP_mptask_bind(mptask_t *task, mpobj_t *obj)
{
    return mptask_bindobj(task, obj) == 0;
}

bool ASMP_mptask_exec(mptask_t *task)
{
    return mptask_exec(task) == 0;
}

bool ASMP_mptask_join(mptask_t *task)
{
    int exit_status = 0;
    return mptask_join(task, &exit_status) == 0;
}

bool ASMP_mptask_attr_init(mptask_attr_t *attr)
{
    return mptask_attr_init(attr) == 0;
}

bool ASMP_mptask_setattr(mptask_t *task, const mptask_attr_t *attr)
{
    return mptask_setattr(task, attr);
}

bool ASMP_mptask_getattr(mptask_t *task, mptask_attr_t *attr)
{
    return mptask_getattr(task, attr);
}
