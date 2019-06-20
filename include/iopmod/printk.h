// SPDX-License-Identifier: GPL-2.0

#ifndef IOPMOD_PRINTK_H
#define IOPMOD_PRINTK_H

#include "iopmod/module-prototype.h"
#include "iopmod/module/printk.h"

/**
 * putk - print message in the main processor kernel log
 * @msg: message to print
 *
 * At most %CMD_PACKET_PAYLOAD_MAX-1 characters are printed. Any remaining
 * characters are truncated.
 *
 * Context: any
 * Return: number of characters printed, or a negative error number
 */
int putk(const char *msg);

#define KERN_SOH	"\001"		/* ASCII start of header */

#define KERN_EMERG	KERN_SOH "0"	/* System is unusable */
#define KERN_ALERT	KERN_SOH "1"	/* Action must be taken immediately */
#define KERN_CRIT	KERN_SOH "2"	/* Critical conditions */
#define KERN_ERR	KERN_SOH "3"	/* Error conditions */
#define KERN_WARNING	KERN_SOH "4"	/* Warning conditions */
#define KERN_NOTICE	KERN_SOH "5"	/* Normal but significant condition */
#define KERN_INFO	KERN_SOH "6"	/* Informational */
#define KERN_DEBUG	KERN_SOH "7"	/* Debug-level messages */

#define KERN_DEFAULT	KERN_SOH "d"	/* The default kernel loglevel */

/*
 * Dummy printk for disabled debugging statements to use whilst maintaining
 * GCC's format checking.
 */
#define no_printk(fmt, ...)						\
	({								\
		if (0)							\
			printk(fmt, ##__VA_ARGS__);			\
		0;							\
	})

#define pr_fmt(fmt) fmt

/*
 * These can be used to print at the various log levels. All of these will
 * print unconditionally, although note that pr_debug() and other debug macros
 * are compiled out unless either DEBUG is defined.
 */
#define pr_emerg(fmt, ...) \
	printk(KERN_EMERG pr_fmt(fmt), ##__VA_ARGS__)
#define pr_alert(fmt, ...) \
	printk(KERN_ALERT pr_fmt(fmt), ##__VA_ARGS__)
#define pr_crit(fmt, ...) \
	printk(KERN_CRIT pr_fmt(fmt), ##__VA_ARGS__)
#define pr_err(fmt, ...) \
	printk(KERN_ERR pr_fmt(fmt), ##__VA_ARGS__)
#define pr_warn(fmt, ...) \
	printk(KERN_WARNING pr_fmt(fmt), ##__VA_ARGS__)
#define pr_notice(fmt, ...) \
	printk(KERN_NOTICE pr_fmt(fmt), ##__VA_ARGS__)
#define pr_info(fmt, ...) \
	printk(KERN_INFO pr_fmt(fmt), ##__VA_ARGS__)

#if defined(DEBUG)
#define pr_debug(fmt, ...) \
	printk(KERN_DEBUG pr_fmt(fmt), ##__VA_ARGS__)
#else
#define pr_debug(fmt, ...) \
	no_printk(KERN_DEBUG pr_fmt(fmt), ##__VA_ARGS__)
#endif

#endif /* IOPMOD_PRINTK_H */
