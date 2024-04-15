// lua.hpp
// Lua header files for C++
// <<extern "C">> not supplied automatically because Lua also compiles as C++

extern "C" {
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}
// #include <stdio.h>
// #include <iostream>
// #include "lua.hpp"

// lua_State *L;

// extern "C" {
// static int l_add(lua_State *L);
// int luaopen_libadd(lua_State* L);
// }

// static int l_add(lua_State *L) {
//   double num1 = luaL_checknumber(L, 1);
//   double num2 = luaL_checknumber(L, 2);

//   lua_pushnumber(L, num1 + num2);
//   return 1;
// }

// static const struct luaL_Reg libadd[] = {
//         {"myadd", l_add},
//         {NULL, NULL}
// };

// extern ‘c’ int luaopen_libadd(lua_State *L) {
//   luaL_newlib(L, libadd);
//   return 1;
// }
