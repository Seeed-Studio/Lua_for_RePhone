

#include <string.h>

#include "vmlog.h"
#include "vmgsm_tel.h"
#include "vmchset.h"
#include "vmgsm_sim.h"

#include "lua.h"
#include "lauxlib.h"

enum VoiceCall_Status { IDLE_CALL, CALLING, RECEIVINGCALL, TALKING };

vm_gsm_tel_call_listener_callback g_call_status_callback = NULL;

static void (*g_call_state_changed_callback)(VMINT8) = NULL;

vm_gsm_tel_id_info_t g_uid_info;
VMINT8 g_call_status = IDLE_CALL;
VMINT8 g_number[42];

void call_listener_func(vm_gsm_tel_call_listener_data_t *data) {
  vm_log_info("call_listener_func");

  if (data->call_type == VM_GSM_TEL_INDICATION_INCOMING_CALL) {
    vm_gsm_tel_call_info_t *ind = (vm_gsm_tel_call_info_t *)data->data;
    g_uid_info.call_id = ind->uid_info.call_id;
    g_uid_info.group_id = ind->uid_info.group_id;
    g_uid_info.sim = ind->uid_info.sim;
    strcpy(g_number, (char *)ind->num_uri);
    g_call_status = RECEIVINGCALL;

    vm_log_info("incoming call");
  }

  else if (data->call_type == VM_GSM_TEL_INDICATION_OUTGOING_CALL) {
    vm_gsm_tel_call_info_t *ind = (vm_gsm_tel_call_info_t *)data->data;
    g_uid_info.call_id = ind->uid_info.call_id;
    g_uid_info.group_id = ind->uid_info.group_id;
    g_uid_info.sim = ind->uid_info.sim;
    strcpy(g_number, (char *)ind->num_uri);
    g_call_status = CALLING;

    vm_log_info("calling");
  }

  else if (data->call_type == VM_GSM_TEL_INDICATION_CONNECTED) {
    vm_gsm_tel_connect_indication_t *ind =
        (vm_gsm_tel_connect_indication_t *)data->data;
    g_uid_info.call_id = ind->uid_info.call_id;
    g_uid_info.group_id = ind->uid_info.group_id;
    g_uid_info.sim = ind->uid_info.sim;
    g_call_status = TALKING;

    vm_log_info("connected");
  }

  else if (data->call_type == VM_GSM_TEL_INDICATION_CALL_ENDED) {
    g_call_status = IDLE_CALL;
    vm_log_info("endded");
  }

  else {
    vm_log_info("bad operation type");
  }

  vm_log_info("g_call_status is %d", g_call_status);

  if (g_call_state_changed_callback) {
    g_call_state_changed_callback(g_call_status);
  }
}

void callregisterCallback(void (*call_state_changed_callback)(VMINT8)) {
  g_call_state_changed_callback = call_state_changed_callback;

  vm_gsm_tel_call_reg_listener(call_listener_func);
}

static void
call_voiceCall_callback(vm_gsm_tel_call_actions_callback_data_t *data) {
  vm_log_info("call_voiceCall_callback");

  if (data->type_action == VM_GSM_TEL_CALL_ACTION_DIAL) {

    if (data->data_act_rsp.result_info.result == VM_GSM_TEL_OK) {
      g_call_status = CALLING;
    } else {
      g_call_status = IDLE_CALL;
    }
  }

  else if (data->type_action == VM_GSM_TEL_CALL_ACTION_ACCEPT) {
    if (data->data_act_rsp.result_info.result == VM_GSM_TEL_OK) {
      g_call_status = TALKING;
    } else {
      g_call_status = IDLE_CALL;
    }
  }

  else if (data->type_action == VM_GSM_TEL_CALL_ACTION_HOLD) {

  }

  else if (data->type_action == VM_GSM_TEL_CALL_ACTION_END_SINGLE) {
    g_call_status = IDLE_CALL;
  } else {
  }
}

int _gsm_call(const char *phone_number) {
  vm_gsm_tel_dial_action_request_t req;
  vm_gsm_tel_call_actions_data_t data;

  vm_chset_ascii_to_ucs2((VMWSTR)req.num_uri, VM_GSM_TEL_MAX_NUMBER_LENGTH,
                         (VMSTR)phone_number);
  req.sim = VM_GSM_TEL_CALL_SIM_1;
  req.is_ip_dial = 0;
  req.module_id = 0;
  req.phonebook_data = NULL;

  data.action = VM_GSM_TEL_CALL_ACTION_DIAL;
  data.data_action = (void *)&req;
  data.user_data = NULL;
  data.callback = call_voiceCall_callback;

  return vm_gsm_tel_call_actions(&data);
}

int gsm_call(lua_State *L) {
  const char *phone_number = lua_tostring(L, -1);
  int result = _gsm_call(phone_number);
  lua_pushnumber(L, result);

  return 1;
}

int gsm_anwser(lua_State *L) {
  int result;
  vm_gsm_tel_single_call_action_request_t req;
  vm_gsm_tel_call_actions_data_t data;

  req.action_id.sim = g_uid_info.sim;
  req.action_id.call_id = g_uid_info.call_id;
  req.action_id.group_id = g_uid_info.group_id;

  data.action = VM_GSM_TEL_CALL_ACTION_ACCEPT;
  data.data_action = (void *)&req;
  data.user_data = NULL;
  data.callback = call_voiceCall_callback;

  result = vm_gsm_tel_call_actions(&data);

  lua_pushnumber(L, result);

  return 1;
}

int gsm_hang(lua_State *L) {
  int result = 0;
  vm_gsm_tel_single_call_action_request_t req;
  vm_gsm_tel_call_actions_data_t data;

  // vm_log_info("callhangCall");

  if (IDLE_CALL != g_call_status) {
	  //	  req.action_id.sim   = g_uid_info.sim;
	  //	  req.action_id.call_id = g_uid_info.call_id;
	  //	  req.action_id.group_id = g_uid_info.group_id;
	  req.action_id.sim = 1;
	  req.action_id.call_id = 1;
	  req.action_id.group_id = 1;

	  data.action = VM_GSM_TEL_CALL_ACTION_END_SINGLE;
	  data.data_action = (void *)&req;
	  data.user_data = NULL;
	  data.callback = call_voiceCall_callback;

	  result = vm_gsm_tel_call_actions(&data);


  }

  lua_pushnumber(L, result);

  return 1;
}


#undef MIN_OPT_LEVEL
#define MIN_OPT_LEVEL 0
#include "lrodefs.h"

const LUA_REG_TYPE gsm_map[] =
{
    {LSTRKEY("call"), LFUNCVAL(gsm_call)},
    {LSTRKEY("answer"), LFUNCVAL(gsm_anwser)},
    {LSTRKEY("hang"), LFUNCVAL(gsm_hang)},
    {LNILKEY, LNILVAL}
};

LUALIB_API int luaopen_gsm(lua_State *L)
{
	vm_gsm_tel_call_reg_listener(call_listener_func);

    luaL_register(L, "gsm", gsm_map);
    return 1;
}
