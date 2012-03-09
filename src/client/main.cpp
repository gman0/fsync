#include <iostream>
#include "Client.h"
#include "Exit.h"
#include "FSException.h"

using namespace std;

int main(int argc, char * argv[])
{
	try
	{
		Client app(argc, argv);
	}
	catch (FSException & e)
	{
		cerr << "An exception has occured: " << e.what() << endl;
	}
	catch (Exit&)
	{
		LogManager::getInstancePtr()->destroy();
	}

	return 0;
}
