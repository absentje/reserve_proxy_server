#define _CRT_SECURE_NO_WARNINGS
#define __LCC__
#include <cstddef>
#include <iostream>
#include <fstream>
#include <cstring>
#include <memory>
#include <thread>
#include <cstdlib>
#include <cstdio>
#include <ctime>
#include <mysql.h>
#include "mysql_function.h"

// BOOST includes
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

using boost::asio::ip::tcp;

class Session
{
public:
	Session(boost::asio::io_service& io_service, ofstream * file)
		: socket_(io_service)
	{
		fileLog = file;
	}

	tcp::socket& socket()
	{
		return socket_;
	}

	void start()
	{
		socket_.async_read_some(boost::asio::buffer(data_, max_length_data),
			boost::bind(&Session::handle_first_read, this,
				boost::asio::placeholders::error,
				boost::asio::placeholders::bytes_transferred));
	}

private:
	void handle_first_read(const boost::system::error_code& error, size_t bytes_transferred)
	{						//считываем и проверяем правильность введенных логин+пароль
		if (!error)
		{
			int ii = 0, length_login = 0, length_passwd = 0;
			login_[0] = 0;
			passwd_[0] = 0;
			while ((data_[ii] != ' ') && (data_[ii] != 0) && (ii < max_length_login)) {
				login_[ii] = data_[ii];
				ii++;
			}
			length_login = ii;
			if (data_[ii] == ' ') {
				length_passwd = 1;
			}
			else {
				length_passwd = 0;
			}
			if (ii == max_length_login) {
				login_[ii - 1] = 0;
			}
			else
				login_[ii] = 0;
			ii = 0;
			if (length_passwd) {
				while ((data_[length_login + 1 + ii] != 0) && (ii < max_length_passwd)) {
					passwd_[ii] = data_[length_login + 1 + ii];
					ii++;
				}
			}
			if (ii == max_length_passwd) {
				passwd_[ii - 1] = 0;
			}
			else
				passwd_[ii] = 0;
			if (conn_user(login_, passwd_))
				strcpy(data_, "1\0");
			else
				strcpy(data_, "0\0");
			boost::asio::async_write(socket_,
				boost::asio::buffer(data_, 2),
				boost::bind(&Session::handle_write, this,
					boost::asio::placeholders::error));
		}
		else
		{
			delete this;
		}
	}
	void handle_read(const boost::system::error_code& error, size_t bytes_transferred)
	{
		if (!error)
		{
			if (fileLog->is_open()) {
				*fileLog << "client ip:\n" << socket_.local_endpoint().address().to_string() << '\n';
				*fileLog << "client login and password:\n" << login_ << ' ' << passwd_ << '\n';
				*fileLog << "database and query:\n" << data_ << "\n\n";
			}
			int length_db = 0;
			while (data_[length_db] != ' ') {
				length_db++;
			}
			data_[length_db] = 0;
			char data_base[max_length_db];
			char query[max_length_query];
			strcpy(query, &data_[length_db + 1]);
			data_[max_length_db] = 0;
			strcpy(data_base, data_);
			mysql_function(query, "localhost", login_, passwd_, data_base, 3306);
			ifstream fin(resultOfQuery);
			if (!fin.is_open()) // если файл не открыт
				delete this;
			else {
				int ii = 0;
				while (fin.get(data_[ii])) {
					ii++;
				}
				data_[ii] = 0;
				boost::asio::async_write(socket_,
					boost::asio::buffer(data_, strlen(data_) + 1),
					boost::bind(&Session::handle_write, this,
						boost::asio::placeholders::error));
			}
		}
		else
		{
			delete this;
		}
	}

	void handle_write(const boost::system::error_code& error)
	{
		if (!error)
		{
			socket_.async_read_some(boost::asio::buffer(data_, max_length_data),
				boost::bind(&Session::handle_read, this,
					boost::asio::placeholders::error,
					boost::asio::placeholders::bytes_transferred));
		}
		else
		{
			delete this;
		}
	}

	tcp::socket socket_;
	enum {
		max_length_data = 5000, max_length_login = 25, max_length_passwd = 25, max_length_db = 25,
		max_length_query = 4000
	};
	char data_[max_length_data];
	char login_[max_length_login];
	char passwd_[max_length_passwd];
	const char resultOfQuery[18] = "resultOfQuery.txt";
	ofstream * fileLog;
};

class Server
{
public:
	Server(boost::asio::io_service& io_service, short port)
		: io_service_(io_service),
		acceptor_(io_service, tcp::endpoint(tcp::v4(), port))
	{
		
		const char NAME_FILE[] = "file_log.txt";
		file_log = new ofstream(NAME_FILE);
		start_accept();
	}
	~Server() {
		if (file_log) {
			file_log->close();
			delete file_log;
		}
	}

private:
	void start_accept()
	{
		Session* new_session = new Session(io_service_, file_log);
		acceptor_.async_accept(new_session->socket(),
			boost::bind(&Server::handle_accept, this, new_session,
				boost::asio::placeholders::error));
	}

	void handle_accept(Session* new_session,
		const boost::system::error_code& error)
	{
		if (!error)
		{
			new_session->start();
		}
		else
		{
			delete new_session;
		}

		start_accept();
	}

	boost::asio::io_service& io_service_;
	tcp::acceptor acceptor_;
	ofstream * file_log;
};

int main(int argc, char* argv[])
{
	setlocale(LC_CTYPE, "Russian");
	try
	{
		cout << "Введите порт открываемого сервера :\n";
		int port;
		cin >> port;
		boost::asio::io_service io_service;

		using namespace std; // For atoi.
		Server s(io_service, port);

		io_service.run();
	}
	catch (std::exception& e)
	{
		std::cerr << "Exception: " << e.what() << "\n";
		system("pause");
	}

	return 0;
}
