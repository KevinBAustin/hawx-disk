//
// formatted console output -- printf, panic.
//

#include <stdarg.h>

#include "types.h"
#include "riscv.h"
#include "console.h"
#include "port.h"
#include "string.h"

volatile int panicked = 0;

static char digits[] = "0123456789abcdef";


static int get_padding(char *fmt, int *i)
{
  int padding = 0;
  int sign = 1;

  if (fmt[*i]  == '-') {
    sign = -1;
    (*i)++;
  }

  while (fmt[*i] >= '0' && fmt[*i] <= '9') {
    padding = padding * 10 + fmt[*i] - '0';
    (*i)++;
  }

  return padding * sign;
}


static void print_padding(int port, int padding, int len)
{
  if(padding < 0) {
    padding *= -1;
  }

  for (int i = 0; i < padding - len; i++)
    port_write(port, " ", 1);
}


static void
printint(int port, int xx, int base, int sign, int padding)
{
  char buf[16];
  int i;
  uint x;
  int len;

  if(sign && (sign = xx < 0))
    x = -xx;
  else
    x = xx;

  i = 0;
  do {
    buf[i++] = digits[x % base];
  } while((x /= base) != 0);

  if(sign)
    buf[i++] = '-';

  len = i;
  if(padding > 0)
    print_padding(port, padding, len);

  while(--i >= 0)
    port_write(port, buf + i, 1);

  if(padding < 0)
    print_padding(port, padding, len);
}

static void
printptr(int port, uint64 x, int padding)
{
  int i;
  char buf[16];

  for (i = 0; i < (sizeof(uint64) * 2); i++, x <<= 4)
    buf[i] = digits[x >> (sizeof(uint64) * 8 - 4)];

  if(padding > 0)
    print_padding(port, padding, i+2);

  port_write(port, "0x", 2);
  port_write(port, buf, i);

  if(padding < 0)
    print_padding(port, padding, i+2);
}


static void
printstr(int port, char *s, int padding)
{
  int len;

  if(s == 0)
    s = "(null)";
  len = strlen(s);

  if(padding > 0)
    print_padding(port, padding, len);

  while(*s)
    port_write(port, s++, 1);

  if(padding < 0)
    print_padding(port, padding, len);
}


static void 
printchar(int port, int c, int padding)
{
  if(padding > 0)
    print_padding(port, padding, 1);

  port_write(port, (char*)&c, 1);

  if(padding < 0)
    print_padding(port, padding, 1);
}


// Print to the console. only understands %d, %x, %p, %s.
static void
printf_driver(int port, char *fmt, va_list ap) 
{
  int i, c;
  int padding;

  if (fmt == 0)
    panic("null fmt");

  for(i = 0; (c = fmt[i] & 0xff) != 0; i++){
    if(c != '%'){
      port_write(port, (char*) &c, 1);
      continue;
    }
    i++;
    padding = get_padding(fmt, &i);
    c = fmt[i] & 0xff;
    if(c == 0)
      break;
    switch(c){
    case 'c':
      printchar(port, va_arg(ap, int), padding);
      break;
    case 'd':
      printint(port, va_arg(ap, int), 10, 1, padding);
      break;
    case 'x':
      printint(port, va_arg(ap, int), 16, 1, padding);
      break;
    case 'p':
      printptr(port, va_arg(ap, uint64), padding);
      break;
    case 's':
      printstr(port, va_arg(ap, char*), padding);
      break;
    case '%':
      printchar(port, '%', padding);
      break;
    default:
      // Print unknown % sequence to draw attention.
      port_write(port, "%", 1);
      port_write(port, (char*)&c, 1);
      break;
    }
  }
}

void printf(char *fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);
  printf_driver(PORT_CONSOLEOUT, fmt, ap);
  va_end(ap);
  uartstart();
}

void pprintf(int port, char *fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);
  printf_driver(port, fmt, ap);
  va_end(ap);
}

// Panic! Print one last plea for help and then hardlocked the kernel.
void
panic(char *s)
{
  const char *panic_msg = "panic: ";

  uartflush();
  while(*panic_msg) { uartputc(*panic_msg++); }
  while(*s) { uartputc(*s++); }

  uartputc('\n');
  panicked = 1; // freeze uart output from other CPUs
  for(;;)
    ;
}



// handy debug function for unit tests
void
print_pass(int passed)
{
    printf("%s\n", passed ? "PASSED" : "FAILED");
}
