#include"server.h"
int main()
{
	Server base("127.0.0.1", 1111);
	base.working_server();
	return 0;
}

