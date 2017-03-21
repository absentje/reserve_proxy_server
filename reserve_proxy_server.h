#ifndef _RESERVE_PROXY_SERVER__
#define _RESERVE_PROXY_SERVER__

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
// include project-files
#include "user_session.h"

using namespace std;
using boost::asio::ip::tcp;

class Server {
public:
	Server(boost::asio::io_service& io_service, short local_port, const std::string remote_ip, short remote_port);
	boost::shared_ptr<UserSession> create_session();
	bool start_accept();
	void handle_accept(const boost::system::error_code& error);

private:
	boost::asio::io_service& _io_service;
	tcp::acceptor acceptor_;
	boost::shared_ptr<UserSession> session;
	std::string _remote_ip;
	short _remote_port;
	boost::shared_ptr<ofstream> file_log;
};


#endif // !_RESERVE_PROXY_SERVER__
