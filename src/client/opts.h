/*
	Copyright (C) 2011 Róbert Vašek <gman@codefreax.org>

    This file is part of fsync.

    fsync is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    fsync is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with fsync.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef OPTS_H
#define OPTS_H


#define PACKAGE "fsync-client (fsync)"
#define VERSION "1.0.0"
#define HELP \
	"\t-h  --help\t\t\tShow this message.\n" \
	"\t-v  --version\t\t\tShow version.\n" \
	"\t-s host  --host\t\t\tSet host.\n" \
	"\t-p port  --port\t\t\tSet port.\n" \
	"\t-c config file  --config-file\tRead settings from user specified config file.\n"

#define OPT "hvs:p:"
#define OPT_HELP "[-hv] [-s HOST] [-p PORT] [-c PATH TO CONFIG FILE]"
#define LOG_FILE "fsync_client.log"
#define CONF_FILE "client.conf"


#endif // OPTS_H
