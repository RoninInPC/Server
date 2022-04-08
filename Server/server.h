#pragma comment(lib, "ws2_32.lib")
#include <winsock2.h>
#include <iostream>
//#include<thread>
#include<mutex>
#include<list>
#include<Windows.h>
#include<fstream>
#include<string>
#include<map>
#include<functional>
#include<ctime>
#include<json/json.h>
#pragma warning(disable: 4996)
using namespace std;
class Server {
private:
	void handler_error(list<SOCKET>::iterator it);
	void send_int(int number, list<SOCKET>::iterator it);//отослать число
	void send_message(string& msg, list<SOCKET>::iterator it);//отослать сообщение
	int take_int(list<SOCKET>::iterator it);//получение числа
	string take_message(list<SOCKET>::iterator it);//получение сообщения
	void HandlerClient(list<SOCKET>::iterator it);
	string make_name_json(string name_file);
	string make_json(string name_file);
	void make_log(string command, list<SOCKET>::iterator it);
	SOCKET sListen;
	mutex mtx{};
	SOCKADDR_IN addr{};
	int sizeofaddr{};
	list<SOCKET> Conections_socket{};
	map<string, string> request_execution{
		{"SendFile","GetFile"},
		{"GetFile","SendFile"}
	};
	map<int, string> file_type{
		{FILE_ATTRIBUTE_ARCHIVE,"ARCHIVE"},
		{FILE_ATTRIBUTE_COMPRESSED,"COMPRESSED"},
		{FILE_ATTRIBUTE_DIRECTORY,"DIRECTORY"},
		{FILE_ATTRIBUTE_ENCRYPTED,"ENCRYPTED"},
		{FILE_ATTRIBUTE_HIDDEN,"HIDDEN"},
		{FILE_ATTRIBUTE_NORMAL,"NORMAL"},
		{FILE_ATTRIBUTE_OFFLINE,"OFFLINE"},
		{FILE_ATTRIBUTE_READONLY,"READONLY"},
		{FILE_ATTRIBUTE_REPARSE_POINT,"REPARSE_POINT"},
		{FILE_ATTRIBUTE_SPARSE_FILE,"SPARSE_FILE"},
		{FILE_ATTRIBUTE_SYSTEM,"SYSTEM"},
		{FILE_ATTRIBUTE_TEMPORARY,"TEMPORARY"}
	};
	map<int, string> file_size{
		{0,"bit"},
		{1,"byte"},
		{2,"kb"},
		{3,"mb"},
		{4,"gb"},
		{5,"tb"}
	};
	map < string, function<void(list<SOCKET>::iterator&)>> fun_body{
		{"SendFile",[&](list<SOCKET>::iterator it) {send_file(it); }},
		{"GetFile",[&](list<SOCKET>::iterator it) {get_file(it); }}
	};
	const int save_message_size = 1024;
	const string path_home_directory = "D:\\Server\\Files\\";
	const string path_json_files = "D:\\Server\\Jsons\\";
	const string name_log_file = "D:\\Server\\Logs.txt";

public:
	Server() = default;
	Server(string adr, u_short port, int family = AF_INET);
	void send_file(list<SOCKET>::iterator it);
	void get_file(list<SOCKET>::iterator it);
	void working_server();
};