// SPDX-License-Identifier: GPL-2.0

MODULE_ID(Stdio, 0x0102);
LIBRARY_ID(stdio, 0x0102);

id_(4) int stdio_printf(const char *format, ...)
	alias_(printf);
id_(5) int stdio_getchar(void)
	alias_(getchar);
id_(6) int stdio_putchar(int c)
	alias_(putchar);
id_(7) int stdio_puts(const char *s)
	alias_(puts);
id_(8) char * stdio_gets(char *s)
	alias_(gets);
id_(9) int stdio_fprintf(int fd, const char *format, ...)
	alias_(fdprintf);
id_(10) int stdio_fgetc(int fd)
	alias_(fdgetc);
id_(11) int stdio_fputc(int c, int fd)
	alias_(fdputc);
id_(12) int stdio_fputs(const char *s, int fd)
	alias_(fdputs);
id_(13) char * stdio_fgets(char *buf, int fd)
	alias_(fdgets);
id_(14) int stdio_vfprintf(int fd, const char *format, va_list ap)
	alias_(vfdprintf);
