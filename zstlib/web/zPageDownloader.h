#pragma once

#include <curl/curl.h>

//------------------------------------------------------------------------------------------
namespace zCurlUtl
{
	struct FtpFile
	{
		std::string m_file_name;
		std::wstring m_wfile_name;
		std::string m_url;	//храним чисто для отладки (и чтоб в конце скачанного файла выводить)

		FILE *stream;

		FtpFile() : stream(0)
		{
		}
		FtpFile(std::string file_name) : stream(0)
		{
			m_file_name = file_name;
		}
		FtpFile(std::wstring wfile_name) : stream(0)
		{
			m_wfile_name = wfile_name;
		}
		FtpFile(std::wstring wfile_name, std::string url) : stream(0)
		{
			m_wfile_name = wfile_name;
			m_url = url;
		}
	};

	size_t my_fwrite(void *buffer, size_t size, size_t nmemb, void *stream);
	extern const std::string USER_AGENT;
	extern const int TIME_OUT;
};



//------------------------------------------------------------------------------------------
class zPageDownloader
{
	enum
	{
		MAX_MH_CNT = 100	//максимальное количество потоков, сделано только из-за m_mh_files
	};

	//для поточной закачки
	CURLM *m_mh;
	std::vector<CURL*> m_mh_handles;
	std::vector<zCurlUtl::FtpFile> m_mh_files;
	std::vector<std::vector<char>> m_mh_last_errors;

	//файл для хранения cookie
	std::string m_cookie_file;

	std::string m_last_error;

	//если true, то дописывает в конец файла url, с которого сохранили
	bool m_add_url_to_file;

	//автоматом не редиректит (чтоб файлы огромные не качать случайно)
	bool m_no_follow_location;

public:
	zPageDownloader();
	virtual ~zPageDownloader();

	//установить файл для хранения cookie
	void set_cookie_file(std::string cookie_file) { m_cookie_file = cookie_file; };


	//если time_out ноль, то берется дефолтовый
	bool get_UrlToFile(std::string sUrl, std::string sFileName, int time_out = 0);
	bool get_UrlToFile(std::string sUrl, std::wstring sFileName, int time_out = 0);

	/// закачка в несколько одновременных потоков

	//инициализация multi-handle-режима
	bool init_mh_task();
	//добавить еще одну задачу (для multi-handle-режима)
	//если time_out ноль, то берется дефолтовый
	void add_mh_task(std::string url, std::wstring file_name, int time_out = 0);
	//выполнить все задачи (для multi-handle-режима)
	bool perform_mh_task();

	//последняя ошибка
	std::string get_last_error();

	//количество приготовленных для скачивания mh-handles
	int get_mh_cnt() { return (int)m_mh_handles.size(); }

	//удобная закачка в несколько одновременных потоков
	//то же, что и add_mh_task, но само делает всё, что надо:
	//init_mh_task() и perform_mh_task(), если количество в очереди достигло max_queue_size
	void add_perform_mh_task(std::string url, std::wstring file_name, int max_queue_size, int time_out = 0);

	//чтоб автоматом не редиректить (чтоб файлы огромные не качать случайно)
	void set_no_follow_location(bool no_follow_location) { m_no_follow_location = no_follow_location; }

private:
	//очищает текущий multi-handle-режим
	void _cleanup_mh();
};

