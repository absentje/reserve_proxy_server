#include "reserve_proxy_server.h"
#include <iostream>
#include <boost/bind.hpp>

using boost::asio::ip::tcp;
using std::ofstream;
using std::cerr;
using std::endl;

Server::Server(boost::asio::io_service& io_service, short local_port, const std::string remote_ip, short remote_port) 
	:	_io_service(io_service), acceptor_(io_service, tcp::endpoint(tcp::v4(), local_port)),
	_remote_ip(remote_ip), _remote_port(remote_port) 
{
	static const char NAME_FILE[] = "file_log.txt";
	file_log = boost::shared_ptr<ofstream>(new ofstream(NAME_FILE));
	start_accept();
}

boost::shared_ptr<UserSession> Server::create_session() {
	return boost::shared_ptr<UserSession>(new UserSession(_io_service, file_log));
}

bool Server::start_accept() noexcept {
	try {
		session = create_session();
		acceptor_.async_accept(session->get_client_socket(),
			boost::bind(&Server::handle_accept, this,
				boost::asio::placeholders::error));
	}
	catch (const std::exception& e) {
		cerr << "create_session exception: " << e.what() << endl;
		return false;
	}
	catch (...) {
		cerr << "another exception, need to check it." << endl;
	}
	return true;
}

void Server::handle_accept(const boost::system::error_code& error) noexcept {
	if (!error) {
		session->start(_remote_ip, _remote_port);
		if (!start_accept()) {
			cerr << "Failed to accept.\n\n";
		}
	}
	else {
		cerr << error.message() << endl;
	}
}
