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

#include <iostream>
#include "FSException.h"
#include "Exit.h"
#include "Server.h"

using namespace std;

int main(int argc, char * argv[])
{
	try
	{
		Server app(argc, argv);
	}
	catch (FSException & e)
	{
		cerr << "An exception has occured: " << e.what() << endl;
		return 1;
	}
	catch (Exit&)
	{
		LogManager::getInstancePtr()->destroy();
	}

	return 0;
}
