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

#include <cstring>
#include <iostream>
#include <boost/lexical_cast.hpp>
#include "PathTransform.h"
#include "LogManager.h"
#include "defs.h"

using namespace std;
using namespace boost::filesystem;

path PathTransform::getPath(PathId id)
{
	for (ID_Path_pairList::const_iterator i = m_id_path_pairList.begin(); i != m_id_path_pairList.end(); i++)
	{
		if (i->first == id)
		{
			if (id >= 0)
				return i->second;
			else
			{
				path parentPath = getPath(id * -1);
				parentPath /= i->second;

				return parentPath;
			}
		}
	}

	return path();
}

PathId PathTransform::getPathId(const path & p)
{
	for (ID_Path_pairList::const_iterator i = m_id_path_pairList.begin(); i != m_id_path_pairList.end(); i++)
	{
		if (i->second == p)
			return i->first;
	}

	return 0;
}

bool PathTransform::checkPathAndLog(const path & p, PathId pathId)
{
	if (strlen(p.c_str()) == 0)
	{
		string errMsg = "Path with id ";
		errMsg += boost::lexical_cast<string>(pathId);
		errMsg += " is not defined, skipping";
		LogManager::getInstancePtr()->log(errMsg, LogManager::L_WARNING);

		return false;
	}

	return true;
}

void PathTransform::cutPath(PathId pathId, const char * pth, char * outPath)
{
	path p = getPath(pathId);
	strcpy(outPath, (pth + strlen(p.c_str()) + 1)); // +1 because of the directory separator
}

path PathTransform::glueCutPath(PathId pathId, const char * p)
{
	return getPath(pathId) / p;
}

PathList PathTransform::getIncludingPaths()
{
	PathList pl;

	for (ID_Path_pairList::const_iterator i = m_id_path_pairList.begin(); i != m_id_path_pairList.end(); i++)
	{
		if (i->first >= 0)
			pl.push_back(i->second);
	}

	return pl;
}

PathList PathTransform::getExcludingPaths()
{
	PathList pl;

	for (ID_Path_pairList::const_iterator i = m_id_path_pairList.begin(); i != m_id_path_pairList.end(); i++)
	{
		PathId id = i->first;

		if (id < 0)
		{
			path parentPath = getPath(id * -1);
			parentPath /= i->second;

			pl.push_back(parentPath);
		}
	}

	return pl;
}

bool PathTransform::isExcluded(const path & currentPath, PathId pathId)
{
	vector<path> absoluteExcldPaths;
	path parentPath = getPath(pathId);
	PathId excldPathId = pathId * -1;

	for (ID_Path_pairList::const_iterator i = m_id_path_pairList.begin(); i != m_id_path_pairList.end(); i++)
	{
		if (i->first == excldPathId)
			absoluteExcldPaths.push_back(parentPath / i->second);
	}

	if (absoluteExcldPaths.empty())
		return false;

	vector<string> currentPathParts = makeParts(currentPath);
	int currentPartsSize = currentPathParts.size();

	for (vector<path>::const_iterator i = absoluteExcldPaths.begin(); i != absoluteExcldPaths.end(); i++)
	{
		vector<string> absoluteExcldParts = makeParts(*i);
		int excldPartsSize = absoluteExcldParts.size();

		if (currentPartsSize < excldPartsSize)
			continue;
		
		bool isExcluded = true;

		for (int j = 0; j < excldPartsSize; j++)
		{
			if (currentPathParts[j] != absoluteExcldParts[j])
			{
				isExcluded = false;
				break;
			}
		}

		if (isExcluded)
			return true;
	}
	
	return false;
}

vector<string> PathTransform::makeParts(const path & p) const
{
	vector<string> parts;

	for (path::const_iterator i = p.begin(); i != p.end(); i++)
		parts.push_back(i->string());
	
	return parts;
}

path PathTransform::operator[](PathId id)
{
	return getPath(id);
}
