#include "user_session.h"

// для проверки работы сервера введены пометки с выводом на экран (label)

UserSession::UserSession(boost::asio::io_service& io_service, boost::shared_ptr<ofstream> file)
	:_client_socket(io_service),
	_proxy_socket(io_service),
	_file_log(file)
{}

tcp::socket& UserSession::get_client_socket() {
	return _client_socket;
}

void UserSession::start(const std::string& proxy_ip, std::size_t proxy_port) {
	_proxy_socket.async_connect(
		tcp::endpoint(boost::asio::ip::address::from_string(proxy_ip), proxy_port),
		boost::bind(&UserSession::handle_connect,
			shared_from_this(),
			boost::asio::placeholders::error));
}

void UserSession::handle_connect(const boost::system::error_code& error) {
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

void UserSession::handle_client_read(const boost::system::error_code& error, const std::size_t bytes_transferred) {
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

void UserSession::handle_proxy_write(const boost::system::error_code& error) {
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

void UserSession::handle_proxy_read(const boost::system::error_code& error, const std::size_t bytes_transferred) {
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

void UserSession::handle_client_write(const boost::system::error_code& error) {
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

void UserSession::close() {
	if (_client_socket.is_open()) {
		cout << "close_client_socket port_client: " << _client_socket.remote_endpoint().port() << "\n\n"; // (label)
		_client_socket.close();
	}
	if (_proxy_socket.is_open()) {
		cout << "close_proxy_socket port_client: " << _proxy_socket.remote_endpoint().port() << "\n\n"; // (label)
		_proxy_socket.close();
	}
}
