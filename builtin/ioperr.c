// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2019 Fredrik Noring
 */

#include "iopmod/errno.h"
#include "iopmod/iop-error.h"

/**
 * errno_for_iop_error - kernel error number corresponding to a given IOP error
 * @ioperr: IOP error number
 *
 * Return: approximative kernel error number
 */
int errno_for_iop_error(int ioperr)
{
	switch (ioperr) {
#define IOP_ERROR_ERRNO(identifier, number, errno, description)		\
	case -IOP_E##identifier: return -errno;
	IOP_ERRORS(IOP_ERROR_ERRNO)
	}

	return -1000 < ioperr && ioperr < 0 ? -EINVAL : ioperr;
}

/**
 * iop_error_message - message corresponding to a given IOP error
 * @ioperr: IOP error number
 *
 * Return: error message string
 */
const char *iop_error_message(int ioperr)
{
	switch (ioperr) {
	case 0:              return "Success";
	case 1:              return "Error";
#define IOP_ERROR_MSG(identifier, number, errno, description)		\
	case IOP_E##identifier: return description;
	IOP_ERRORS(IOP_ERROR_MSG)
	}

	return "Unknown error";
}
