#include <iostream>
#include "Client.h"
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

	return 0;
}
