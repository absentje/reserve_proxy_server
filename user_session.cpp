#include "user_session.h"
#include <boost/bind.hpp>
#include <iostream>

using std::cerr;
using std::endl;
using namespace boost::asio;
using boost::bind;
using ErrorCode = boost::system::error_code;
using std::size_t;

using boost::asio::ip::tcp;

UserSession::UserSession(io_service& io_service, boost::shared_ptr<std::ofstream> file)
	:_client_socket(io_service),
	_proxy_socket(io_service),
	_file_log(file)
{}

const tcp::socket& UserSession::get_client_socket() const noexcept {
	return _client_socket;
}
tcp::socket& UserSession::get_client_socket() noexcept {
	return _client_socket;
}


void UserSession::start(const std::string& proxy_ip, short proxy_port) noexcept {
	_proxy_socket.async_connect(
		tcp::endpoint(ip::address::from_string(proxy_ip), proxy_port),
		bind(&UserSession::handle_connect,
			shared_from_this(),
			placeholders::error));
}

void UserSession::handle_connect(const ErrorCode& error) noexcept {
	if (!error) {
		_proxy_socket.async_read_some(buffer(_proxy_buffer, default_buffer_size),
			bind(&UserSession::handle_proxy_read,
				shared_from_this(),
				placeholders::error,
				placeholders::bytes_transferred));
		_client_socket.async_read_some(
			buffer(_client_buffer, default_buffer_size),
			bind(&UserSession::handle_client_read,
				shared_from_this(),
				placeholders::error,
				placeholders::bytes_transferred));
	}
	else {
		cerr << error.message() << endl;
		close();
	}
}

void UserSession::handle_client_read(const ErrorCode& error, const size_t bytes_transferred) noexcept {
	if (!error) {
		for (int i = 0; i < bytes_transferred; i++) {
			*_file_log << _client_buffer[i];
		}
		*_file_log << '\n';
		async_write(_proxy_socket,
			buffer(_client_buffer, bytes_transferred),
			transfer_all(),
			bind(&UserSession::handle_proxy_write,
				shared_from_this(),
				placeholders::error));
	}
	else {
		cerr << error.message() << endl;
		close();
	}
}

void UserSession::handle_proxy_write(const ErrorCode& error) noexcept {
	if (!error) {
		_client_socket.async_read_some(buffer(_client_buffer, default_buffer_size),
			bind(&UserSession::handle_client_read,
				shared_from_this(),
				placeholders::error,
				placeholders::bytes_transferred));
	}
	else {
		cerr << error.message() << endl;
		close();
	}
}

void UserSession::handle_proxy_read(const ErrorCode& error, const std::size_t bytes_transferred) noexcept {
	if (!error) {
		async_write(_client_socket,
			buffer(_proxy_buffer, bytes_transferred),
			transfer_all(),
			bind(&UserSession::handle_client_write,
				shared_from_this(),
				placeholders::error));
	}
	else {
		cerr << error.message() << endl;
		close();
	}
}

void UserSession::handle_client_write(const ErrorCode& error) noexcept {
	if (!error) {
		_proxy_socket.async_read_some(buffer(_proxy_buffer, default_buffer_size),
			bind(&UserSession::handle_proxy_read,
				shared_from_this(),
				placeholders::error,
				placeholders::bytes_transferred));
	}
	else {
		cerr << error.message() << endl;
		close();
	}
}

void UserSession::close() noexcept {
	if (_client_socket.is_open()) {
		_client_socket.close();
	}
	if (_proxy_socket.is_open()) {
		_proxy_socket.close();
	}
}
