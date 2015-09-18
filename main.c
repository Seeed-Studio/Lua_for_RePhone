
#include <string.h>

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

#include "shell.h"

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

extern int luaopen_audio(lua_State *L);
extern int luaopen_gsm(lua_State *L);
extern int luaopen_timer(lua_State *L);

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



VMINT handle_keypad_event(VM_KEYPAD_EVENT event, VMINT code){
    /* output log to monitor or catcher */
    vm_log_info("key event=%d,key code=%d",event,code); /* event value refer to VM_KEYPAD_EVENT */

    if (code == 30) {
        if (event == 3) {   // long pressed

        } else if (event == 2) { // down
            printf("key is pressed\n");
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

void setup_lua()
{
    VM_THREAD_HANDLE handle;

    L = lua_open();
    lua_gc(L, LUA_GCSTOP, 0);  /* stop collector during initialization */
    luaL_openlibs(L);  /* open libraries */

    luaopen_audio(L);
    luaopen_gsm(L);
    luaopen_timer(L);

    lua_register(L, "msleep", msleep_c);

    lua_gc(L, LUA_GCRESTART, 0);

    luaL_dofile(L, "init.lua");

    if (0)
    {
        const char *script = "audio.play('nokia.mp3')";
        int error;
        error = luaL_loadbuffer(L, script, strlen(script), "line") ||
                lua_pcall(L, 0, 0, 0);
        if (error) {
            fprintf(stderr, "%s", lua_tostring(L, -1));
            lua_pop(L, 1);  /* pop error message from the stack */
        }
    }

    handle = vm_thread_create(shell_thread, L, 0);
	vm_thread_change_priority(handle, 245);

}

void handle_sysevt(VMINT message, VMINT param)
{
    switch (message) {
        case VM_EVENT_CREATE:
            // sys_timer_id = vm_timer_create_precise(1000, sys_timer_callback, NULL);
            setup_lua();
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

    console_init();
    console_puts("hello, linkit assist\n");



    key_init();
    vm_keypad_register_event_callback(handle_keypad_event);

    /* register system events handler */
    vm_pmng_register_system_event_callback(handle_sysevt);
}
