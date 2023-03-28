#pragma once


enum HTML_ITEM_TYPE
{
	HTML_UNKNOWN = 0,
	HTML_COMMENT,
	HTML_TAG,
	HTML_TEXT,
	HTML_PHP
};


//флаги для zHtmlItem
enum
{
	HF_STRANGE_TAG = 0x1,		//какой-то странный тэг, который не хотим парсить, например <!DOCTYPE html или <![cdata
	HF_DONT_PRINT = 0x2,
	HF_CLOSE_TAG = 0x4,			//закрывающий тэг
	HF_SELF_CLOSE_TAG = 0x8,	//самозакрывающий тэг, типа '<br/>'
};


//флаги для zHtmlValue
enum
{
	HVF_HIDDEN = 0x1,			//<!DOCTYPE html
	HVF_QUOTES = 0x2,			//при считывании значения у него были кавычки, но мы их отрубили
};

