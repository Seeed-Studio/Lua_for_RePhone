
#include <string.h>
#include "vmdcl.h"
#include "vmdcl_sio.h"
#include "vmboard.h"
#include "vmthread.h"
#include "vmlog.h"

#define SERIAL_BUFFER_SIZE      64

/* Module owner of APP */
static VM_DCL_OWNER_ID g_owner_id = 0;
static VM_DCL_HANDLE console_device_handle = -1;
VM_SIGNAL_ID console_rx_signal_id;
static char console_rx_buffer[SERIAL_BUFFER_SIZE];
unsigned console_rx_buffer_head = 0;
unsigned console_rx_buffer_tail = 0;

void __console_irq_handler(void* parameter, VM_DCL_EVENT event, VM_DCL_HANDLE device_handle)
{
    if(event == VM_DCL_SIO_UART_READY_TO_READ)
    {
        char data[SERIAL_BUFFER_SIZE];
        int i;
        VM_DCL_STATUS status;
        VM_DCL_BUFFER_LENGTH returned_len = 0;

        status = vm_dcl_read(device_handle,
                             (VM_DCL_BUFFER *)data,
                             SERIAL_BUFFER_SIZE,
                             &returned_len,
                             g_owner_id);
        if(status < VM_DCL_STATUS_OK)
        {
            // vm_log_info((char*)"read failed");
        }
        else if (returned_len)
        {
            if (console_rx_buffer_head == console_rx_buffer_tail) {
                vm_signal_post(console_rx_signal_id);
            }

            for (i = 0; i < returned_len; i++)
            {
                console_rx_buffer[console_rx_buffer_head % SERIAL_BUFFER_SIZE] = data[i];
                console_rx_buffer_head++;
                if ((unsigned)(console_rx_buffer_head - console_rx_buffer_tail) > SERIAL_BUFFER_SIZE) {
                    console_rx_buffer_tail = console_rx_buffer_head - SERIAL_BUFFER_SIZE;
                }
            }
        }

    }
    else
    {
    }
}

void console_init(void)
{
    VM_DCL_HANDLE uart_handle;
    vm_dcl_sio_control_dcb_t settings;

    if (console_device_handle != -1)
    {
        return;
    }

    vm_dcl_config_pin_mode(D8, VM_DCL_PIN_MODE_UART);
    vm_dcl_config_pin_mode(D9, VM_DCL_PIN_MODE_UART);

    g_owner_id = vm_dcl_get_owner_id();
    uart_handle = vm_dcl_open(VM_DCL_SIO_UART_PORT1, g_owner_id);
    settings.owner_id = g_owner_id;
    settings.config.dsr_check = 0;
    settings.config.data_bits_per_char_length = VM_DCL_SIO_UART_BITS_PER_CHAR_LENGTH_8;
    settings.config.flow_control = VM_DCL_SIO_UART_FLOW_CONTROL_NONE;
    settings.config.parity = VM_DCL_SIO_UART_PARITY_NONE;
    settings.config.stop_bits = VM_DCL_SIO_UART_STOP_BITS_1;
    settings.config.baud_rate = VM_DCL_SIO_UART_BAUDRATE_115200;
    settings.config.sw_xoff_char = 0x13;
    settings.config.sw_xon_char = 0x11;
    vm_dcl_control(uart_handle, VM_DCL_SIO_COMMAND_SET_DCB_CONFIG, (void *)&settings);

    console_rx_signal_id = vm_signal_create();

    vm_dcl_register_callback(uart_handle,
                             VM_DCL_SIO_UART_READY_TO_READ,
                             (vm_dcl_callback)__console_irq_handler,
                             (void*)NULL);

    console_device_handle = uart_handle;
}

void console_putc(char ch)
{
    VM_DCL_BUFFER_LENGTH writen_len = 0;
    vm_dcl_write(console_device_handle, (VM_DCL_BUFFER *)&ch, 1, &writen_len, g_owner_id);
}

void console_puts(const char *str)
{
    VM_DCL_BUFFER_LENGTH writen_len = 0;
    VM_DCL_BUFFER_LENGTH len = strlen(str);

    vm_dcl_write(console_device_handle, (VM_DCL_BUFFER *)str, len, &writen_len, g_owner_id);
}

int console_getc(void)
{
    char ch;
    if (console_rx_buffer_head == console_rx_buffer_tail)
    {
        vm_signal_wait(console_rx_signal_id);
    }


    ch = console_rx_buffer[console_rx_buffer_tail % SERIAL_BUFFER_SIZE];
    console_rx_buffer_tail++;

    return ch;
}
