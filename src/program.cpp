#include "HttpServer.h"

#include <unistd.h>

#include <cstdio>
#include <cstdlib>

#include <limits>

int main(int argc, char* const argv[])
{
	uint16_t port = 80;

	int opt;
	while ((opt = getopt(argc, argv, "p:")) != -1)
	{
		switch (opt)
		{
		case 'p':
			int p = atoi(optarg);
			if (p < 0 || p > std::numeric_limits<uint16_t>::max())
			{
				printf("Invalid port number\n");
				return 1;
			}
			port = p;
			break;
		}
	}

	try
	{
		HttpServer server(port);
	}
	catch (std::exception& ex)
	{
		fprintf(stderr, "%s\n", ex.what());
		return EXIT_FAILURE;
	}

	printf("Hello World!\n");

	return 0;
}
