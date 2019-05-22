/*
	Project			 : Wolf Engine. Copyright(c) Pooya Eimandar (http://PooyaEimandar.com) . All rights reserved.
	Source			 : Please direct any bug to https://github.com/PooyaEimandar/Wolf.Engine/issues
	Website			 : http://WolfSource.io
	Name			 : w_compress_data_type.h
	Description		 : data structure types
	Comment          :
*/

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef __W_COMPRESS_DATA_TYPE_H__
#define __W_COMPRESS_DATA_TYPE_H__

typedef enum
{
	W_DEFAULT,
	W_FAST
}
w_compress_mode;

typedef struct
{
	size_t				size_in;
	size_t				size_out;
	char*				data;
} w_compress_result;

#endif //__W_COMPRESS_DATA_TYPE_H__
