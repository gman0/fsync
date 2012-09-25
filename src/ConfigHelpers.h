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

#ifndef CONFIG_HELPERS_H
#define CONFIG_HELPERS_H

#include <cstdlib>
#include <boost/filesystem.hpp>

static boost::filesystem::path getFsyncHomePath()
{
#ifdef _WIN32
	return boost::filesystem::path(getenv("HOMEDRIVE")) / getenv("HOMEPATH") / "fsync";
#endif

	return boost::filesystem::path(getenv("HOME")) / ".fsync";
}

#endif // CONFIG_HELPERS_H
