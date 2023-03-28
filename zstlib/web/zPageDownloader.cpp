#include "stdafx.h"
#include "zPageDownloader.h"


#ifdef ZZZ_MEMORY_LEAK_DETECTION
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace std;


//------------------------------------------------------------------------------------------
namespace zCurlUtl
{
	size_t my_fwrite(void *buffer, size_t size, size_t nmemb, void *stream)
	{
		struct FtpFile *out=(struct FtpFile *)stream;
		if(out && !out->stream)
		{
			//open file for writing
			if(!out->m_wfile_name.empty())
				out->stream = _wfopen(out->m_wfile_name.c_str(), L"wb");
			else
				out->stream = fopen(out->m_file_name.c_str(), "wb");
			if(!out->stream)
				return (_ass(0), -1);	//failure, can't open file to write
		}
		return out ? fwrite(buffer, size, nmemb, out->stream) : 0;
	}
	const int TIME_OUT = 60;
	const string USER_AGENT = "Mozilla/5.0 (Windows; U; Windows NT 5.1; en-US; rv:1.9.1) Gecko/20090624 Firefox/3.5";
}


//------------------------------------------------------------------------------------------
zPageDownloader::zPageDownloader() :
	m_mh(0),
	m_add_url_to_file(true),
	m_no_follow_location(false)
{
	m_mh_files.reserve(MAX_MH_CNT);
	m_mh_last_errors.reserve(MAX_MH_CNT);
}


//------------------------------------------------------------------------------------------
zPageDownloader::~zPageDownloader()
{
	_ass(get_mh_cnt() == 0);	//если assert, то кто-то что-то не докачал
	_cleanup_mh();
}


//------------------------------------------------------------------------------------------
//если time_out ноль, то берется дефолтовый
bool zPageDownloader::get_UrlToFile(std::string sUrl, std::string sFileName, int time_out/* = 0*/)
{
	return get_UrlToFile(sUrl, zstr::s_to_w(sFileName), time_out);
}


//------------------------------------------------------------------------------------------
//если time_out ноль, то берется дефолтовый
bool zPageDownloader::get_UrlToFile(std::string sUrl, std::wstring sFileName, int time_out/* = 0*/)
{
	static char err_buf[1024];

	CURL *curl;
	CURLcode res = CURLE_OK;

	if(time_out == 0)
		time_out = zCurlUtl::TIME_OUT;

	curl = curl_easy_init();
	if(curl)
	{
		struct zCurlUtl::FtpFile ftpfile(sFileName);
		ftpfile.m_url = sUrl;	//для отладки
		//
		curl_easy_setopt(curl, CURLOPT_URL, sUrl.c_str());
		curl_easy_setopt(curl, CURLOPT_USERAGENT, zCurlUtl::USER_AGENT.c_str());
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, zCurlUtl::my_fwrite);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &ftpfile);
		curl_easy_setopt(curl, CURLOPT_TIMEOUT, time_out);
		curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, &err_buf[0]);
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, m_no_follow_location ? 0 : 1);
		//
		if(!m_cookie_file.empty())
		{
			curl_easy_setopt(curl, CURLOPT_COOKIEFILE, m_cookie_file.c_str());
			curl_easy_setopt(curl, CURLOPT_COOKIEJAR, m_cookie_file.c_str());
		}
		//
		res = curl_easy_perform(curl);

		// always cleanup
		curl_easy_cleanup(curl);

		//если нужно, дописываем в конец файла url, с которого сохранили
		if(m_add_url_to_file && ftpfile.stream)
			fprintf(ftpfile.stream, "\n<!--%s-->\n", sUrl.c_str());

		if(ftpfile.stream)
			fclose(ftpfile.stream);
	}

	bool ret = (res == CURLE_OK);

	if(!ret)
		m_last_error = err_buf;

	_ass(ret);
	return ret;
}


//------------------------------------------------------------------------------------------
//инициализация multi-handle-режима
bool zPageDownloader::init_mh_task()
{
	m_mh_last_errors.clear();

	_cleanup_mh();
	m_mh = curl_multi_init(); 

	return true;
}


//------------------------------------------------------------------------------------------
//добавить еще одну задачу (для multi-handle-режима)
void zPageDownloader::add_mh_task(std::string url, std::wstring file_name, int time_out/* = 0*/)
{
	if(m_mh_handles.size() >= MAX_MH_CNT)
		return (_ass(0));

	CURL *curl = curl_easy_init();
	if(!curl)
		return (_ass(0));

	if(time_out == 0)
		time_out = zCurlUtl::TIME_OUT;

	m_mh_handles.push_back(curl);
	m_mh_files.push_back(zCurlUtl::FtpFile(file_name, url));
	m_mh_last_errors.push_back(vector<char>(1024, 0));

	int ind = (int)m_mh_files.size() - 1;
	_ass(m_mh_handles.size() == m_mh_files.size() && m_mh_handles.size() == m_mh_last_errors.size());

	zCurlUtl::FtpFile *ftpfile = &(m_mh_files[ind]);

	curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
	curl_easy_setopt(curl, CURLOPT_USERAGENT, zCurlUtl::USER_AGENT.c_str());
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, zCurlUtl::my_fwrite);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, ftpfile);
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, time_out);
	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, m_no_follow_location ? 0 : 1);

	if(!m_cookie_file.empty())
	{
		curl_easy_setopt(curl, CURLOPT_COOKIEFILE, m_cookie_file.c_str());
		curl_easy_setopt(curl, CURLOPT_COOKIEJAR, m_cookie_file.c_str());
	}

	curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, &(m_mh_last_errors[ind][0]));

	curl_multi_add_handle(m_mh, curl);
}


//------------------------------------------------------------------------------------------
//выполнить все задачи (для multi-handle-режима)
bool zPageDownloader::perform_mh_task()
{
	if(get_mh_cnt() == 0)
		return true;

	int running = 0;
	while(1)
	{
		curl_multi_perform(m_mh, &running);
		if(running <= 0)
			break;
	};

	//проверяем, что всё нормально
	bool ok = true;
	while(1)
	{
		int msgs_in_queue = 0;
		CURLMsg *msg = curl_multi_info_read(m_mh, &msgs_in_queue);

		if(msg && msg->data.result != CURLE_OK)
			ok = false;

		if(msgs_in_queue <= 0)
			break;
	}

	//чистим всё
	_cleanup_mh();

	//_ass(ok);
	if(!ok)
		_warn("problem in perform_mh_task()");
	return ok;
}


//------------------------------------------------------------------------------------------
//очищает текущий multi-handle-режим
void zPageDownloader::_cleanup_mh()
{
	if(!m_mh)
		return;

	//чистим дочерние handles
	for(int i = 0; i < (int)m_mh_handles.size(); i++)
	{
		curl_multi_remove_handle(m_mh, m_mh_handles[i]);
		curl_easy_cleanup(m_mh_handles[i]);
	}
	m_mh_handles.clear();

	//закрываем m_mh
	curl_multi_cleanup(m_mh);
	m_mh = 0;

	//закрываем файлы
	for(int i = 0; i < (int)m_mh_files.size(); i++)
	{
		if(m_mh_files[i].stream)
		{
			//если нужно, дописываем в конец файла url, с которого сохранили
			if(m_add_url_to_file)
				fprintf(m_mh_files[i].stream, "\n<!--%s-->\n", m_mh_files[i].m_url.c_str());

			fclose(m_mh_files[i].stream);
		}
	}
	m_mh_files.clear();
}


//------------------------------------------------------------------------------------------
//последняя ошибка
std::string zPageDownloader::get_last_error()
{
	if(m_mh_last_errors.empty())
		return m_last_error;

	string ret;
	for(int i = 0; i < (int)m_mh_last_errors.size(); i++)
	{
		string str = (const char *)(&(m_mh_last_errors[i][0]));

		if(!str.empty())
		{
			ret += str;
			ret += "\n";
		}
	}

	return ret;
}


//------------------------------------------------------------------------------------------
//удобная закачка в несколько одновременных потоков
//то же, что и add_mh_task, но само делает всё, что надо:
//init_mh_task() и perform_mh_task(), если количество в очереди достигло max_queue_size
void zPageDownloader::add_perform_mh_task(std::string url, std::wstring file_name, int max_queue_size, int time_out/* = 0*/)
{
	if(!m_mh)
		init_mh_task();

	add_mh_task(url, file_name, time_out);

	if(get_mh_cnt() >= max_queue_size)
	{
		bool ok = perform_mh_task();
		if(!ok)
			zdebug::LogImp("curl error: " + get_last_error());

		_cleanup_mh();
	}
}

