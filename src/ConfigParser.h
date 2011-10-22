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

#ifndef CONFIG_PARSER_H
#define CONFIG_PARSER_H

#include <string>
#include <list>
#include <utility>
#include <map>
#include <boost/filesystem.hpp>


typedef std::map<std::string, std::list<std::string> >  CFGMap;
typedef unsigned short int PathId;
typedef std::list<std::pair<PathId, boost::filesystem::path> > ID_Path_pairList;

/*
 * configuration files will be plain and simple:
 * KEY VALUE
 *
 * you can also make comments by typing # at the beginning of a line:
 * # foo
 *
 * for synchronization paths it's as following example:
 * path ID:PATH		- where ID is * or a number bigger than 0 and PATH is an absolute path to a directory
 * 					- also notice that the ID has to be unique (except for the case you enter * as the ID)
 *
 * any duplicate lines will be removed
 * only the last definition will take place:
 * foo 1
 * foo 2
 * =====
 * foo	->	2
 *
 * except for the synchronization paths, which will aggregate:
 * path 1:/home/bar/folder1
 * path 2:/home/bar/folder2
 * ========================
 * path	->	1:/home/bar/folder1
 * 		->	2:/home/bar/folder2
 */

class ConfigParser
{
	private:
		const char * m_configFileName;
		CFGMap  m_pairs;

	public:
		ConfigParser(const char * configFile);

	protected:
		ID_Path_pairList getPathList();
		unsigned int getPort();
		bool recursiveFileSearchEnabled();
		std::string getHost();
		boost::filesystem::path getFilesDbPath();

	private:
		void generatePairs(const std::string & configBuffer);
		void reportError(const std::string & where, const std::string & what, bool appendSkipping = true) const;
		bool isPathIdUnique(const ID_Path_pairList & list, PathId id) const;
		bool string2bool(const std::string & str) const;

		template<class Return> Return checkKey(std::list<std::string> & l, const char * arg = 0);
};

#endif // CONFIG_PARSER_H
