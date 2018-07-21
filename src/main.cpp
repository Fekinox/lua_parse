// Lua libraries
extern "C" {
	#include <lua.h>
	#include <lualib.h>
	#include <lauxlib.h>
}
#include <cstdlib>

#include <random>		//RNG
#include <chrono>		//Seeding RNG
#include <array>		//Arrays
#include <cinttypes>	//I'm a neat freak

using sysc = std::chrono::system_clock;

constexpr uint8_t max_data = 16;	//Max data entries
constexpr uint8_t max_scripts = 4;	//Max # of scripts

struct movedata;
struct scripts;

int move(lua_State*);
int getpos(lua_State*);
int setpos(lua_State*);

// Container to hold all script states
struct scripts {
	// Arrays for states and filenames
	std::array<lua_State*, max_scripts> states;
	std::array<std::string, max_scripts> filenames;
	//Number of states; also doubles as the index to put the next state
	uint8_t scrc = 0;
	//Loads a script and returns its index
	uint8_t load_script(const std::string& filename) {
		// Fail if script limit reached
		if(scrc == max_scripts) { printf("Script limit reached\n"); return; }
		// Initialize the new Lua state
		auto s = luaL_newstate();
		// Open Lua standard libraries
		luaL_openlibs(s);
		// Attempt to load and execute the file
		if(luaL_dofile(s, filename.c_str())) {
			printf("Failed to load %s\n", filename.c_str());
			return;
		}

		// Register functions
		lua_register(s, "move", move);
		lua_register(s, "getpos", getpos);
		lua_register(s, "setpos", setpos);

		// Update the arrays
		states[scrc] = s;
		filenames[scrc] = filename;
		printf("Script %s loaded\n", filename.c_str());
		return scrc++;
	}
	void unload_script(uint8_t idx) {
		// Close the lua state
		lua_close(states[idx]);
		// Replace the empty state with the last element
		states[idx] = states[scrc];
		states[scrc--] = 0;
	}

	// Return the filename of a script at an index
	inline std::string get_filename(uint8_t idx) const { return filenames[idx]; }
};

// Container to hold position data
struct movedata {
	struct vec {
		// Barebones vector type.
		float x, y = 0;
	};
	// Arrays to hold position data and script indices
	std::array<vec, max_data> posdata;
	std::array<uint8_t, max_data> scriptindices;

	// Initialize all indices to 0
	movedata () {
		scriptindices.fill(0);
	}

	//Dump all information about the data
	void dump(const scripts& scrct) {
		for (int i = 0; i < max_data; ++i) {
			auto &p = posdata[i];
			auto &s = scriptindices[i];
			printf(
				"----------\n"
				"Index: %d\n"
				"X: %.2f\n"
				"Y: %.2f\n"
				"Script: %s\n"
				"----------\n", i, p.x, p.y, scrct.get_filename(s).c_str());
		}
	}

	// Run the scripts!
	void run_process(const scripts& scrct) {
		// Load in script states
		auto &scrs = scrct.states;
		for (int i = 0; i < max_data; ++i) {
			auto &p = posdata[i];
			auto &s = scrs[scriptindices[i]];
			// Set global variables to refer to the data containers and the current index.
			lua_pushlightuserdata(s, this);
			lua_setglobal(s, "mvdata");
			lua_pushinteger(s, i);
			lua_setglobal(s, "index");
			// Find the process function and run it.
			lua_getglobal(s, "process");
			lua_pcall(s, 0, 0, 0);
		}
	}
};

// FUNCTIONS
int move(lua_State* ls) {
	lua_getglobal(ls, "mvdata");
	lua_getglobal(ls, "index");

	float dx = lua_tonumber(ls, 1);
	float dy = lua_tonumber(ls, 2);
	auto mvdata = static_cast<movedata*>(lua_touserdata(ls, 3));
	uint8_t idx = lua_tointeger(ls, 4); 

	auto &p = mvdata->posdata[idx];
	p.x += dx;
	p.y += dy;
	return 0;
}
int getpos(lua_State* ls) {
	lua_getglobal(ls, "mvdata");
	lua_getglobal(ls, "index");
	auto mvdata = static_cast<movedata*>(lua_touserdata(ls, 1));
	auto idx = lua_tointeger(ls, 2);
	auto &p = mvdata->posdata[idx];
	lua_pushnumber(ls, p.x);
	lua_pushnumber(ls, p.y);
	return 2;
}
int setpos(lua_State* ls) {
	lua_getglobal(ls, "mvdata");
	lua_getglobal(ls, "index");
	float x = lua_tonumber(ls, 1);
	float y = lua_tonumber(ls, 2);
	auto mvdata = static_cast<movedata*>(lua_touserdata(ls, 3));
	auto idx = lua_tointeger(ls, 4);

	auto &p = mvdata->posdata[idx];
	p.x = x;
	p.y = y;
	return 0;	
}

int main(int argc, char const *argv[]) {
	unsigned seed = sysc::now().time_since_epoch().count();
	std::mt19937 gen(seed);
	std::uniform_real_distribution<float> fdist(0.0, 100.0);
	std::uniform_int_distribution<int> idist(0, 1);
	movedata m;
	scripts s;
	s.load_script("double.lua");
	s.load_script("half.lua");
	for (int i = 0; i < max_data; ++i) {
		auto &pos = m.posdata[i];
		auto &scr = m.scriptindices[i];
		pos.x = fdist(gen);
		pos.y = fdist(gen);
		scr = idist(gen);
	}

	m.dump(s);

	m.run_process(s);

	m.dump(s);
	return 0;
}