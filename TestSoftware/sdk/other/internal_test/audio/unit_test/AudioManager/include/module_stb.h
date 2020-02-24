#ifndef _MODULE_STB_H_
#define _MODULE_STB_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef uint32_t SYS_Module;
int SYS_LoadModule(SYS_Module *mod, const char *name, void *data);
int SYS_UnloadModule(SYS_Module mod, void *data);

#ifdef __cplusplus
}
#endif

#endif /* _MODULE_STB_H_ */
