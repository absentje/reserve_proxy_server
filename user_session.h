#ifndef _USER_SESSION__
#define _USER_SESSION__

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




class UserSession : public boost::enable_shared_from_this<UserSession> {
public:
	UserSession(boost::asio::io_service& io_service, boost::shared_ptr<ofstream> file);

	tcp::socket& get_client_socket();

	void start(const std::string& proxy_ip, std::size_t proxy_port);

	void handle_connect(const boost::system::error_code& error);

	void handle_client_read(const boost::system::error_code& error, const std::size_t bytes_transferred);

	void handle_proxy_write(const boost::system::error_code& error);

	void handle_proxy_read(const boost::system::error_code& error, const std::size_t bytes_transferred);

	void handle_client_write(const boost::system::error_code& error);

	void close();

private:
	enum { default_buffer_size = 8192 };
	tcp::socket _client_socket;
	tcp::socket _proxy_socket;
	char _client_buffer[default_buffer_size];
	char _proxy_buffer[default_buffer_size];
	boost::shared_ptr<ofstream> _file_log;
};

#endif // !_USER_SESSION__