#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <string>
#include <cstddef>
#include <fstream>
#include <cstring>
#include <cstdlib>
#include <cstdio>

// BOOST includes

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/array.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

using namespace std;
using boost::asio::ip::tcp;

//////////////////////////////////////////////////////////////////////
//					USER_SESSION
/////////////////////////////////////////////////////////////////////

// для проверки работы сервера введены пометки с выводом на экран (label)

class UserSession : public boost::enable_shared_from_this<UserSession> {
public:
	UserSession(boost::asio::io_service& io_service, boost::shared_ptr<ofstream> file)
		:_client_socket(io_service),
		_proxy_socket(io_service),
		_file_log(file)
	{}

	tcp::socket& get_client_socket() {
		return _client_socket;
	}

	void start(const std::string& proxy_ip, std::size_t proxy_port) {
		_proxy_socket.async_connect(
			tcp::endpoint(boost::asio::ip::address::from_string(proxy_ip), proxy_port),
			boost::bind(&UserSession::handle_connect,
				shared_from_this(),
				boost::asio::placeholders::error));
	}

	void handle_connect(const boost::system::error_code& error) {
		if (!error) {
			cout << "handle_connect port_client: " << _client_socket.remote_endpoint().port() << "\n\n"; // (label)
			_proxy_socket.async_read_some(boost::asio::buffer(_proxy_buffer, default_buffer_size),
				boost::bind(&UserSession::handle_proxy_read,
					shared_from_this(),
					boost::asio::placeholders::error,
					boost::asio::placeholders::bytes_transferred));
			_client_socket.async_read_some(
				boost::asio::buffer(_client_buffer, default_buffer_size),
				boost::bind(&UserSession::handle_client_read,
					shared_from_this(),
					boost::asio::placeholders::error,
					boost::asio::placeholders::bytes_transferred));
		}
		else {
			cout << "error_handle_connect\n\n"; // (label)
			cerr << error.message() << endl;
			close();
		}

	}

	void handle_client_read(const boost::system::error_code& error, const std::size_t bytes_transferred) {
		if (!error) {
			cout << "handle_client_read port_client: " << _client_socket.remote_endpoint().port() << "\n\n"; // (label)
			for (int i = 0; i < bytes_transferred; i++) {
				*_file_log << _client_buffer[i];
			}
			*_file_log << '\n';
			boost::asio::async_write(_proxy_socket,
				boost::asio::buffer(_client_buffer, bytes_transferred),
				boost::asio::transfer_all(),
				boost::bind(&UserSession::handle_proxy_write,
					shared_from_this(),
					boost::asio::placeholders::error));
		}
		else {
			cout << "error_handle_client_read\n\n"; // (label)
			cerr << error.message() << endl;
			close();
		}
	}

	void handle_proxy_write(const boost::system::error_code& error) {
		if (!error) {
			cout << "handle_proxy_write port_client:" << _client_socket.remote_endpoint().port() << "\n\n"; // (label)
			_client_socket.async_read_some(boost::asio::buffer(_client_buffer, default_buffer_size),
				boost::bind(&UserSession::handle_client_read,
					shared_from_this(),
					boost::asio::placeholders::error,
					boost::asio::placeholders::bytes_transferred));
		}
		else {
			cout << "error_handle_proxy_write\n\n"; // (label)
			cerr << error.message() << endl;
			close();
		}
	}

	void handle_proxy_read(const boost::system::error_code& error, const std::size_t bytes_transferred) {
		if (!error) {
			cout << "handle_proxy_read port_client: " << _client_socket.remote_endpoint().port() << "\n\n"; // (label)
			boost::asio::async_write(_client_socket,
				boost::asio::buffer(_proxy_buffer, bytes_transferred),
				boost::asio::transfer_all(),
				boost::bind(&UserSession::handle_client_write,
					shared_from_this(),
					boost::asio::placeholders::error));
		}
		else {
			cout << "error_handle_proxy_read\n\n"; // (label)
			cerr << error.message() << endl;
			close();
		}
	}

	void handle_client_write(const boost::system::error_code& error) {
		if (!error) {
			cout << "handle_client_write port_client: " << _client_socket.remote_endpoint().port() << "\n\n"; // (label)
			_proxy_socket.async_read_some(boost::asio::buffer(_proxy_buffer, default_buffer_size),
				boost::bind(&UserSession::handle_proxy_read,
					shared_from_this(),
					boost::asio::placeholders::error,
					boost::asio::placeholders::bytes_transferred));
		}
		else {
			cout << "error_handle_client_write\n\n"; // (label)
			cerr << error.message() << endl;
			close();
		}
	}

	void close() {
		if (_client_socket.is_open()) {
			cout << "close_client_socket port_client: " << _client_socket.remote_endpoint().port() << "\n\n"; // (label)
			_client_socket.close();
		}
		if (_proxy_socket.is_open()) {
			cout << "close_proxy_socket port_client: " << _proxy_socket.remote_endpoint().port() << "\n\n"; // (label)
			_proxy_socket.close();
		}
	}

private:
	enum { default_buffer_size = 8192 };
	tcp::socket _client_socket;
	tcp::socket _proxy_socket;
	char _client_buffer[default_buffer_size];
	char _proxy_buffer[default_buffer_size];
	boost::shared_ptr<ofstream> _file_log;
};
/////////////////////////////////////////////////////////////////////
//						SERVER
/////////////////////////////////////////////////////////////////////
class Server {
public:
	Server(boost::asio::io_service& io_service,	short local_port, const std::string remote_ip, short remote_port) :
		_io_service(io_service), acceptor_(io_service, tcp::endpoint(tcp::v4(), local_port)),
		_remote_ip(remote_ip), _remote_port(remote_port)
	{
		const char NAME_FILE[] = "file_log.txt";
		file_log = boost::shared_ptr<ofstream>(new ofstream(NAME_FILE));
		start_accept();
	}

	boost::shared_ptr<UserSession> create_session(){
		return boost::shared_ptr<UserSession>(new UserSession(_io_service, file_log));
	}

	bool start_accept() {
		try {
			session = create_session();
			acceptor_.async_accept(session->get_client_socket(),
				boost::bind(&Server::handle_accept, this,
					boost::asio::placeholders::error));
		}
		catch (const std::exception& e) {
			cerr << "acceptor exception: " << e.what() << std::endl;
			return false;
		}
		return true;
	}

	void handle_accept(const boost::system::error_code& error) {
		if (!error) {
			cout << "handle_accept port_client: " << session->get_client_socket().remote_endpoint().port() << "\n\n"; // (label)
			session->start(_remote_ip, _remote_port);
			if (!start_accept()) {
				std::cerr << "Failed to accept.\n\n";
			}
		}
		else {
			cerr << error.message() << endl;
		}
	}

private:
	boost::asio::io_service& _io_service;
	tcp::acceptor acceptor_;
	boost::shared_ptr<UserSession> session;
	std::string _remote_ip;
	short _remote_port;
	boost::shared_ptr<ofstream> file_log;
};

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