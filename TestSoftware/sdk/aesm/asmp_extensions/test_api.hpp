/*
 * test_api.hpp
 *
 *  Created on: 2019年3月12日
 *      Author: uas
 */

#ifndef EXAMPLES_ASMP_EXTENSIONS_TEST_API_HPP_
#define EXAMPLES_ASMP_EXTENSIONS_TEST_API_HPP_

#include <asmp/mpmq.h>
#include <asmp/mpmutex.h>
#include <asmp/mpshm.h>
#include <asmp/mptask.h>


bool ASMP_mpmq_init(mpmq_t *mq, key_t key, cpuid_t cpu);
bool ASMP_mpmq_destroy(mpmq_t *mq);
bool ASMP_mpmq_send(mpmq_t *mq, int8_t msgid, uint32_t data);
int ASMP_mpmq_receive(mpmq_t *mq, uint32_t *data);
bool ASMP_mpmq_notify(mpmq_t *mq, int signo, void *sigdata);

bool ASMP_mpmutex_init(mpmutex_t *mutex, key_t key);
bool ASMP_mpmutex_destroy(mpmutex_t *mutex);
bool ASMP_mpmutex_lock(mpmutex_t *mutex);
bool ASMP_mpmutex_trylock(mpmutex_t *mutex);
bool ASMP_mpmutex_unlock(mpmutex_t *mutex);

bool ASMP_mpshm_init(mpshm_t *shm, key_t key, size_t size);
bool ASMP_mpshm_destroy(mpshm_t *shm);
void *ASMP_mpshm_attach(mpshm_t *shm, int shmflg);
bool ASMP_mpshm_detach(mpshm_t *shm);
bool ASMP_mpshm_control(mpshm_t *shm, int cmd, void *buf);
uintptr_t ASMP_mpshm_virt2phys(mpshm_t *shm, void *vaddr);
void *ASMP_mpshm_phys2virt(mpshm_t *shm, uintptr_t paddr);
bool ASMP_mpshm_remap(mpshm_t *shm, void *vaddr);

bool ASMP_mptask_init(mptask_t *task, const char *filename);
bool ASMP_mptask_init_secure(mptask_t *task, const char *filename);
bool ASMP_mptask_destroy(mptask_t *task);
bool ASMP_mptask_assign(mptask_t *task);
cpuid_t ASMP_mptask_getcpuid(mptask_t *task);
bool ASMP_mptask_bind(mptask_t *task, mpobj_t *obj);
bool ASMP_mptask_exec(mptask_t *task);
bool ASMP_mptask_join(mptask_t *task);
bool ASMP_mptask_attr_init(mptask_attr_t *attr);
bool ASMP_mptask_setattr(mptask_t *task, const mptask_attr_t *attr);
bool ASMP_mptask_getattr(mptask_t *task, mptask_attr_t *attr);


#endif /* EXAMPLES_ASMP_EXTENSIONS_TEST_API_HPP_ */
