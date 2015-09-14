#include <stdio.h>
#include <fcntl.h>
#if defined (  __GNUC__  ) /* GCC CS3 */
#include <sys/stat.h>
#endif
#include "vmtype.h"
#include "vmlog.h"
#include "vmmemory.h"
#include "vmdcl.h"
#include "vmdcl_gpio.h"
#include "vmdcl_pwm.h"
#include "vmfs.h"
#include "vmchset.h"
#include "console.h"


typedef VMINT (*vm_get_sym_entry_t)(char* symbol);
vm_get_sym_entry_t vm_get_sym_entry;

static unsigned int g_memory_size = 1024 * 200;
static void* g_base_address = NULL;

#define RESERVED_MEMORY_SIZE 300*1024


int __g_errno = 0;

 void __cxa_pure_virtual()
{
    while (1);
}

int * __errno()
{
	return &__g_errno;
}


extern caddr_t _sbrk ( int incr )
{
    static void* heap = NULL ;
    static void* base = NULL ;
    void* prev_heap ;

    if ( heap == NULL )
    {
        base = (unsigned char *)g_base_address;
				if(base == NULL)
				{
					vm_log_fatal("malloc failed");
				}
				else
				{
					heap = base;
					vm_log_info("init memory success");
				}
    }

    if (heap + incr > g_base_address + g_memory_size)
    {
        vm_log_fatal("memory not enough");
    }

    prev_heap = heap;

    heap += incr ;

    return (caddr_t) prev_heap ;
}

extern int link( char *cOld, char *cNew )
{
    return -1 ;
}

int _open(const char *file, int flags, int mode)
{
    VMUINT fs_mode;
    VMWCHAR wfile_name[16];

    vm_chset_ascii_to_ucs2(wfile_name, sizeof(wfile_name), file);

    if (flags & O_CREAT) {
        fs_mode = VM_FS_MODE_CREATE_ALWAYS_WRITE;
    } else if ((flags & O_RDWR) || (flags & O_WRONLY)) {
        fs_mode = VM_FS_MODE_WRITE;
    } else {
        fs_mode = VM_FS_MODE_READ;
    }

    if (flags & O_APPEND) {
        fs_mode |= VM_FS_MODE_APPEND;
    }

    return vm_fs_open(wfile_name, fs_mode, 0);
}

extern int _close( int file )
{
    vm_fs_close(file);
    return 0;
}

extern int _fstat( int file, struct stat *st )
{
    st->st_mode = S_IFCHR ;

    return 0 ;
}

extern int _isatty( int file )
{
    return 1 ;
}

extern int _lseek( int file, int ptr, int dir )
{
    return 0 ;
}

extern int _read(int file, char *ptr, int len)
{
    if (file < 3) {
        int i;
        for (i = 0; i < len; i++) {
            *ptr = console_getc();
            ptr++;
        }
        return len;
    } else {
        int read_bytes = len;
        int bytes;
        bytes = vm_fs_read(file, ptr, len, &read_bytes);
        return bytes;
    }
}

extern int _write( int file, char *ptr, int len )
{
    if (file < 3) {
        int i;

        for (i = 0; i < len; i++) {
            console_putc(*ptr);
            ptr++;
        }
        return len ;
    } else {
        VMUINT written_bytes;

        vm_fs_write(file, ptr, len, &written_bytes);

        return written_bytes;
    }
}

extern void _exit( int status )
{
    for ( ; ; ) ;
}

extern void _kill( int pid, int sig )
{
    return ;
}

extern int _getpid ( void )
{
    return -1 ;
}

int __cxa_guard_acquire(int *g) {return !*(char *)(g);};
void __cxa_guard_release (int *g) {*(char *)g = 1;};

typedef void (**__init_array) (void);

void __libc_init_array(void);

void gcc_entry(unsigned int entry, unsigned int init_array_start, unsigned int count)
{
	unsigned int i;
	__init_array ptr;
	unsigned int size = 0;
	vm_get_sym_entry = (vm_get_sym_entry_t)entry;

	size = vm_pmng_get_total_heap_size();

#if defined(__LINKIT_ONE__)
 	 if(size > RESERVED_MEMORY_SIZE)
	 {
		g_memory_size = size - RESERVED_MEMORY_SIZE;
 	 }
#endif

  g_base_address = vm_malloc(g_memory_size);

  ptr = (__init_array)init_array_start;
  for (i = 1; i < count; i++)
  {
  		ptr[i]();
  }
	vm_main();
}


#define VM_DCL_PIN_MODE_MAX 10

typedef struct{
   VMINT32 gpio;
   VMINT32 eint;
   VMINT32 adc;
   VMINT32 pwm;
   VMCHAR mux[VM_DCL_PIN_MODE_MAX];
}VM_DCL_PIN_MUX;

#if defined(__HDK_LINKIT_ONE_V1__)
#define VM_DCL_PIN_TABLE_SIZE 19
#elif defined(__HDK_LINKIT_ASSIST_2502__)
#define VM_DCL_PIN_TABLE_SIZE 21
#elif defined(__HDK_MT2502D_DEV_BOARD__)
#define VM_DCL_PIN_TABLE_SIZE 21
#else
#define VM_DCL_PIN_TABLE_SIZE 1
#endif

#if defined(__HDK_LINKIT_ONE_V1__)
const VM_DCL_PIN_MUX pinTable[VM_DCL_PIN_TABLE_SIZE] = {
	{D0, 0, 0, 0, {VM_DCL_PIN_MODE_GPIO, VM_DCL_PIN_MODE_UART, 0, 0 , 0, 0, 0, 0, 0, 0}},
	{D1, 0, 0, 0, {VM_DCL_PIN_MODE_GPIO, VM_DCL_PIN_MODE_UART, 0, 0 , 0, 0, 0, 0, 0, 0}},
  	{D2, 20, 0, 0,  {VM_DCL_PIN_MODE_GPIO, VM_DCL_PIN_MODE_PWM, VM_DCL_PIN_MODE_EINT, 0 , 0, 0, 0, 0, 0, 0}},
	{D3, 11, 0, VM_DCL_PWM_1,  {VM_DCL_PIN_MODE_GPIO, 0, VM_DCL_PIN_MODE_EINT, VM_DCL_PIN_MODE_PWM , 0, 0, 0, 0, 0, 0}},
	{D4, 0, 0, 0,  {VM_DCL_PIN_MODE_GPIO, 0, 0, 0 , 0, 0, 0, 0, 0, 0}},
 	{D5, 0, 0, 0, {VM_DCL_PIN_MODE_GPIO, 0, 0, 0 , 0, 0, 0, 0, 0, 0}},
	{D6, 0, 0, 0, {VM_DCL_PIN_MODE_GPIO, 0, 0, 0 , 0, 0, 0, 0, 0, 0}},
	{D7, 0, 0, 0, {VM_DCL_PIN_MODE_GPIO, 0, 0, 0 , 0, 0, 0, 0, 0, 0}},
  	{D8, 0, 0, 0, {VM_DCL_PIN_MODE_GPIO, 0, 0, 0 , 0, 0, 0, 0, 0, 0}},
	{D9, 0, 0, VM_DCL_PWM_4,{VM_DCL_PIN_MODE_GPIO, 0, VM_DCL_PIN_MODE_PWM, 0 , 0, 0, 0, 0, 0, 0}},
	{D10, 0, 0, 0, {VM_DCL_PIN_MODE_GPIO, 0, 0, 0 , VM_DCL_PIN_MODE_SPI, 0, 0, 0, 0, 0}},
  	{D11, 0, 0, 0, {VM_DCL_PIN_MODE_GPIO, 0, 0, 0 , VM_DCL_PIN_MODE_SPI, 0, 0, 0, VM_DCL_PIN_MODE_SDIO, 0}},
	{D12, 0, 0, 0, {VM_DCL_PIN_MODE_GPIO, 0, 0, 0 , VM_DCL_PIN_MODE_SPI, 0, 0, 0, VM_DCL_PIN_MODE_SDIO, 0}},
	{D13, 0, 0, 0, {VM_DCL_PIN_MODE_GPIO, 0, 0, 0 , VM_DCL_PIN_MODE_SPI, 0, 0, 0, VM_DCL_PIN_MODE_SDIO, 0}},
  	{D14, 0, 12, 0, {VM_DCL_PIN_MODE_GPIO, 0, VM_DCL_PIN_MODE_ADC, 0 , 0, 0, 0, 0, 0, 0}},
  	{D15, 0, 15, 0, {VM_DCL_PIN_MODE_GPIO, 0, VM_DCL_PIN_MODE_ADC, 0 , 0, 0, 0, 0, 0, 0}},
	{D16, 0, 13, 0, {VM_DCL_PIN_MODE_GPIO, 0, VM_DCL_PIN_MODE_ADC, 0 , 0, 0, 0, 0, 0, 0}},
	{D18, 0, 0, 0, {VM_DCL_PIN_MODE_GPIO, VM_DCL_PIN_MODE_I2C, 0, 0 , 0, 0, 0, 0, 0, 0}},
    {D19, 0, 0, 0, {VM_DCL_PIN_MODE_GPIO, VM_DCL_PIN_MODE_I2C, 0, 0 , 0, 0, 0, 0, 0, 0}}
};
#elif defined(__HDK_LINKIT_ASSIST_2502__)
static const VM_DCL_PIN_MUX pinTable[VM_DCL_PIN_TABLE_SIZE] = {
	{D0, 11, 0, VM_DCL_PWM_1, {VM_DCL_PIN_MODE_GPIO, 0, VM_DCL_PIN_MODE_EINT, VM_DCL_PIN_MODE_PWM , 0, 0, 0, 0, 0, 0}},
	{D1, 0, 0, VM_DCL_PWM_4, {VM_DCL_PIN_MODE_GPIO, 0, 0, 0 , VM_DCL_PIN_MODE_PWM, 0, 0, 0, 0, 0}},
    {D2, 0, 0, 0, {VM_DCL_PIN_MODE_GPIO, 0, 0, 0 , VM_DCL_PIN_MODE_SPI, 0, 0, 0, 0, 0}},
	{D3, 0, 0, 0, {VM_DCL_PIN_MODE_GPIO, 0, 0, 0 , VM_DCL_PIN_MODE_SPI, 0, 0, 0, 0, 0}},
	{D4, 0, 0, 0, {VM_DCL_PIN_MODE_GPIO, 0, 0, 0 , VM_DCL_PIN_MODE_SPI, 0, 0, 0, 0, 0}},
    {D5, 0, 0, 0, {VM_DCL_PIN_MODE_GPIO, 0, 0, 0 , VM_DCL_PIN_MODE_SPI, 0, 0, 0, 0, 0}},
	{D6, 0, 0, 0, {VM_DCL_PIN_MODE_GPIO, VM_DCL_PIN_MODE_I2C, 0, 0 , 0, 0, 0, 0, 0, 0}},
	{D7, 0, 0, 0, {VM_DCL_PIN_MODE_GPIO, VM_DCL_PIN_MODE_I2C, 0, 0 , 0, 0, 0, 0, 0, 0}},
    {D8, 0, 0, 0, {VM_DCL_PIN_MODE_GPIO, VM_DCL_PIN_MODE_UART, 0, 0 , 0, 0, 0, 0, 0, 0}},
	{D9, 0, 0, 0, {VM_DCL_PIN_MODE_GPIO, VM_DCL_PIN_MODE_UART, 0, 0 , 0, 0, 0, 0, 0, 0}},
	{D10, 0, 0, 0, {VM_DCL_PIN_MODE_GPIO, 0, 0, 0 , 0, 0, 0, 0, 0, 0}},
    {D11, 0, 0, 0, {VM_DCL_PIN_MODE_GPIO, 0, 0, 0 , 0, 0, 0, 0, 0, 0}},
	{D12, 0, 0, 0, {VM_DCL_PIN_MODE_GPIO, VM_DCL_PIN_MODE_DBI_C, 0, 0 , 0, 0, 0, 0, 0, 0}},
	{D13, 0, 0, 0, {VM_DCL_PIN_MODE_GPIO, VM_DCL_PIN_MODE_DBI_C, 0, 0 , 0, 0, 0, 0, 0, 0}},
    {D14, 0, 0, 0, {VM_DCL_PIN_MODE_GPIO, VM_DCL_PIN_MODE_DBI_C, 0, 0 , 0, 0, 0, 0, 0, 0}},
	{D15, 0, 0, 0, {VM_DCL_PIN_MODE_GPIO, VM_DCL_PIN_MODE_DBI_C, 0, 0 , 0, 0, 0, 0, 0, 0}},
	{A0, 0, 4, 0, {VM_DCL_PIN_MODE_GPIO, 0, VM_DCL_PIN_MODE_ADC, 0 , 0, 0, 0, 0, 0, 0}},
    {A1, 0, 12, 0, {VM_DCL_PIN_MODE_GPIO, VM_DCL_PIN_MODE_EINT, VM_DCL_PIN_MODE_ADC, 0 , 0, 0, 0, 0, 0, 0}},
    {A2, 1, 15, 0, {VM_DCL_PIN_MODE_GPIO, VM_DCL_PIN_MODE_EINT, VM_DCL_PIN_MODE_ADC, 0 , 0, 0, 0, 0, 0, 0}},
	{A3, 2, 13, 0,  {VM_DCL_PIN_MODE_GPIO, VM_DCL_PIN_MODE_EINT, VM_DCL_PIN_MODE_ADC, 0 , 0, 0, 0, 0, 0, 0}},
	{E0, 23, 0, 0, {VM_DCL_PIN_MODE_GPIO, 0, VM_DCL_PIN_MODE_EINT, 0 , 0, 0, 0, 0, 0, 0}}
};
#elif defined(__HDK_MT2502D_DEV_BOARD__)
static const VM_DCL_PIN_MUX pinTable[VM_DCL_PIN_TABLE_SIZE] = {
	{D0, 0, 0, 0, {VM_DCL_PIN_MODE_GPIO, 0, VM_DCL_PIN_MODE_EINT, VM_DCL_PIN_MODE_PWM , 0, 0, 0, 0, 0, 0}},
	{D1, 0, 0, 0, {VM_DCL_PIN_MODE_GPIO, 0, 0, 0 , VM_DCL_PIN_MODE_PWM, 0, 0, 0, 0, 0}},
  {D2, 0, 0, 0, {VM_DCL_PIN_MODE_GPIO, 0, 0, 0 , VM_DCL_PIN_MODE_SPI, 0, 0, 0, 0, 0}},
	{D3, 0, 0, 0, {VM_DCL_PIN_MODE_GPIO, 0, 0, 0 , VM_DCL_PIN_MODE_SPI, 0, 0, 0, 0, 0}},
	{D4, 0, 0, 0, {VM_DCL_PIN_MODE_GPIO, 0, 0, 0 , VM_DCL_PIN_MODE_SPI, 0, 0, 0, 0, 0}},
  {D5, 0, 0, 0, {VM_DCL_PIN_MODE_GPIO, 0, 0, 0 , VM_DCL_PIN_MODE_SPI, 0, 0, 0, 0, 0}},
	{D6, 0, 0, 0, {VM_DCL_PIN_MODE_GPIO, VM_DCL_PIN_MODE_I2C, 0, 0 , 0, 0, 0, 0, 0, 0}},
	{D7, 0, 0, 0, {VM_DCL_PIN_MODE_GPIO, VM_DCL_PIN_MODE_I2C, 0, 0 , 0, 0, 0, 0, 0, 0}},
  {D8, 0, 0, 0, {VM_DCL_PIN_MODE_GPIO, VM_DCL_PIN_MODE_UART, 0, 0 , 0, 0, 0, 0, 0, 0}},
	{D9, 0, 0, 0, {VM_DCL_PIN_MODE_GPIO, VM_DCL_PIN_MODE_UART, 0, 0 , 0, 0, 0, 0, 0, 0}},
	{D10, 0, 0, 0, {VM_DCL_PIN_MODE_GPIO, 0, 0, 0 , 0, 0, 0, 0, 0, 0}},
  {D11, 0, 0, 0, {VM_DCL_PIN_MODE_GPIO, 0, 0, 0 , 0, 0, 0, 0, 0, 0}},
	{D12, 0, 0, 0, {VM_DCL_PIN_MODE_GPIO, VM_DCL_PIN_MODE_DBI_C, 0, 0 , 0, 0, 0, 0, 0, 0}},
	{D13, 0, 0, 0, {VM_DCL_PIN_MODE_GPIO, VM_DCL_PIN_MODE_DBI_C, 0, 0 , 0, 0, 0, 0, 0, 0}},
  {D14, 0, 0, 0, {VM_DCL_PIN_MODE_GPIO, VM_DCL_PIN_MODE_DBI_C, 0, 0 , 0, 0, 0, 0, 0, 0}},
	{D15, 0, 0, 0, {VM_DCL_PIN_MODE_GPIO, VM_DCL_PIN_MODE_DBI_C, 0, 0 , 0, 0, 0, 0, 0, 0}},
	{A0, 0, 0, 0, {VM_DCL_PIN_MODE_GPIO, 0, 0, 0 , 0, 0, 0, 0, 0, 0}},
  {A1, 0, 0, 0, {VM_DCL_PIN_MODE_GPIO, 0, VM_DCL_PIN_MODE_ADC, 0 , 0, 0, 0, 0, 0, 0}},
  {A2, 0, 0, 0, {VM_DCL_PIN_MODE_GPIO, 0, VM_DCL_PIN_MODE_ADC, 0 , 0, 0, 0, 0, 0, 0}},
	{A3, 0, 0, 0,  {VM_DCL_PIN_MODE_GPIO, 0, VM_DCL_PIN_MODE_ADC, 0 , 0, 0, 0, 0, 0, 0}},
	{E0, 0, 0, 0, {VM_DCL_PIN_MODE_GPIO, 0, 0, 0 , 0, 0, 0, 0, 0, 0}}
};
#else
static const VM_DCL_PIN_MUX pinTable[VM_DCL_PIN_TABLE_SIZE] = {
	{D0, 0, 0, 0, {0, 0, 0, 0 , 0, 0, 0, 0, 0, 0}}
};
#endif

VM_DCL_STATUS vm_dcl_config_pin_mode(VMUINT pin, VM_DCL_PIN_MODE mode)
{
	VM_DCL_HANDLE gpio_handle;
	VMINT i, j;
	VM_DCL_STATUS status = VM_DCL_STATUS_FAIL;

	for(i = 0; i<VM_DCL_PIN_TABLE_SIZE; i++)
	{
		if(pinTable[i].gpio == pin)
		{
			break;
		}
	}
	if(i >=VM_DCL_PIN_TABLE_SIZE)
		return status;

	gpio_handle = vm_dcl_open(VM_DCL_GPIO,pin);

	for(j = 0; j<VM_DCL_PIN_MODE_MAX; j++)
	{
		if(pinTable[i].mux[j] == mode)
		{
			vm_dcl_control(gpio_handle,4+j,NULL);
			status = VM_DCL_STATUS_OK;
			break;
		}
	}
	vm_dcl_close(gpio_handle);
	return status;
}

VMUINT vm_dcl_pin_to_pwm(VMUINT pin)
{
	VMINT i;

	for(i = 0; i<VM_DCL_PIN_TABLE_SIZE; i++)
	{
		if(pinTable[i].gpio == pin)
		{
			return pinTable[i].pwm;
		}
	}

	return 0;
}

VMUINT vm_dcl_pin_to_channel(VMUINT pin)
{
	VMINT i;

	for(i = 0; i<VM_DCL_PIN_TABLE_SIZE; i++)
	{
		if(pinTable[i].gpio == pin)
		{
			return pinTable[i].adc;
		}
	}

	return 0;
}

VMUINT vm_dcl_pin_to_eint(VMUINT pin)
{
	VMINT i;

	for(i = 0; i<VM_DCL_PIN_TABLE_SIZE; i++)
	{
		if(pinTable[i].gpio == pin)
		{
			return pinTable[i].eint;
		}
	}

	return 0;
}
