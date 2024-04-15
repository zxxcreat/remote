
#ifndef EXAMPLE_Hlua
#define EXAMPLE_Hlua
// 头文件
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif
#include <lua/lua.h>
#include <lua/lualib.h>
#include <lua/lauxlib.h>




lua_State * lua_ini(void);
void lua_register_c(lua_State *L,luaL_Reg *myModule,char *name);

int lua_error_m(lua_State *L);

#ifdef __cplusplus
}
#endif

#endif // EXAMPLE_H1
