#include <stdint.h>
#include <sel4cp.h>

#define SET_TIME_WRITE_OFFSET 0x000         /* width 32 */
#define SET_TIME_READ_OFFSET 0x004          /* width 32, ro */
#define CALIB_WRITE_OFFSET 0x008            /* width 21 */
#define CALIB_READ_OFFSET 0x00C             /* width 21, ro */
#define CURRENT_TIME_OFFSET 0x010           /* width 32 */
#define ALARM_OFFSET 0x018                  /* width 32 */
#define RTC_INT_STATUS_OFFSET 0x020         /* width 2 */
#define RTC_INT_MASK_OFFSET 0x024           /* width 2, ro */
#define RTC_INT_ENABLE_OFFSET 0x028         /* width 2 */
#define RTC_INT_DISABLE_OFFSET 0x02C        /* width 2 */
#define ADDR_ERROR_OFFSET 0x030             /* width 1 */
#define ADDR_ERROR_INT_MASK_OFFSET 0x034    /* width 1, ro */
#define ADDR_ERROR_INT_ENABLE_OFFSET 0x038  /* width 1 */
#define ADDR_ERROR_INT_DISABLE_OFFSET 0x03C /* width 1 */
#define CONTROL_OFFSET 0x040                /* width 32 */
#define SAFETY_CHK_OFFSET 0x050             /* width 32 */

#define RTC_ALARM_INTERRUPT 8
#define RTC_SECONDS_INTERRUPT 9
#define PRINTER_CHANNEL 1


static char hexchar(unsigned int v)
{
    return v < 10 ? '0' + v : ('a' - 10) + v;
}

static void puthex32(uint32_t x)
{
    char buffer[11];
    buffer[0] = '0';
    buffer[1] = 'x';
    buffer[2] = hexchar((x >> 28) & 0xf);
    buffer[3] = hexchar((x >> 24) & 0xf);
    buffer[4] = hexchar((x >> 20) & 0xf);
    buffer[5] = hexchar((x >> 16) & 0xf);
    buffer[6] = hexchar((x >> 12) & 0xf);
    buffer[7] = hexchar((x >> 8) & 0xf);
    buffer[8] = hexchar((x >> 4) & 0xf);
    buffer[9] = hexchar(x & 0xf);
    buffer[10] = 0;
    sel4cp_dbg_puts(buffer);
}

/*****************************************************************************/

uintptr_t rtclock_vaddr;

uint32_t read_time()
{
    return *(volatile uint32_t*) (rtclock_vaddr+CURRENT_TIME_OFFSET);
}

void set_time(uint32_t to)
{
    *(volatile uint32_t*) (rtclock_vaddr+SET_TIME_WRITE_OFFSET) = to;
}

void set_alarm(uint32_t seconds)
{
    uint32_t target = read_time() + seconds;
    *(volatile uint32_t*) (rtclock_vaddr+ALARM_OFFSET) = target;
}

void init_time()
{
    // Xilinx Zynq UltraScale+ user guide p178:
    // Init RTC programming sequence
    volatile uint32_t* rtclock_calib_write =
      (volatile uint32_t*) (rtclock_vaddr+CALIB_WRITE_OFFSET);
    rtclock_calib_write[0] = 0x00198231;
    volatile uint32_t* rtclock_control =
      (volatile uint32_t*) (rtclock_vaddr+CONTROL_OFFSET);
    rtclock_control[0] = 0x01000000;
    volatile uint8_t* rtclock_int_status =
      (volatile uint8_t*) (rtclock_vaddr+RTC_INT_STATUS_OFFSET);
    rtclock_int_status[0] &= 0xFC; 
    volatile uint32_t* rtclock_int_disable =
      (volatile uint32_t*) (rtclock_vaddr+RTC_INT_DISABLE_OFFSET);
    rtclock_int_disable[0] = 0x03;
    volatile uint32_t* rtclock_int_enable =
      (volatile uint32_t*) (rtclock_vaddr+RTC_INT_ENABLE_OFFSET);
    rtclock_int_enable[0] = 0x03;
}



/*****************************************************************************/

void init(void)
{
    init_time();
    sel4cp_dbg_puts("timer: started\n");

    sel4cp_dbg_puts("timer: reading time\n");
    uint32_t current_time = read_time();
    sel4cp_dbg_puts("timer: read ");
    puthex32(current_time);
    sel4cp_dbg_puts("\n");

    sel4cp_dbg_puts("timer: setting time\n");
    set_time(0xDEAFBABE);

    sel4cp_dbg_puts("timer: reading time\n");
    current_time = read_time();
    sel4cp_dbg_puts("timer: read ");
    puthex32(current_time);
    sel4cp_dbg_puts("\n");

    sel4cp_dbg_puts("timer: setting alarm for 7 seconds\n");
    set_alarm(7);
}

void notified(sel4cp_channel ch)
{
    uint32_t current_time = 0xDEAFBABE;
    switch(ch) {
        case RTC_ALARM_INTERRUPT:
            sel4cp_dbg_puts("timer: notified of RTC_ALARM\n");
            sel4cp_dbg_puts("timer: reading time\n");
            current_time = read_time();
            sel4cp_dbg_puts("timer: read ");
            puthex32(current_time);
            sel4cp_dbg_puts("\n");
            sel4cp_dbg_puts("timer: setting alarm for 7 seconds\n");
            set_alarm(7);
            sel4cp_irq_ack(ch);
            break;
        case RTC_SECONDS_INTERRUPT:
            sel4cp_dbg_puts("timer: notified of RTC_SECONDS\n");
            sel4cp_dbg_puts("timer: reading time\n");
            current_time = read_time();
            sel4cp_dbg_puts("timer: read ");
            puthex32(current_time);
            sel4cp_dbg_puts("\n");
            sel4cp_irq_ack(ch);
            break;
        default:
            sel4cp_dbg_puts("timer: unknown notification fault\n");
            break;
    }
}
