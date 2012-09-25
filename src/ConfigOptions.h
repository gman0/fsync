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

#ifndef CONFIG_OPTIONS_H
#define CONFIG_OPTIONS_H

#include <string>
#include <map>
#include <getopt.h>

static const option g_options[] = {
	{"help",		no_argument,		0,	'h'},
	{"version",		no_argument,		0,	'v'},

	{"host",		required_argument,	0,	's'},
	{"port",		required_argument,	0,	'p'},

	{"config-file",	required_argument,	0,	'c'},
	{"db-file",		required_argument,	0,	'd'},

	{"dont-save-db",no_argument,		0,	'a'},
	{"update-db",	no_argument,		0,	'u'},
	{"ignore-db",	no_argument,		0,	'i'},
	{"ignore-rb",	no_argument,		0,	'r'},

	{"store-timestamp",no_argument,		0,	't'},
	{"store-permissions",no_argument,	0,	'm'},
	{0,				0,					0,	  0}
};


class ConfigOptions
{
	private:
		std::map<std::string, std::string> m_optionsMap;

	public:
		ConfigOptions(int argc, char ** argv, const char * optStr, const char * package, const char * version,
						const char * description, const char * help, const char * helpOptions);

	protected:
		void printHelp(const char * description, const char * progName, const char * helpOptions,
						const char * help);
		void printVersion(const char * package, const char * version);

		std::string & getHost();
		unsigned int getPort();
		std::string & getConfigFile();
		std::string & getDbFile();
		bool dontSaveDb();
		bool updateDb();
		bool ignoreDb();
		bool ignoreRb();
		bool storeTimestamp();
		bool storePermissions();

	private:
		void setHost(const char * host);
		void setPort(const char * port);
		void setConfigFile(const char * path);
		void setDbFile(const char * path);
		void setDontSaveDb();
		void setUpdateDb();
		void setIgnoreDb();
		void setIgnoreRb();
		void setStoreTimestamp();
		void setStorePermissions();

		bool checkEntry(const std::string & key);
};

#endif // CONFIG_OPTIONS_H
