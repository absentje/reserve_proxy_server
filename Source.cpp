#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <string>
#include <cstring>
#include <cstdlib>
#include <cstdio>

// BOOST includes
#include <boost/asio.hpp>
// include project-files
#include "reserve_proxy_server.h"

using namespace std;
using boost::asio::ip::tcp;



int main(int argc, char** argv) {
	setlocale(LC_CTYPE, "Russian");
	try
	{
		cout << "Введите порт открываемого прокси сервера :\n";
		string port;
		getline(cin, port);
		cout << "Введите ip адрес сервера с БД :\n";
		string ip_proxy;
		getline(cin, ip_proxy);
		cout << "Введите порт сервера с БД :\n";
		string port_proxy;
		getline(cin, port_proxy);
		boost::asio::io_service io_service;
		Server server(io_service, (short)atoi(port.c_str()), ip_proxy, (short)atoi(port_proxy.c_str()));
		server.start_accept();
		io_service.run();
	}
	catch (std::exception& e)
	{
		std::cerr << "Exception: " << e.what() << "\n";
		system("pause");
	}
	catch (...)
	{
		system("pause");
	}
	return 0;
}
