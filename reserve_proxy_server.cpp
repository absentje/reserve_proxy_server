#include "reserve_proxy_server.h"

Server::Server(boost::asio::io_service& io_service, short local_port, const std::string remote_ip, short remote_port) :
	_io_service(io_service), acceptor_(io_service, tcp::endpoint(tcp::v4(), local_port)),
	_remote_ip(remote_ip), _remote_port(remote_port)
{
	const char NAME_FILE[] = "file_log.txt";
	file_log = boost::shared_ptr<ofstream>(new ofstream(NAME_FILE));
	start_accept();
}

boost::shared_ptr<UserSession> Server::create_session() {
	return boost::shared_ptr<UserSession>(new UserSession(_io_service, file_log));
}

bool Server::start_accept() {
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

void Server::handle_accept(const boost::system::error_code& error) {
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