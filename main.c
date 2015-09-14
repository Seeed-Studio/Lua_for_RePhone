
#include "vmtype.h"
#include "vmlog.h"
#include "vmsystem.h"
#include "vmgsm_tel.h"
#include "vmgsm_sim.h"
#include "vmtimer.h"
#include "vmdcl.h"
#include "vmdcl_kbd.h"
#include "vmkeypad.h"
#include "console.h"
#include "vmthread.h"

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

lua_State *L = NULL;

VM_TIMER_ID_PRECISE sys_timer_id = 0;

void key_init(void){
    VM_DCL_HANDLE kbd_handle;
    vm_dcl_kbd_control_pin_t kbdmap;

    kbd_handle = vm_dcl_open(VM_DCL_KBD,0);
    kbdmap.col_map = 0x09;
    kbdmap.row_map =0x05;
    vm_dcl_control(kbd_handle,VM_DCL_KBD_COMMAND_CONFIG_PIN, (void *)(&kbdmap));

    vm_dcl_close(kbd_handle);
}



void sys_timer_callback(VM_TIMER_ID_PRECISE sys_timer_id, void* user_data)
{

    vm_log_info("tick");
    // console_puts("ping\n");

}

void handle_sysevt(VMINT message, VMINT param)
{
    switch (message) {
        case VM_EVENT_CREATE:
            sys_timer_id = vm_timer_create_precise(1000, sys_timer_callback, NULL);
            break;
        case VM_EVENT_QUIT:
            break;
    }
}

VMINT handle_keypad_event(VM_KEYPAD_EVENT event, VMINT code){
    /* output log to monitor or catcher */
    vm_log_info("key event=%d,key code=%d",event,code); /* event value refer to VM_KEYPAD_EVENT */

    if (code == 30) {
        if (event == 3) {   // long pressed

        } else if (event == 2) { // down

        } else if (event == 1) { // up

        }
    }
    return 0;
}


static int msleep_c(lua_State *L)
{
    long ms = lua_tointeger(L, -1);
    vm_thread_sleep(ms);
    return 0;
}

VMINT32 __main_thread(VM_THREAD_HANDLE thread_handle, void* user_data)
{
    if (1)
    {
        const char *script = "print('hello')";
        int error;
        error = luaL_loadbuffer(L, script, strlen(script), "line") ||
                lua_pcall(L, 0, 0, 0);
        if (error) {
            fprintf(stderr, "%s", lua_tostring(L, -1));
            lua_pop(L, 1);  /* pop error message from the stack */
        }
    }

    dotty(L);

	for (;;)
	{
        // console_puts("hello, linkit\n");


        console_putc(console_getc());
	}
}

/* Entry point */
void vm_main(void)
{
    VM_THREAD_HANDLE handle;
    console_init();
    console_puts("hello, linkit assist\n");

    L = lua_open();
    lua_gc(L, LUA_GCSTOP, 0);  /* stop collector during initialization */
    luaL_openlibs(L);  /* open libraries */

    lua_pushcfunction(L, luaopen_io);
    lua_pushstring(L, "io");
    lua_call(L, 1, 0);

    lua_pushcfunction(L, luaopen_table);
    lua_pushstring(L, "table");
    lua_call(L, 1, 0);

    lua_pushcfunction(L, luaopen_string);
    lua_pushstring(L, "string");
    lua_call(L, 1, 0);

    lua_pushcfunction(L, luaopen_package);
    lua_pushstring(L, "package");
    lua_call(L, 1, 0);

    lua_register(L, "msleep", msleep_c);

    lua_gc(L, LUA_GCRESTART, 0);

    luaL_dofile(L, "init.lua");

    key_init();
    vm_keypad_register_event_callback(handle_keypad_event);

    /* register system events handler */
    vm_pmng_register_system_event_callback(handle_sysevt);

    handle = vm_thread_create(__main_thread, NULL, 0);
	vm_thread_change_priority(handle, 245);
}
