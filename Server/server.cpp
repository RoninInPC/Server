#include"server.h"
void Server::handler_error(list<SOCKET>::iterator it)
{
	closesocket(*it);
}

void Server::send_int(int number, list<SOCKET>::iterator it)
{
	auto num = send(*it, (char*)&number, sizeof(int), NULL);
	if (num == -1 || num == 0) {
		handler_error(it);
	}
}

void Server::send_message(string& msg, list<SOCKET>::iterator it)
{
	send_int(msg.size(), it);
	auto num = send(*it, msg.c_str(), msg.size(), NULL);
	if (num == -1 || num == 0) handler_error(it);
}

int Server::take_int(list<SOCKET>::iterator it)
{
	int number;
	int num = recv(*it, (char*)&number, sizeof(int), NULL);
	if (num == -1 || num == 0) handler_error(it);
	return number;
}

string Server::take_message(list<SOCKET>::iterator it)
{
	int size_str = take_int(it);
	char* msg = new char[size_str + 1];
	auto num = recv(*it, msg, size_str, NULL);
	if (num == -1 || num == 0) handler_error(it);
	msg[size_str] = '\0';
	return string(msg);
}

string Server::make_json(string name_file)
{
	auto new_name_file = make_name_json(name_file);


	auto str = path_home_directory + name_file;
	wstring path(str.begin(), str.end());
	WIN32_FILE_ATTRIBUTE_DATA file_info;
	GetFileAttributesEx(path.c_str(),(GET_FILEEX_INFO_LEVELS)0,&file_info);
	auto type = file_type[file_info.dwFileAttributes];
	int size = file_info.nFileSizeLow;
	int degree_size = 1;
	while (size >1024) {
		degree_size += size / 1024 != 0 ? 1 : 0;
		size /= 1024;
	}
	auto type_size = file_size[degree_size];


	ofstream F(path_json_files + new_name_file);
	Json::StyledStreamWriter writer;
	Json::Value value_write;
	value_write["filename"] = name_file;
	value_write["typefile"] = type;
	value_write["size"] = to_string(size);
	value_write["typesize"] = file_size[degree_size];
	writer.write(F, value_write);
	F.close();

	return new_name_file;
}

void Server::make_log(string command, list<SOCKET>::iterator it)
{
	ofstream F(name_log_file, ios::app);
	time_t now = time(0);
	string dt = ctime(&now);
	string c_or_r = "command";
	if (request_execution.find(command) != request_execution.end() || command=="Exit") {
		c_or_r = "request";
	}
	F << "user in SOCKET " << *it << " get "<<c_or_r<<" "<< command << " in " << dt;
	F.close();
}

Server::Server(string adr, u_short port, int family)
{
	WSAData wsaData;
	WORD DLLVersion = MAKEWORD(2, 1);
	if (WSAStartup(DLLVersion, &wsaData) != 0) {
		std::cout << "Error" << std::endl;
		exit(1);
	}

	sizeofaddr = sizeof(addr);
	addr.sin_addr.s_addr = inet_addr(adr.c_str());
	addr.sin_port = htons(port);
	addr.sin_family = family;

	sListen = socket(family, SOCK_STREAM, NULL);
	::bind(sListen, (SOCKADDR*)&addr, sizeof(addr));
	::listen(sListen, SOMAXCONN);
}

void Server::send_file(list<SOCKET>::iterator it)
{
	string err("ERROR|NOFILE!");
	auto name_file = take_message(it);
	if (!ifstream(path_home_directory+name_file)) {
		send_message(err,it);
		return;
	}
	auto name_json = make_json(name_file);
	send_message(name_json, it);
	ifstream F(path_json_files + name_json);
	char* str = new char[save_message_size * 4];
	while (!F.eof()) {
		F.getline(str, save_message_size * sizeof(char));
		string send(str);
		if (send.size() != 0) {
			send_int(1, it);
			send_message(send, it);
		}
	}
	send_int(0,it);
	F.close();
}

void Server::get_file(list<SOCKET>::iterator it)
{
	auto name_file = take_message(it);
	if(name_file=="ERROR | NOFILE!"){
		return;
	}
	ofstream F(path_home_directory+name_file);
	while (take_int(it) != 0) {
		F << take_message(it);
	}
	F.close();
}

void Server::working_server()
{
	auto count = 0;
	SOCKET newConnection;
	while (1)
	{
		newConnection = accept(sListen, (SOCKADDR*)&addr, &sizeofaddr);

		if (newConnection == 0) {
			closesocket(newConnection);
			exit(1);
		}
		else
		{
			mtx.lock();
			Conections_socket.emplace_front(newConnection);
			auto it = Conections_socket.begin();
			mtx.unlock();
			thread t([this](list<SOCKET>::iterator it) {HandlerClient(it); }, it);
			t.detach();

		}
	}
};

void Server::HandlerClient(list<SOCKET>::iterator it)
{
	while (true) {
		auto command = take_message(it);
		make_log(command,it);//делает запись в файл любой команды, исполняемой в клиенте, например READ
		if (command == "Exit") {
			closesocket(*it);
			break;
		}
		if (request_execution.find(command) != request_execution.end()) {
			fun_body[request_execution[command]](it);
		}
	}
}

string Server::make_name_json(string name_file)
{
	string new_name;
	for (int i = 0; i < name_file.size(); i++) {
		if (name_file[i] == '.') {
			break;
		}
		new_name += name_file[i];
	}
	new_name += ".json";
	return new_name;
}
