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

#include <ctime>
#include <fstream>
#include <boost/filesystem.hpp>
#include "LogManager.h"
#include "FSException.h"

using namespace std;
using namespace boost::filesystem;

LogManager * LogManager::m_instance = NULL;

void LogManager::instance(const char * path)
{
	m_instance = new LogManager;

	if (exists(path))
	{
		if (is_directory(path))
			throw FSException("Path to log file is a directory, file expected", __FILE__, __LINE__);

		m_instance->m_logPath = path;
	}
	else
		m_instance->m_logPath = "fsync_server.log";
}

void LogManager::destroy()
{
	delete m_instance;
}

LogManager & LogManager::getInstance()
{
	return (*m_instance);
}

LogManager * LogManager::getInstancePtr()
{
	return (m_instance) ? m_instance : NULL;
}

void LogManager::log(const char * msg, int level, bool stdOutput)
{
	time_t rawTime;
	time(&rawTime);
	string stime = ctime(&rawTime);
	stime = stime.substr(0, stime.length() - 1); // omg! Why the \n ???

	ofstream logFile(m_logPath, ofstream::app);

	logFile << "[" << stime << "] ";

	switch (level)
	{
		case L_DEBUG:
			logFile << "(debug) ";
			break;
		case L_NOTICE:
			logFile << "(notice) ";
			break;
		case L_WARNING:
			logFile << "(warning) ";
			break;
		case L_ERROR:
			logFile << "(error) ";
			break;
	}

	logFile << msg << endl;

	logFile.close();

	if (stdOutput)
		cerr << msg << endl;
}

void LogManager::log(const string & msg, int level, bool stdOutput)
{
	log(msg.c_str(), level, stdOutput);
}
