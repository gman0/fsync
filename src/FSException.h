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

#ifndef FS_EXCEPTION
#define FS_EXCEPTION

#include <exception>
#include <iostream>
#include <string>
#include <sstream>

using namespace std;

class FSException : public exception
{
	private:
		string m_message;
		string m_file;
		string m_what;
		int m_line;

	public:
		// obsolete
		FSException(const char * message, const char * file, int line) : m_message(string(message)), m_file(string(file)),
																		m_line(line)
		{
			m_what = message;
		}

		// obsolete
		FSException(const string & message, const char * file, int line) : m_message(message),
																		m_file(string(file)), m_line(line)
		{
			m_what = message;
		}

		FSException(const string & message)
		{
			m_what = message;
		}

		FSException(const char * message)
		{
			m_what = message;
		}

		virtual const char * what() const throw()
		{
			return m_what.c_str(); 
		}

		virtual ~FSException() throw() {}
};

#endif // FS_EXCEPTION
