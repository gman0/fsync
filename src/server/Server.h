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

#ifndef SERVER_H
#define SERVER_H

#include "AppInterface.h"
#include "NetworkManager.h"
#include "FileGatherer.h"
#include "Packet.h"

class ProcessFile_load;
class Rollback;

class Server : public AppInterface
{
	private:
		PathTransform * m_pathTransform;
		FileGatherer * m_fileGatherer;
		NetworkManager * m_networkManager;
		Rollback * m_rollbackSolver;

		FileGatherer::FIProxyPtrVector m_proxyVector;
		FileGatherer::FIProxyPtrVector m_rollbackProxyVector;
		bool m_willRollback;

	public:
		Server(int argc, char ** argv);
		~Server();

	private:
		Server(const Server & s);
		const Server & operator=(const Server & s);

		void getClient();
		void transferFilesLoop(const FileGatherer::FIProxyPtrVector & proxies);
		void transferFiles();

		void sendObjectCount(size_t size);
		PacketHeader prepareFileTransfer(const FileGatherer::FileInfoProxy * proxy);

		PacketHeader packFileInfo(const FileGatherer::FileInfo * fi, short int flags);

		/*
		 * If we encounter an error from the client side we "roll back" the changes gathered from
		 * file system and save them into database using FileGatherer's updateDb so we can redo
		 * the action when the user runs server again.
		 */
		void addRollback(FileGatherer::FileInfoProxy * proxy);
		void processRollbacks();

		void handleNew(bool hasFreeSpace, FileGatherer::FileInfoProxy * proxy);
		void handleChange(bool hasFreeSpace, FileGatherer::FileInfoProxy * proxy);

		void recursiveChunkSearch(ProcessFile_load * file, offset_t endRange);
};

#endif // SERVER_H
