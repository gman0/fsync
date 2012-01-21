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

#ifndef PATH_TRANSFORM_H
#define PATH_TRANSFROM_H

#include <list>
#include <vector>
#include <string>
#include <utility>
#include <boost/filesystem.hpp>

typedef short int PathId;
typedef std::list<std::pair<PathId, boost::filesystem::path> > ID_Path_pairList;
typedef std::list<boost::filesystem::path> PathList;

class PathTransform
{
	private:
		const ID_Path_pairList & m_id_path_pairList;

	public:
		PathTransform(const ID_Path_pairList & id_paths) : m_id_path_pairList(id_paths)
		{}

		boost::filesystem::path getPath(PathId pathId);
		PathId getPathId(const boost::filesystem::path & path);

		/*
		 * This is only a helper method and checks only wether the length (string) of the path
		 * is bigger than 0. Also bear in mind that this method also logs an error
		 * message if the string is empty, therefore LogManager needs to be initialized
		 * before using this method.
		 */
		bool checkPathAndLog(const boost::filesystem::path & path, PathId pathId);

		/*
		 * Removes path with pathId from path.
		 *
		 * outPath should point to the beginning of a char array with at least
		 * PATH_LENGTH (defined in defs.h) items.
		 *
		 * Also note that this method does not make any checks wether path has a parent
		 * path with pathId, so make sure you're setting the right pathId.
		 */
		void cutPath(PathId pathId, const char * path, char * outPath);

		/*
		 * Prepends path with pathId to p
		 */
		boost::filesystem::path glueCutPath(PathId pathId, const char * p);

		PathList getIncludingPaths();
		PathList getExcludingPaths();

		bool isExcluded(const boost::filesystem::path & currentPath, PathId currentPathId);

		boost::filesystem::path operator[](PathId id);
	
	private:
		int getPathLevelCount(const boost::filesystem::path & p) const;
		std::vector<std::string> makeParts(const boost::filesystem::path & p) const;
};

#endif // PATH_TRANSFORM_H
