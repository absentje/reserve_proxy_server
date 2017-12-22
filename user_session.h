#ifndef _USER_SESSION__
#define _USER_SESSION__

#define _CRT_SECURE_NO_WARNINGS
#include <cstddef>
#include <fstream>
#include <string>

// BOOST includes
#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>


class UserSession final : public boost::enable_shared_from_this<UserSession> {
	using ErrorCode = boost::system::error_code;
	using tcp = boost::asio::ip::tcp;

public:
	UserSession(boost::asio::io_service& io_service, boost::shared_ptr<std::ofstream> file);

	const tcp::socket& get_client_socket() const noexcept;
	tcp::socket& get_client_socket() noexcept;

	void start(const std::string& proxy_ip, short proxy_port) noexcept;

	void handle_connect(const ErrorCode& error) noexcept;

	void handle_client_read(const ErrorCode& error, const std::size_t bytes_transferred) noexcept;

	void handle_proxy_write(const ErrorCode& error) noexcept;

	void handle_proxy_read(const ErrorCode& error, const std::size_t bytes_transferred) noexcept;

	void handle_client_write(const ErrorCode& error) noexcept;

	void close() noexcept;

private:
	enum { default_buffer_size = 8192 };
	tcp::socket _client_socket;
	tcp::socket _proxy_socket;
	char _client_buffer[default_buffer_size];
	char _proxy_buffer[default_buffer_size];
	boost::shared_ptr<std::ofstream> _file_log;
};

#endif // !_USER_SESSION__
