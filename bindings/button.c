

#include "vmdcl.h"
#include "vmdcl_kbd.h"
#include "vmkeypad.h"

#include "lua.h"
#include "lauxlib.h"


extern lua_State *L;
int g_button_cb_ref = LUA_NOREF;


VMINT handle_keypad_event(VM_KEYPAD_EVENT event, VMINT code)
{
    /* output log to monitor or catcher */
    // vm_log_info("key event=%d,key code=%d", event, code); /* event value refer to VM_KEYPAD_EVENT */

    if(code == 30) {
        // 3 - long pressed
        // 2 - down
        // 1 - up
        lua_rawgeti(L, LUA_REGISTRYINDEX, g_button_cb_ref);
        lua_pushinteger(L, event);
        lua_call(L, 1, 0);
    }
    return 0;
}

int button_attach(lua_State *L)
{
    lua_pushvalue(L, 1);
    g_button_cb_ref = luaL_ref(L, LUA_REGISTRYINDEX);

    return 0;
}
 

#undef MIN_OPT_LEVEL
#define MIN_OPT_LEVEL 0
#include "lrodefs.h"

const LUA_REG_TYPE button_map[] =
{
    {LSTRKEY("attach"), LFUNCVAL(button_attach)},
    {LNILKEY, LNILVAL}
};


LUALIB_API int luaopen_button(lua_State *L)
{
    VM_DCL_HANDLE kbd_handle;
    vm_dcl_kbd_control_pin_t kbdmap;

    kbd_handle = vm_dcl_open(VM_DCL_KBD, 0);
    kbdmap.col_map = 0x09;
    kbdmap.row_map = 0x05;
    vm_dcl_control(kbd_handle, VM_DCL_KBD_COMMAND_CONFIG_PIN, (void*)(&kbdmap));

    vm_dcl_close(kbd_handle);
    
    vm_keypad_register_event_callback(handle_keypad_event);
    
    luaL_register(L, "button", button_map);
    return 1;
}
