#include "stdafx.h"
#include "zPageDownloader.h"

#include <curl/curl.h>


//------------------------------------------------------------------------------------------
namespace zCurlUtl
{
	size_t my_fwrite(void *buffer, size_t size, size_t nmemb, void *stream)
	{
		struct FtpFile *out=(struct FtpFile *)stream;
		if(out && !out->stream) {
		//open file for writing
		out->stream=fopen(out->filename, "wb");
		if(!out->stream)
		  return -1;	///failure, can't open file to write
		}
		return fwrite(buffer, size, nmemb, out->stream);
	}
	const int TIME_OUT = 20;
	const std::string USER_AGENT = "Mozilla/5.0 (Windows; U; Windows NT 5.1; en-US; rv:1.9.1) Gecko/20090624 Firefox/3.5";
	//const std::string WORK_FOLDER = "wrk/";
	//const std::string COOKIE_FILE = "__cookies.txt";
	//
	std::string g_sImgPart = "";
}


//------------------------------------------------------------------------------------------
zPageDownloader::zPageDownloader()
{
}


//------------------------------------------------------------------------------------------
zPageDownloader::~zPageDownloader()
{
}


//------------------------------------------------------------------------------------------
bool zPageDownloader::get_UrlToFile(std::string sUrl, std::string sFileName)
{
	CURL *curl;
	CURLcode res = CURLE_OK;

	curl = curl_easy_init();
	if(curl)
	{
		struct zCurlUtl::FtpFile ftpfile = {sFileName.c_str(), NULL};
		//
		curl_easy_setopt(curl, CURLOPT_URL, sUrl.c_str());
		curl_easy_setopt(curl, CURLOPT_USERAGENT, zCurlUtl::USER_AGENT.c_str());
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, zCurlUtl::my_fwrite);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &ftpfile);
		curl_easy_setopt(curl, CURLOPT_TIMEOUT, zCurlUtl::TIME_OUT);
		//
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
		//
		res = curl_easy_perform(curl);

		// always cleanup
		curl_easy_cleanup(curl);
		if(ftpfile.stream)
			fclose(ftpfile.stream);
	}

	return res == CURLE_OK;
}

