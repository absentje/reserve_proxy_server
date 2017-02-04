#define _CRT_SECURE_NO_WARNINGS
#define __LCC__
#include <mysql.h>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <cstddef>
using namespace std;


// функция проверяющая правильность введенных логин + пароль
bool conn_user(const char user[], const char passwd[])
{
	MYSQL mysql; // Дескриптор соединения
	mysql_init(&mysql);  // Инициализация
	if (mysql_real_connect(&mysql, "localhost", user, passwd, "", 3306, NULL, 0)) {
		return true;
	}
	else
		return false;
}
// функция обращающаяся к БД с запросом
void mysql_function(const char query[],
	const char host[] = "localhost", const char user[] = "admin", const char passwd[] = "passwd123321",
	const char db[] = "cpp_data", const int port = 3306) // host[]-хост, user[]-пользователь, passwd[]-пароль, 
														 // library[]-название бд, port-порт
{
	const char resultOfQuery[] = "resultOfQuery.txt";
	ofstream fout(resultOfQuery); // рез-т запроса выведется в этот файл
	if (!fout.is_open()) // если файл небыл открыт
	{
		throw 1; // ошибка при открытии файла
	}
	MYSQL mysql;
	MYSQL_RES *res;
	MYSQL_ROW row;
	mysql_init(&mysql);
	mysql_real_connect(&mysql, host, user, passwd, db, port, NULL, 0);
	if (mysql_query(&mysql, query) > 0) // запрос. Если ошибок нет, то продолжаем работу
	{
		// Если была ошибка, ...
		fout << mysql_error(&mysql);	// ... выведем ее
		return; // и завершим работу
	}
	res = mysql_store_result(&mysql); // Берем результат,
	int num_fields = mysql_num_fields(res); // количество полей
	long long num_rows = mysql_num_rows(res); // и количество строк.
	size_t t = 0; // счетчик для второй строки, заполненной '-'
	for (int i = 0; i < num_fields; i++) // Выводим названия полей
	{
		char *field_name = mysql_fetch_field_direct(res, i)->name;
		fout << setw(strlen(field_name) + 3) << field_name << " |";
		t += strlen(field_name) + 5;
	}

	fout << '\n';
	for (int i = 0; i < t; i++) {	// отделяем шапку таблицы от данных
		fout << '-';
	}
	fout << '\n';

	for (int i = 0; i < num_rows; i++) // Вывод таблицы
	{
		row = mysql_fetch_row(res); // получаем строку
		for (int l = 0; l < num_fields; l++) {
			char *field_name = mysql_fetch_field_direct(res, l)->name; // необходимо для выравнивания рез-та
			fout << setw(strlen(field_name) + 3) << row[l] << " |";
		}
		fout << "\n";
	}
	fout << "Count records = " << num_rows; // Вывод информации о количестве записей
	mysql_free_result(res); // очищаем результаты
	mysql_close(&mysql); // закрываем соединение
	fout.close(); // закрываем файл
}

