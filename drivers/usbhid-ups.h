/* usbhid-ups.h - Driver for serial/USB HID UPS units
 *
 * Copyright (C)
 *  2003-2005 Arnaud Quette <http://arnaud.quette.free.fr/contact.html>
 *  2005-2006 Peter Selinger <selinger@users.sourceforge.net>
 *  2007      Arjen de Korte <adkorte-guest@alioth.debian.org>
 *
 * This program is sponsored by MGE UPS SYSTEMS - opensource.mgeups.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 */

#ifndef USBHID_UPS_H
#define USBHID_UPS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "config.h"
#include "libhid.h"

#define DRIVER_VERSION		"0.32"

extern hid_dev_handle_t	udev;

/* --------------------------------------------------------------- */
/*      Model Name formating entries                               */
/* --------------------------------------------------------------- */

typedef struct
{
	char	*iProduct;
	char	*iModel;
	int	comp_size;	/* size of the comparison, -1 for full */
	char	*finalname;
} models_name_t;

/* Driver's parameters */
#define HU_VAR_ONDELAY		"ondelay"
#define HU_VAR_OFFDELAY		"offdelay"
#define HU_VAR_POLLFREQ		"pollfreq"

/* Parameters default values */
#define DEFAULT_ONDELAY		30	/* Delay between return of utility power */
					/* and powering up of load, in seconds */
					/* CAUTION: ondelay > offdelay */
#define DEFAULT_OFFDELAY	20	/* Delay before power off, in seconds */
#define DEFAULT_POLLFREQ	30	/* Polling interval, in seconds */
					/* The driver will wait for Interrupt */
					/* and do "light poll" in the meantime */

#define MAX_STRING_SIZE    	128


/* --------------------------------------------------------------- */
/* Struct & data for lookup between HID and NUT values             */
/* (From USB/HID, Power Devices Class standard)                    */
/* --------------------------------------------------------------- */

typedef struct {
	long	hid_value;	/* HID value */
	char	*nut_value;	/* NUT value */
        char    *(*fun)(long value); /* special case: if fun!=NULL, then
				     ignore hid_value and nut_value,
				     and use the conversion function
				     instead. This is used for more
				     complex formatting such as
				     dates. Fun is expected to return
				     a statically allocated string. */
} info_lkp_t;

/* declarations of public lookup tables */
/* boolean status values from UPS */
extern info_lkp_t online_info[];
extern info_lkp_t discharging_info[];
extern info_lkp_t charging_info[];
extern info_lkp_t lowbatt_info[];
extern info_lkp_t overload_info[];
extern info_lkp_t replacebatt_info[];
extern info_lkp_t trim_info[];
extern info_lkp_t boost_info[];
extern info_lkp_t bypass_info[];
extern info_lkp_t off_info[];
extern info_lkp_t calibration_info[];
extern info_lkp_t nobattery_info[];
extern info_lkp_t fanfail_info[];
extern info_lkp_t shutdownimm_info[];
extern info_lkp_t overheat_info[];
extern info_lkp_t awaitingpower_info[];
extern info_lkp_t commfault_info[];
extern info_lkp_t timelimitexpired_info[];
extern info_lkp_t battvoltlo_info[];
extern info_lkp_t battvolthi_info[];
extern info_lkp_t chargerfail_info[];
extern info_lkp_t emergency_stop_info[];
extern info_lkp_t fullycharged_info[];
extern info_lkp_t depleted_info[];

/* input.transfer.reason */
extern info_lkp_t vrange_info[];
extern info_lkp_t frange_info[];

/* non specific */
extern info_lkp_t test_write_info[];
extern info_lkp_t test_read_info[];
extern info_lkp_t beeper_info[];
extern info_lkp_t yes_no_info[];
extern info_lkp_t on_off_info[];
extern info_lkp_t date_conversion[];
extern info_lkp_t hex_conversion[];
extern info_lkp_t stringid_conversion[];
extern info_lkp_t divide_by_10_conversion[];
extern info_lkp_t kelvin_celsius_conversion[];

/* --------------------------------------------------------------- */
/* Structure containing information about how to get/set data      */
/* from/to the UPS and convert these to/from NUT standard          */
/* --------------------------------------------------------------- */

typedef struct {
	char	*info_type;		/* NUT variable name */
	int	info_flags;		/* NUT flags (to set in addinfo) */
	int	info_len;		/* if ST_FLAG_STRING: length of the string */
					/* if HU_TYPE_CMD: command value */
	char	*hidpath;		/* Full HID Object path (or NULL for server side vars) */
	HIDData_t *hiddata;		/* Full HID Object data (for caching purpose, filled at runtime) */
	char	*dfl;			/* if HU_FLAG_ABSENT: default value ; format otherwise */
	unsigned long hidflags;		/* driver's own flags */
	info_lkp_t *hid2info;		/* lookup table between HID and NUT values */

/*	char *info_HID_format;	*//* FFE: HID format for complex values */
/*	interpreter interpret;	*//* FFE: interpreter fct, NULL if not needed  */
/*	void *next;			*//* next hid_info_t */
} hid_info_t;

/* Data walk modes */
#define HU_WALKMODE_INIT		1
#define HU_WALKMODE_QUICK_UPDATE	2
#define HU_WALKMODE_FULL_UPDATE		3
#define HU_WALKMODE_RESET		4

/* TODO: rework flags */
#define HU_FLAG_OK			1		/* show element to upsd. */
#define HU_FLAG_STATIC			2		/* retrieve info only once. */
#define HU_FLAG_SEMI_STATIC		4		/* retrieve info smartly */
#define HU_FLAG_ABSENT			8		/* data is absent in the device, */
							/* use default value. */
#define HU_FLAG_QUICK_POLL		16		/* Mandatory vars	*/
#define HU_FLAG_STALE			32		/* data stale, don't try too often. */

/* hints for su_ups_set, applicable only to rw vars */
#define HU_TYPE_CMD				64		/* instant command */

#define HU_CMD_MASK		0x2000

#define HU_INFOSIZE		128

#define MAX_TRY		2		/* max number of GetItem retry */

/* --------------------------------------------------------------- */
/*      Subdriver interface                                        */
/* --------------------------------------------------------------- */

/* subdriver structure. Each subdriver is intended to support a
 * particular manufacturer (e.g. MGE, APC, Belkin), or a particular
 * range of models. */

struct subdriver_s {
	char *name;                  /* name of this subdriver */
	int (*claim)(HIDDevice_t *hd); /* return 1 if device covered by
				      * this subdriver */
	usage_tables_t *utab;        /* points to array of usage tables */
	hid_info_t *hid2nut;         /* main table of vars and instcmds */
	int (*shutdown)(int ondelay, int offdelay);
                                     /* driver-specific shutdown cmd.
					Returns 1 on success, 0 on
					failure */
	char *(*format_model)(HIDDevice_t *hd);  /* driver-specific methods */
	char *(*format_mfr)(HIDDevice_t *hd);    /* for preparing human-    */
	char *(*format_serial)(HIDDevice_t *hd); /* readable information    */
};
typedef struct subdriver_s subdriver_t;


/* the following functions are exported for the benefit of subdrivers */
int instcmd(const char *cmdname, const char *extradata);
int setvar(const char *varname, const char *val);

#endif /* USBHID_UPS_H */
