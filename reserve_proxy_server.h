#ifndef _RESERVE_PROXY_SERVER__
#define _RESERVE_PROXY_SERVER__

#define _CRT_SECURE_NO_WARNINGS
#include <string>
#include <fstream>

// BOOST includes
#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
// include project-files
#include "user_session.h"

class Server final {
	using tcp = boost::asio::ip::tcp;
public:
	Server(boost::asio::io_service& io_service, short local_port, const std::string remote_ip, short remote_port);
	boost::shared_ptr<UserSession> create_session();
	bool start_accept() noexcept;
	void handle_accept(const boost::system::error_code& error) noexcept;

private:
	boost::asio::io_service& _io_service;
	tcp::acceptor acceptor_;
	boost::shared_ptr<UserSession> session;
	std::string _remote_ip;
	short _remote_port;
	boost::shared_ptr<std::ofstream> file_log;
};


#endif // !_RESERVE_PROXY_SERVER__
