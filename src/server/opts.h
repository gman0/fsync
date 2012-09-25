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


#define PACKAGE "fsync-server (fsync)"
#define VERSION "1.0.0"
#define HELP \
	"\t-h  --help\t\t\tShow this message.\n" \
	"\t-v  --version\t\t\tShow version.\n" \
	"\t-p port  --port\t\t\tSet port.\n" \
	"\t-c config file  --config-file\tRead settings from user specified config file.\n" \
	"\t-d database file  --db-file\tRead file database from user specified file.\n" \
	"\t-a  --dont-save-db\t\tDon't save file database on exit.\n" \
	"\t-u  --update-db\t\t\tUpdate file database and quit.\n" \
	"\t-i  --ignore-db\t\t\tIgnore file database, mark all files as new.\n" \
	"\t-r  --ignore-rb\t\t\tIgnore rollbacks.\n" \
	"\t-t  --store-timestamp\t\tStore file's timestamp.\n" \
	"\t-m  --store-permissions\t\tStore file's permissions.\n"

#define OPT "hvp:c:d:auirtm"
#define OPT_HELP "[-hvauirtm] [-p PORT] [-c PATH TO CONFIG FILE] [-d PATH TO FILE DATABASE]"
#define LOG_FILE "fsync_server.log"
#define CONF_FILE "server.conf"


#endif // OPTS_H
