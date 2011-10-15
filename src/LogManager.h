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

#ifndef LOG_MANAGER_H
#define LOG_MANAGER_H

#include <string>

class LogManager
{
	private:
		static LogManager * m_instance;
		const char * m_logPath;
	
	public:
		enum
		{
			L_DEBUG,
			L_NOTICE,
			L_WARNING,
			L_ERROR
		};


	private:
		LogManager() {}
		LogManager & operator=(const LogManager & lm);
	
	public:
		static void instance(const char * path);
		void destroy();

		static LogManager & getInstance();
		static LogManager * getInstancePtr();
		
		void log(const char * msg, int level = L_NOTICE, bool stdOutput = true);
		void log(const std::string & msg, int level = L_NOTICE, bool stdOutput = true);
};

#endif // LOG_MANAGER_H

