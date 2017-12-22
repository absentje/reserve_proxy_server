#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <string>

// BOOST includes
#include <boost/asio.hpp>
// include project-files
#include "reserve_proxy_server.h"


using std::string;
using std::cout;
using std::cin;
using std::cerr;
using std::endl;
using boost::asio::ip::tcp;

int main(int argc, char** argv) {
	setlocale(LC_CTYPE, "Russian");
	try {
		short port, port_proxy;
		string ip_proxy;
		{
			// input parameters
			cout << "Введите порт открываемого прокси сервера :\n";
			cin >> port;
			cout << "Введите ip адрес сервера с БД :\n";
			cin >> ip_proxy;
			cout << "Введите порт сервера с БД :\n";
			cin >> port_proxy;
		}

		boost::asio::io_service io_service;
		Server server(io_service, port, ip_proxy, port_proxy);
		server.start_accept();
		io_service.run();
	}
	catch (std::exception& e)
	{
		cerr << "Exception: " << e.what() << "\n";
		system("pause");
	}
	catch (...)
	{
		system("pause");
	}
	return 0;
}
