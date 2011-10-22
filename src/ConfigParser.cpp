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

#include <fstream>
#include <iostream>
#include <cstring>
#include <cstdlib>
#include "ConfigParser.h"
#include "FSException.h"
#include "LogManager.h"

using namespace std;
using namespace boost::filesystem;

ConfigParser::ConfigParser(const char * configFile) : m_configFileName(configFile)
{
	ifstream fin(configFile);
	string configBuffer;
	string buff;

	if (fin.is_open())
	{
		while (fin.good())
		{
			getline(fin, buff);

			// if this isn't just an empty line or a comment
			if (buff[0] != '\0' && buff[0] != '#')
				configBuffer += buff + '\n';
		}
	}
	else
	{
		string errMsg = "Cannot open configuration file \"";
		errMsg += configFile;
		errMsg += '\"';
		LogManager::getInstancePtr()->log(errMsg, LogManager::L_ERROR);
		throw FSException(errMsg, __FILE__, __LINE__);
	}

	fin.close();
	

	generatePairs(configBuffer);
}

void ConfigParser::generatePairs(const string & configBuffer)
{
	list<string> lines, tmplist; // tmplist is a placeholder as the further container for the vaules
	string tmp;

	// separate lines by \n and fill the lines list with them
	for (string::const_iterator i = configBuffer.begin(); i != configBuffer.end(); i++)
	{
		if (*i != '\n')
			tmp += *i;
		else
		{
			lines.push_back(tmp);
			tmp.clear();
		}
	}

	lines.unique();
	
	string key;
	list<string> values;

	// create KEY (space) VALUE pairs
	for (list<string>::const_iterator i = lines.begin(); i != lines.end(); i++)
	{
		int delimiter = i->find(' ');

		// get key
		key = i->substr(0, delimiter);

		// find its values
		for (list<string>::const_iterator j = lines.begin(); j != lines.end(); j++)
		{
			int d = j->find(' ');

			if (key == j->substr(0, d))
				values.push_back(j->substr(d + 1, j->length() - d - 1));
		}

		// assign the values to the key
		m_pairs[key] = values;

		key.clear();
		values.clear();
	}
}

void ConfigParser::reportError(const string & where, const string & what, bool appendSkipping) const
{
	string errMsg("ConfigParser error(");
	errMsg += m_configFileName;
	errMsg += "): there was an error during processing this part of line: \"" + where + "\" - " + what;

	if (appendSkipping)
		errMsg += ", skipping";

	LogManager::getInstancePtr()->log(errMsg, LogManager::L_WARNING);
}

unsigned int ConfigParser::getPort()
{
	list<string> l = m_pairs["port"];

	if (l.empty())
		return 0;
	
	return atoi(l.back().c_str());
}

bool ConfigParser::isPathIdUnique(const ID_Path_pairList & list, PathId id) const
{
	for (ID_Path_pairList::const_iterator i = list.begin(); i != list.end(); i++)
	{
		if (i->first == id)
			return false;
	}

	return true;
}

ID_Path_pairList ConfigParser::getPathList()
{
	list<string> values = m_pairs["path"];
	ID_Path_pairList paths;

	unsigned int delimiter = 0;
	PathId id = 0;
	string tmp;
	
	for (list<string>::const_iterator i = values.begin(); i != values.end(); i++)
	{
		delimiter = i->find(':');
		
		unsigned int len = i->length();

		if (delimiter > len)
		{
			reportError(*i, "delimiter ':' not found");
			continue;
		}
		else if (delimiter == len - 1)
		{
			reportError(*i, "no path defined");
			continue;
		}

		tmp = i->substr(0, delimiter);

		// user can set the path ID to an asterisk (*) or a number bigger than 0 (0 is excluded)
		if (tmp.compare("*") == 0)
			id = 0;
		else
		{
			id = atoi(tmp.c_str());

			// if user entered something other than * or he actually entered 0
			if (id == 0)
			{
				reportError(*i, "path ID has to be a number bigger than 0 or an asterisk (*)");
				continue;
			}

			if (!isPathIdUnique(paths, id))
			{
				reportError(*i, "path ID has to be unique");
				continue;
			}
		}

		paths.push_back(pair<PathId, path>(id, i->substr(delimiter + 1, i->length() - delimiter - 1)));
	}

	return paths;
}

bool ConfigParser::string2bool(const string & str) const
{
	if (str.length() == 0)
		return false; // defaults to false

	const int count = 3;

	string _true[count] = {"true", "yes", "1"};
	string _false[count] = {"false", "no", "0"};

	for (int i = 0; i < count; i++)
	{
		if (str.compare(_true[i]) == 0)
			return true;
	}

	for (int i = 0; i < count; i++)
	{
		if (str.compare(_false[i]) == 0)
			return false;
	}

	reportError(str, "boolean value expected (true/yes/1 or false/no/0), defaulting to false", false);

	return false;
}

template<class Return>
Return ConfigParser::checkKey(list<string> & l, const char * arg)
{
	if (l.empty())
	{
		if (arg)
			return Return(arg);
		else
			return Return();
	}
	else
		return l.back();
}

bool ConfigParser::recursiveFileSearchEnabled()
{
	list<string> l = m_pairs["recursiveFileSearch"];

	if (l.empty())
		return false;

	return string2bool(l.back());
}

string ConfigParser::getHost()
{
	list<string> l = m_pairs["host"];
	return checkKey<string>(l, "");
}

path ConfigParser::getFilesDbPath()
{
	list<string> l = m_pairs["filesDbPath"];
	return checkKey<path>(l, "server.db");
}
