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

#ifndef CLIENT_H
#define CLIENT_H

#include <boost/filesystem.hpp>
#include <string>
#include "AppInterface.h"
#include "NetworkManager.h"
#include "PathTransform.h"
#include "Packet.h"
#include "defs.h"

class ProcessFile_store;

class Client : public AppInterface
{
	private:
		NetworkManager * m_networkManager;
		PathTransform * m_pathTransform;

	public:
		Client(int argc, char ** argv);
		~Client();

	private:
		Client(const Client & c);
		const Client & operator=(const Client & c);

		void getServer(const std::string & host);
		void fileTransfer();

		/*
		 * We will create ph_result and it shall be sent to server.
		 * It also tells us what to do next:
		 * Wether there was an error and continue to another "object"
		 * or handle new/change actions. We don't need a handler for
		 * delete action because there's no need to inform the server
		 * that we deleted the file - server doesn't care about this
		 * and we will delete the file inside this method (well, not
		 * inside but using deleteFile method).
		 *
		 * ph_fi_out is the actual PacketHeader_FileInfo that we
		 * recieved from server (also note that the recieving is done
		 * inside this method) and we will use it later in the handling
		 * phase.
		 *
		 * This method returns bool to see if we can actually
		 * continue or just skip because of an error (for example we
		 * ran out of disk space and cannot recieve the file).
		 */
		bool prepareFileTransfer(PacketHeader * ph_result, PacketHeader_FileInfo * ph_fi_out);

		void handleNew(const PacketHeader_FileInfo * ph_fi);
		void handleChange(const PacketHeader_FileInfo * ph_fi);
		void deleteFile(boost::filesystem::path path);

		/*
		 * Searches for chunks requested by server and sends it's hash.
		 * If the hashes don't match, server sends the whole chunk and
		 * this method rewrites the existing data in the file with the
		 * recieved chunk.
		 *
		 * Returns true if we're finished, otherwise false.
		 */
		bool chunkSearchAndStore(ProcessFile_store * file);
};

#endif // CLIENT_H
