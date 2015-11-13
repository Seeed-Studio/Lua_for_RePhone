
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "vmbt_cm.h"
#include "vmbt_spp.h"

#include "lua.h"
#include "lauxlib.h"

#define LOG(args...) printf(args)

#define LUA_BLUETOOTH "bluetooth"

#define BT_NAME "RePhone"

#define SPP_DATA "Hi, this is RePhone"

VMINT g_btcm_hdl = -1;  /* handle of BT service */
VMINT g_btspp_hdl = -1; /* handle of SPP service */

VM_BT_SPP_CONNECTION_ID g_spp_connection_id = -1;

static vm_bt_cm_bt_address_t g_btspp_addr; /* Store BT mac address of BT_NAME device we found during search process */
static VMINT g_btspp_min_buf_size;         /* size of buffer */
static void* g_btspp_buf;                  /* buffer that store SPP received data */
static VMINT g_b_server_find;              /* if BT_NAME device is founded or not duing search process */

/* SPP servie callback handler */
void app_btspp_cb(VM_BT_SPP_EVENT evt, vm_bt_spp_event_cntx_t* param, void* user_data)
{
    vm_bt_spp_event_cntx_t* cntx = (vm_bt_spp_event_cntx_t*)param;
    VMINT ret;
    switch(evt) {
    case VM_BT_SPP_EVENT_AUTHORIZE: {
        LOG("bt spp authorize\n");
        memset(&g_btspp_addr, 0, sizeof(g_btspp_addr));
        g_spp_connection_id = cntx->connection_id;
        ret = vm_bt_spp_get_device_address(cntx->connection_id, &g_btspp_addr);
        ret = vm_bt_spp_accept(cntx->connection_id, g_btspp_buf, g_btspp_min_buf_size, g_btspp_min_buf_size);
    } break;

    case VM_BT_SPP_EVENT_READY_TO_WRITE: {
        /* write SPP_DATA example string to remote side */
        LOG("bt spp ready to write\n");
    } break;

    case VM_BT_SPP_EVENT_READY_TO_READ:
        /* read data from remote side and print it out to log */
//        ret = vm_bt_spp_read(cntx->connection_id, g_btspp_buf, g_btspp_min_buf_size);
//        if(ret > 0) {
//            /* log the received data */
//            ((VMCHAR*)g_btspp_buf)[ret] = 0;
//            LOG("bt spp ready to read: [%s]\n", g_btspp_buf);
//            ret = vm_bt_spp_write(cntx->connection_id, SPP_DATA, strlen(SPP_DATA));
//        }
        break;

    case VM_BT_SPP_EVENT_DISCONNECT:
        break;
    }
}

/* BT servie callback handler */
static void app_btcm_cb(VMUINT evt, void* param, void* user_data)
{
    VMINT ret;
    switch(evt) {
    case VM_BT_CM_EVENT_ACTIVATE: {
        /* After activated, continue to scan for devices */
        vm_bt_cm_device_info_t dev_info = { 0 };

        vm_bt_cm_activate_t* active = (vm_bt_cm_activate_t*)param;
        /* set BT device host name */
        ret = vm_bt_cm_set_host_name((VMUINT8*)BT_NAME);
        /* display host info */
        ret = vm_bt_cm_get_host_device_info(&dev_info);
        LOG("BTCM vm_btcm_get_host_dev_info [%d]\n", ret);
        LOG("BTCM vm_btcm_get_host_dev_info[%s][0x%02x:%02x:%02x:%02x:%02x:%02x]\n",
            dev_info.name,
            ((dev_info.device_address.nap & 0xff00) >> 8),
            (dev_info.device_address.nap & 0x00ff),
            (dev_info.device_address.uap),
            ((dev_info.device_address.lap & 0xff0000) >> 16),
            ((dev_info.device_address.lap & 0x00ff00) >> 8),
            (dev_info.device_address.lap & 0x0000ff));
        /* set bt device as visibility*/
        ret = vm_bt_cm_set_visibility(VM_BT_CM_VISIBILITY_ON);
        break;
    }

    case VM_BT_CM_EVENT_DEACTIVATE: {
        ret = vm_bt_cm_exit(g_btcm_hdl);
        g_btcm_hdl = -1;
        break;
    }

    case VM_BT_CM_EVENT_SET_VISIBILITY: {
        vm_bt_cm_device_info_t dev_info = { 0 };

        vm_bt_cm_set_visibility_t* visi = (vm_bt_cm_set_visibility_t*)param;
        LOG("BTCM VM_BT_CM_EVENT_SET_VISIBILITY hdl[%d] rst[%d]\n", visi->handle, visi->result);
        break;
    }

    case VM_BT_CM_EVENT_INQUIRY_IND: {
        /* check if we found the BT_NAME device. And if found, stop the scan */
        vm_bt_cm_inquiry_indication_t* ind = (vm_bt_cm_inquiry_indication_t*)param;
        if(ind->discovered_device_number > 0) {
            VMUINT i = 0;
            vm_bt_cm_device_info_t dev_info = { 0 };
            for(i = 0; i < ind->discovered_device_number; i++) {
                vm_bt_cm_get_device_info_by_index(i, VM_BT_CM_DEVICE_DISCOVERED, &dev_info);
                LOG("find %s\n", dev_info.name);
            }
        }
        break;
    }

    case VM_BT_CM_EVENT_INQUIRY_COMPLETE: {
        vm_bt_cm_inquiry_complete_t* cmpl = (vm_bt_cm_inquiry_complete_t*)param;

        break;
    }

    default: {
        break;
    }
    }
}

int bluetooth_open(lua_State* L)
{
    g_btcm_hdl =
        vm_bt_cm_init(app_btcm_cb,
                      VM_BT_CM_EVENT_ACTIVATE | VM_BT_CM_EVENT_DEACTIVATE | VM_BT_CM_EVENT_SET_VISIBILITY |
                          VM_BT_CM_EVENT_SET_NAME | VM_BT_CM_EVENT_INQUIRY_IND | VM_BT_CM_EVENT_INQUIRY_COMPLETE,
                      NULL);

    if(VM_BT_CM_POWER_OFF == vm_bt_cm_get_power_status()) {
        /* Turn on BT if not yet on */
        vm_bt_cm_switch_on();
    } else {
        vm_bt_cm_set_visibility(VM_BT_CM_VISIBILITY_ON);
    }

    return 0;
}

int bluetooth_close(lua_State* L)
{
    vm_bt_cm_exit(g_btcm_hdl);
    return 0;
}

int bluetooth_set_visibility(lua_State* L)
{
    int v = lua_toboolean(L, 1);

    if(v) {
        vm_bt_cm_set_visibility(VM_BT_CM_VISIBILITY_ON);
    } else {
        vm_bt_cm_set_visibility(VM_BT_CM_VISIBILITY_OFF);
    }

    return 0;
}

int bluetooth_set_name(lua_State* L)
{
    char* name = luaL_checkstring(L, 1);

    vm_bt_cm_set_host_name(name);

    return 0;
}

int bluetooth_scan(lua_State* L)
{
    int ref;

    lua_pushvalue(L, 1);

    ref = luaL_ref(L, LUA_REGISTRYINDEX);

    lua_pushnumber(L, vm_bt_cm_search(10, 50, 0xFFFFFFFF, 1));

    return 1;
}

int bluetooth_spp_open(lua_State* L)
{
    VMINT result;
    VMUINT evt_mask = VM_BT_SPP_EVENT_START | VM_BT_SPP_EVENT_BIND_FAIL | VM_BT_SPP_EVENT_AUTHORIZE |
                      VM_BT_SPP_EVENT_CONNECT | VM_BT_SPP_EVENT_SCO_CONNECT | VM_BT_SPP_EVENT_READY_TO_WRITE |
                      VM_BT_SPP_EVENT_READY_TO_READ | VM_BT_SPP_EVENT_DISCONNECT | VM_BT_SPP_EVENT_SCO_DISCONNECT;

    g_btspp_hdl = vm_bt_spp_open(evt_mask, app_btspp_cb, NULL);
    if(g_btspp_hdl < 0) {
        return luaL_error(L, "vm_bt_spp_open() failed");
    }
    vm_bt_spp_set_security_level(g_btspp_hdl, VM_BT_SPP_SECURITY_NONE);
    g_btspp_min_buf_size = vm_bt_spp_get_min_buffer_size();

    g_btspp_buf = malloc(g_btspp_min_buf_size);
    g_btspp_min_buf_size = g_btspp_min_buf_size / 2;

    return 0;
}

int bluetooth_spp_close(lua_State* L)
{
    vm_bt_spp_close(g_btspp_hdl);
    return 0;
}

int bluetooth_spp_bind(lua_State* L)
{
    uint16_t uuid = luaL_checkinteger(L, 1);

    lua_pushnumber(L, vm_bt_spp_bind(g_btspp_hdl, uuid));
    return 1;
}

int bluetooth_spp_connect(lua_State* L)
{
    //    vm_bt_spp_connect(
    //            g_btspp_hdl, &g_btspp_addr, g_btspp_buf, g_btspp_min_buf_size, g_btspp_min_buf_size, UUID);

    return 0;
}

int bluetooth_spp_disconnect(lua_State* L)
{
    return 0;
}

int bluetooth_spp_read(lua_State* L)
{
    int ret;
    int n = luaL_checkinteger(L, 1);
    char* ptr = malloc(n);
    if(!ptr) {
        return luaL_error(L, "malloc() failed");
    }

    ret = vm_bt_spp_read(g_spp_connection_id, ptr, n);
    if(ret > 0) {
        int i;
        luaL_Buffer b;
        luaL_buffinit(L, &b);

        for(i = 0; i < ret; i++) {
            luaL_addchar(&b, ptr[i]);
        }
        
        luaL_pushresult(&b);
        
        return 1;
    }

    return 0;
}

int bluetooth_spp_write(lua_State* L)
{
    char *str;
    int strlen;
    str = luaL_checklstring(L, 1, &strlen);
    
    lua_pushnumber(L, vm_bt_spp_write(g_spp_connection_id, str, strlen));
    
    return 1;
}

#undef MIN_OPT_LEVEL
#define MIN_OPT_LEVEL 0
#include "lrodefs.h"

const LUA_REG_TYPE bluetooth_map[] = { { LSTRKEY("open"), LFUNCVAL(bluetooth_open) },
                                       { LSTRKEY("set_visibility"), LFUNCVAL(bluetooth_set_visibility) },
                                       { LSTRKEY("set_name"), LFUNCVAL(bluetooth_set_name) },
                                       { LSTRKEY("scan"), LFUNCVAL(bluetooth_scan) },
                                       { LSTRKEY("close"), LFUNCVAL(bluetooth_close) },
                                       { LNILKEY, LNILVAL } };

const LUA_REG_TYPE bluetooth_spp_map[] = { { LSTRKEY("open"), LFUNCVAL(bluetooth_spp_open) },
                                           { LSTRKEY("bind"), LFUNCVAL(bluetooth_spp_bind) },
                                           { LSTRKEY("connect"), LFUNCVAL(bluetooth_spp_connect) },
                                           { LSTRKEY("read"), LFUNCVAL(bluetooth_spp_read) },
                                           { LSTRKEY("write"), LFUNCVAL(bluetooth_spp_write) },
                                           { LSTRKEY("close"), LFUNCVAL(bluetooth_spp_close) },
                                           { LNILKEY, LNILVAL } };

LUALIB_API int luaopen_bluetooth(lua_State* L)
{
    luaL_register(L, "bluetooth", bluetooth_map);

    luaL_register(L, "btspp", bluetooth_spp_map);
    return 1;
}
