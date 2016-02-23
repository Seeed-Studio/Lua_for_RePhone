
#include <stdio.h>
#include <string.h>

#include "vmtype.h"
#include "vmlog.h"
#include "vmsystem.h"
#include "vmgsm_tel.h"
#include "vmgsm_sim.h"
#include "vmtimer.h"
#include "vmthread.h"

#include "shell.h"

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

extern void retarget_setup();
extern int luaopen_audio(lua_State* L);
extern int luaopen_gsm(lua_State* L);
extern int luaopen_timer(lua_State* L);
extern int luaopen_gpio(lua_State* L);
extern int luaopen_screen(lua_State* L);
extern int luaopen_i2c(lua_State* L);
extern int luaopen_tcp(lua_State* L);
extern int luaopen_https(lua_State* L);
extern int luaopen_bluetooth(lua_State* L);
extern int luaopen_button(lua_State* L);

lua_State* L = NULL;

VM_TIMER_ID_PRECISE sys_timer_id = 0;


void sys_timer_callback(VM_TIMER_ID_PRECISE sys_timer_id, void* user_data)
{
    vm_log_info("tick");
}

static int msleep_c(lua_State* L)
{
    long ms = lua_tointeger(L, -1);
    vm_thread_sleep(ms);
    return 0;
}

void lua_setup()
{
    VM_THREAD_HANDLE handle;

    L = lua_open();
    lua_gc(L, LUA_GCSTOP, 0); /* stop collector during initialization */
    luaL_openlibs(L);         /* open libraries */

    luaopen_audio(L);
    luaopen_gsm(L);
    luaopen_timer(L);
    luaopen_gpio(L);
    luaopen_screen(L);
    luaopen_i2c(L);
    luaopen_tcp(L);
    luaopen_https(L);
    luaopen_bluetooth(L);
    luaopen_button(L);

    lua_register(L, "msleep", msleep_c);

    lua_gc(L, LUA_GCRESTART, 0);

    luaL_dofile(L, "init.lua");

    handle = vm_thread_create(shell_thread, L, 0);
    vm_thread_change_priority(handle, 245);
}

void handle_sysevt(VMINT message, VMINT param)
{
    switch(message) {
    case VM_EVENT_CREATE:
        // sys_timer_id = vm_timer_create_precise(1000, sys_timer_callback, NULL);
        lua_setup();
        break;
    case SHELL_MESSAGE_ID:
        shell_docall(L);
        break;
    case VM_EVENT_QUIT:
        break;
    }
}

/* Entry point */
void vm_main(void)
{

    retarget_setup();
    fputs("hello, linkit assist\n", stdout);
    
    /* register system events handler */
    vm_pmng_register_system_event_callback(handle_sysevt);
}
