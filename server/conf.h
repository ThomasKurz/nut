/* conf.h - supporting elements of conf parsing functions for upsd

   Copyright (C) 2001  Russell Kroll <rkroll@exploits.org>

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

/* startup: read all config files (upsd.conf, ups.conf, upsd.users) */
void conf_load(void);

/* flush existing config, then reread everything */
void conf_reload(void);

typedef struct {
	char	*upsname;
	char	*driver;
	char	*port;
	char	*desc;
	void	*next;
}	ups_t;

/* link to user.c for flushing during reloads */
void user_flush(void);

/* used for really clean shutdowns */
void delete_acls(void);
void delete_access(void);

	extern	int	num_ups;
