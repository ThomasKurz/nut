/* netget.c - GET handlers for upsd

   Copyright (C) 2003  Russell Kroll <rkroll@exploits.org>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/

#include "common.h"

#include "upsd.h"
#include "sstate.h"
#include "state.h"
#include "desc.h"
#include "neterr.h"

#include "netget.h"

static void get_numlogins(ctype *client, const char *upsname)
{
	const	upstype	*ups;

	ups = get_ups_ptr(upsname);

	if (!ups) {
		send_err(client, NUT_ERR_UNKNOWN_UPS);
		return;
	}

	if (!ups_available(ups, client))
		return;

	sendback(client, "NUMLOGINS %s %d\n", upsname, ups->numlogins);
}

static void get_upsdesc(ctype *client, const char *upsname)
{
	const	upstype	*ups;
	char	esc[SMALLBUF];

	ups = get_ups_ptr(upsname);

	if (!ups) {
		send_err(client, NUT_ERR_UNKNOWN_UPS);
		return;
	}

	if (!ups_available(ups, client))
		return;

	if (ups->desc) {
		pconf_encode(ups->desc, esc, sizeof(esc));
		sendback(client, "UPSDESC %s \"%s\"\n", upsname, esc);

	} else {

		sendback(client, "UPSDESC %s \"Unavailable\"\n", upsname);
	}
}

static void get_desc(ctype *client, const char *upsname, const char *var)
{
	const	upstype	*ups;
	const	char	*desc;

	ups = get_ups_ptr(upsname);

	if (!ups) {
		send_err(client, NUT_ERR_UNKNOWN_UPS);
		return;
	}

	if (!ups_available(ups, client))
		return;

	desc = desc_get_var(var);

	if (desc)
		sendback(client, "DESC %s %s \"%s\"\n", upsname, var, desc);
	else
		sendback(client, "DESC %s %s \"Unavailable\"\n", upsname, var);
}

static void get_cmddesc(ctype *client, const char *upsname, const char *cmd)
{
	const	upstype	*ups;
	const	char	*desc;

	ups = get_ups_ptr(upsname);

	if (!ups) {
		send_err(client, NUT_ERR_UNKNOWN_UPS);
		return;
	}

	if (!ups_available(ups, client))
		return;

	desc = desc_get_cmd(cmd);

	if (desc)
		sendback(client, "CMDDESC %s %s \"%s\"\n", upsname, cmd, desc);
	else
		sendback(client, "CMDDESC %s %s \"Unavailable\"\n", 
			upsname, cmd);
}

static void get_type(ctype *client, const char *upsname, const char *var)
{
	char	buf[SMALLBUF];
	const	upstype	*ups;
	const	struct	st_tree_t	*node;

	ups = get_ups_ptr(upsname);

	if (!ups) {
		send_err(client, NUT_ERR_UNKNOWN_UPS);
		return;
	}

	if (!ups_available(ups, client))
		return;

	node = sstate_getnode(ups, var);

	if (!node) {
		send_err(client, NUT_ERR_VAR_NOT_SUPPORTED);
		return;
	}

	snprintf(buf, sizeof(buf), "TYPE %s %s", upsname, var);

	if (node->flags & ST_FLAG_RW)
		snprintfcat(buf, sizeof(buf), " RW");

	if (node->enum_list) {
		sendback(client, "%s ENUM\n", buf);
		return;
	}

	if (node->flags & ST_FLAG_STRING) {
		sendback(client, "%s STRING:%d\n", buf, node->aux);
		return;
	}

	/* hmm... */

	sendback(client, "TYPE %s %s UNKNOWN\n", upsname, var);
}		

static void get_var_server(ctype *client, const char *upsname, const char *var)
{
	if (!strcasecmp(var, "server.info")) {
		sendback(client, "VAR %s server.info "
			"\"Network UPS Tools upsd %s - "
			"http://www.networkupstools.org/\"\n", 
			upsname, UPS_VERSION);
		return;
	}

	if (!strcasecmp(var, "server.version")) {
		sendback(client, "VAR %s server.version \"%s\"\n", 
			upsname, UPS_VERSION);
		return;
	}

	send_err(client, NUT_ERR_VAR_NOT_SUPPORTED);
}

static void get_var(ctype *client, const char *upsname, const char *var)
{
	const	upstype	*ups;
	const	char	*val;

	/* ignore upsname for server.* variables */
	if (!strncasecmp(var, "server.", 7)) {
		get_var_server(client, upsname, var);
		return;
	}

	ups = get_ups_ptr(upsname);

	if (!ups) {
		send_err(client, NUT_ERR_UNKNOWN_UPS);
		return;
	}

	if (!ups_available(ups, client))
		return;

	val = sstate_getinfo(ups, var);

	if (!val) {
		send_err(client, NUT_ERR_VAR_NOT_SUPPORTED);
		return;
	}

	/* handle special case for status */
	if ((!strcasecmp(var, "ups.status")) && (ups->fsd))
		sendback(client, "VAR %s %s \"FSD %s\"\n", upsname, var, val);
	else
		sendback(client, "VAR %s %s \"%s\"\n", upsname, var, val);
}

void net_get(ctype *client, int numarg, const char **arg)
{
	if (numarg < 2) {
		send_err(client, NUT_ERR_INVALID_ARGUMENT);
		return;
	}

	/* GET NUMLOGINS UPS */
	if (!strcasecmp(arg[0], "NUMLOGINS")) {
		get_numlogins(client, arg[1]);
		return;
	}

	/* GET UPSDESC UPS */
	if (!strcasecmp(arg[0], "UPSDESC")) {
		get_upsdesc(client, arg[1]);
		return;
	}

	if (numarg < 3) {
		send_err(client, NUT_ERR_INVALID_ARGUMENT);
		return;
	}

	/* GET VAR UPS VARNAME */
	if (!strcasecmp(arg[0], "VAR")) {
		get_var(client, arg[1], arg[2]);
		return;
	}

	/* GET TYPE UPS VARNAME */
	if (!strcasecmp(arg[0], "TYPE")) {
		get_type(client, arg[1], arg[2]);
		return;
	}

	/* GET DESC UPS VARNAME */
	if (!strcasecmp(arg[0], "DESC")) {
		get_desc(client, arg[1], arg[2]);
		return;
	}

	/* GET CMDDESC UPS CMDNAME */
	if (!strcasecmp(arg[0], "CMDDESC")) {
		get_cmddesc(client, arg[1], arg[2]);
		return;
	}

	send_err(client, NUT_ERR_INVALID_ARGUMENT);
	return;
}
