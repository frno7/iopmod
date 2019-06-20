// SPDX-License-Identifier: GPL-2.0

MODULE_ID(printk, 0x0100);
LIBRARY_ID(printk, 0x0100);

/**
 * printk - print formated message in the main processor kernel log
 * @fmt: format of message to print
 * @...: additional arguments corresponding to the format
 *
 * At most %CMD_PACKET_PAYLOAD_MAX-1 characters are printed. Any remaining
 * characters are truncated.
 *
 * Context: any
 * Return: number of characters printed, or a negative error number
 */
id_(0) int printk(const char *fmt, ...) __attribute__((format(printf, 1, 2)));
