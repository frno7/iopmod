// SPDX-License-Identifier: GPL-2.0

MODULE_ID(dev9, 0x0100);
LIBRARY_ID(dev9, 0x0100);

/**
 * dev9_request - request DEV9 hardware access
 *
 * FIXME: The DEV9 hardware is powered on, if necessary.
 *
 * Return: zero on success, or a negative error number
 */
id_(0) int dev9_request();

/**
 * dev9_release - release DEV9 hardware access
 *
 * FIXME: The DEV9 hardware is powered off, if it's no longer used.
 *
 * Return: zero on success, or a negative error number
 */
id_(1) int dev9_release();
