#ifndef	LUASCR_HPP
#define LUASCR_HPP

extern "C" {
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

class scriptmanager {
private:
	lua_State *ls
public:
	ScriptManager() : ls(luaL_newstate()) {
		luaL_openlibs(ls);
	}
	~ScriptManager() {
		lua_close(ls);
	}
	void run_script(const char* filename) {
		luaL_dofile(ls, filename);
	}
};

#endif