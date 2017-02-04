#define _CRT_SECURE_NO_WARNINGS
#define gets gets_s
#include <cstddef>
#include <iostream>
#include <fstream>
#include <cstring>
#include <memory>
#include <thread>
#include <cstdlib>
#include <stdio.h>
using namespace std;
// BOOST includes
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
using boost::asio::ip::tcp;
using namespace boost::asio;

class client {
public:
	//конструктор
	client(boost::asio::io_service& io_service)
		: socket_(io_service)
	{
	}
	void start_connection() {
		const int MAX_IP_PORT = 20;
		char ip[MAX_IP_PORT], port[MAX_IP_PORT];
		//int port;
		cout << "Введите ip адрес сервера :\n";
		gets(ip);
		cout << "Введите порт сервера :\n";
		gets(port);
		ip::tcp::endpoint ep(ip::address::from_string(ip), atoi(port));
		socket_.async_connect(ep, boost::bind(&client::connect_handler, this,
			boost::asio::placeholders::error));
	};
private:
	void connect_handler(const boost::system::error_code & error)
	{
		if (!error) {
			cout << "Введите логин пользователя :\n";
			gets(data_);
			int length_buff = strlen(data_);
			data_[length_buff] = ' '; // отделяем логин от пароля
			length_buff++;
			cout << "Введите пароль :\n";
			gets(&data_[length_buff], max_length_data - length_buff);
			cout << '\n';
			length_buff = strlen(data_);
			
			boost::asio::async_write(socket_,
				boost::asio::buffer(data_, length_buff + 1),
				boost::bind(&client::handle_first_write, this,
					boost::asio::placeholders::error));
		}
		else {
			throw error;
		}
	}
	void handle_first_write(const boost::system::error_code& error) {
		if (!error)													// получаем ответ от сервера
		{															// о правильности логина и пароля
			socket_.async_read_some(boost::asio::buffer(data_, max_length_data),
				boost::bind(&client::handle_first_read, this,
					boost::asio::placeholders::error,
					boost::asio::placeholders::bytes_transferred));
		}
		else {
			throw error;
		}
	}
	void handle_first_read(const boost::system::error_code& error, size_t bytes_transferred){
		if (!error)
		{
			int answer = atoi(data_); // 1 - логин и пароль верны, 0 - неверны
			if (answer)
				handle_read(error, 0);
			else {
				cout << "Неправильные логин или пароль.\n";
				socket_.close();
				start_connection();
			}
		}
		else {
			throw error;
		}
	}
	void handle_write(const boost::system::error_code& error)
	{
		if (!error)
		{
			socket_.async_read_some(boost::asio::buffer(data_, max_length_data),
				boost::bind(&client::handle_read, this,
					boost::asio::placeholders::error,
					boost::asio::placeholders::bytes_transferred));
		}
		else {
			throw error;
		}

	}
	void handle_read(const boost::system::error_code& error, size_t bytes_transferred)
	{
		if (!error)
		{
			if(bytes_transferred)
				cout << data_ << '\n';
			cout << "Введите название базы данных(введите \"quit\" для выхода) :\n";
			gets(data_);
			int length_buff = strlen(data_);
			data_[length_buff] = 0;
			if (!strcmp(data_, "quit")) {
				throw 0;
			}
			data_[length_buff] = ' '; // отделяем название бд от запроса
			length_buff++;
			cout << "Введите запрос (';' - служит окончанием запроса) :\n";
			while (true) {
				bool Flag = false;
				gets(&data_[length_buff], max_length_data - length_buff);
				int n = strlen(data_);
				data_[n] = '\n';
				data_[n + 1] = 0;
				n++;
				for (int i = length_buff; i < n; i++) {
					if (data_[i] == ';') {
						data_[i + 1] = 0;
						length_buff = i + 1;
						Flag = true;
						break;
					}
				}
				length_buff = n;
				if (Flag)
					break;
			}
			cout << '\n';
			if (length_buff == max_length_data) {
				cout << "Ошибка. Переполнение буфера ввода.\n";
				throw 1;
			}
			data_[length_buff] = 0;
			boost::asio::async_write(socket_,
				boost::asio::buffer(data_, length_buff + 1),
				boost::bind(&client::handle_write, this,
					boost::asio::placeholders::error));	
		}
		else {
			throw error;
		}
	}

	//поля
	tcp::socket socket_;
	enum { max_length_data = 5000 };
	char data_[max_length_data];
};


int main(int argc, char* argv[])
{
	setlocale(LC_CTYPE, "Russian");
	try
	{
		io_service service;
		client a(service);
		a.start_connection();
		service.run();
	}
	catch (std::exception& e)
	{
		cerr << "Exception: " << e.what() << "\n";
		system("pause");
	}
	catch (boost::system::error_code err) {
		cout << err.message();
		system("pause");
	}
	catch (int c) {
		return c;
	}
	return 0;
}