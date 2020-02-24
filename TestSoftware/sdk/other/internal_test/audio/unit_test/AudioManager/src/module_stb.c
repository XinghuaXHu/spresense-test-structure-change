#include "module_stb.h"

SYS_Module	*Load_Module_mod;
const char	*Load_Module_name;
void		*Load_Module_data;
SYS_Module	*UnLoad_Module_mod;
void		*UnLoad_Module_data;

int SYS_LoadModule(SYS_Module *mod, const char *name, void *data)
{
	Load_Module_mod = mod;
	Load_Module_name = name;
	Load_Module_data = data;
	return 0;
}

int SYS_UnloadModule(SYS_Module mod, void *data)
{
	UnLoad_Module_mod = mod;
	UnLoad_Module_data = data;
	return 0;
}

