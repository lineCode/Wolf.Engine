#include "w_io.h"
#include <apr-1/apr_strings.h>

#include "base64/chromiumbase64.h"
#include "base64/fastavx512bwbase64.h"
#include "base64/fastavxbase64.h"
#include "base64/klompavxbase64.h"
#include "base64/quicktimebase64.h"
#include "base64/scalarbase64.h"

#ifdef W_PLATFORM_UNIX
#include "base64/linuxbase64.h"
#include <sys/stat.h>
#include <libgen.h>
#endif

w_file w_io_create_file(_In_z_  const char* pPath,
                        _In_z_  const char* pContent,
                        _In_    const bool pBinaryMode,
                        _In_    const bool pBufferedMode,
                        _In_    const bool pNoneBlockMode,
                        _In_    const bool pMultiThreadedMode,
                        _In_    const bool pOpenAppendMode,
                        _In_    const bool pIsLargFile,
                        _In_    const bool pErrorIfFileExists)
{
    apr_status_t _ret = APR_SUCCESS;
    apr_file_t* _file = NULL;
    apr_size_t  _buffer_len = 0;
    apr_int32_t _flags = 0;
    w_mem_pool _pool = w_get_default_memory_pool();
    if(!_pool)
    {
        W_ASSERT(false, "could not get default memory. trace info: w_io_create_directory");
        return NULL;
    }
    
    _flags = APR_FOPEN_CREATE | // create file if not there
             APR_FOPEN_WRITE;   // open for writing
    
    if (pBinaryMode)
    {
        _flags |= APR_FOPEN_BINARY; // binary mode (ignored on UNIX)
    }
    if (pBufferedMode)
    {
        _flags |= APR_FOPEN_BUFFERED; // buffered mode
    }
    if (pNoneBlockMode)
    {
        _flags |= APR_FOPEN_NONBLOCK; // none blocked mode
    }
    if (pMultiThreadedMode)
    {
        _flags |= APR_FOPEN_XTHREAD; // allow multiple threads to use file
    }
    if (pOpenAppendMode)
    {
        _flags |= APR_FOPEN_APPEND; // binary mode (ignored on UNIX)
    }
    if (pIsLargFile)
    {
        _flags |= APR_FOPEN_LARGEFILE; // binary mode (ignored on UNIX)
    }
    if (pErrorIfFileExists)
    {
         _flags |= APR_FOPEN_EXCL; // binary mode (ignored on UNIX)
    }
    
    _ret = apr_file_open(
        &_file, // new file handle
        pPath, // file name
        _flags,
        APR_OS_DEFAULT | 0, // permissions
        _pool // memory pool to use
    );
    if (_ret != APR_SUCCESS)
    {
        return NULL;
    }
    _buffer_len = strlen(pContent);
    _ret = apr_file_write(_file, pContent, &_buffer_len);
    
    return _file;
}

W_RESULT w_io_save_to_file(_In_z_  const char* pPath,
                           _In_z_  const char* pContent,
                           _In_    const bool pBinaryMode,
                           _In_    const bool pBufferedMode,
                           _In_    const bool pNoneBlockMode,
                           _In_    const bool pMultiThreadedMode,
                           _In_    const bool pOpenAppendMode,
                           _In_    const bool pIsLargFile,
                           _In_    const bool pErrorIfFileExists)
{
    apr_status_t _ret = APR_SUCCESS;
    apr_file_t* _file = NULL;
    apr_size_t  _buffer_len = 0;
    apr_int32_t _flags = 0;
    w_mem_pool _pool = w_get_default_memory_pool();
    if(!_pool)
    {
        W_ASSERT(false, "could not get default memory. trace info: w_io_create_directory");
        goto __return;
    }
    
    _flags = APR_FOPEN_CREATE | // create file if not there
             APR_FOPEN_WRITE;   // open for writing
    
    if (pBinaryMode)
    {
        _flags |= APR_FOPEN_BINARY; // binary mode (ignored on UNIX)
    }
    if (pBufferedMode)
    {
        _flags |= APR_FOPEN_BUFFERED; // buffered mode
    }
    if (pNoneBlockMode)
    {
        _flags |= APR_FOPEN_NONBLOCK; // none blocked mode
    }
    if (pMultiThreadedMode)
    {
        _flags |= APR_FOPEN_XTHREAD; // allow multiple threads to use file
    }
    if (pOpenAppendMode)
    {
        _flags |= APR_FOPEN_APPEND; // binary mode (ignored on UNIX)
    }
    if (pIsLargFile)
    {
        _flags |= APR_FOPEN_LARGEFILE; // binary mode (ignored on UNIX)
    }
    if (pErrorIfFileExists)
    {
         _flags |= APR_FOPEN_EXCL; // binary mode (ignored on UNIX)
    }
    
    _ret = apr_file_open(
        &_file, // new file handle
        pPath, // file name
        _flags,
        APR_OS_DEFAULT | 0, // permissions
        _pool // memory pool to use
    );
    if (_ret != APR_SUCCESS)
    {
        goto __return;
    }
    _buffer_len = strlen(pContent);
    _ret = apr_file_write(_file, pContent, &_buffer_len);
    
__return:
    apr_file_close(_file);
    return _ret == APR_SUCCESS ? W_SUCCESS : W_FAILURE;
}

W_RESULT w_io_get_is_file(_In_z_ const char* pPath)
{
#ifdef W_PLATFORM_UNIX
    struct stat _stat;
    int _ret = stat(pPath, &_stat);
    return _ret == -1 ? W_FAILURE : W_SUCCESS;
#else
    //on Windows
    FILE* _file = NULL;
    fopen_s(&_file, pPath, "r");
    if (_file)
    {
        fclose(_file);
        return W_PASSED;
    }
    return W_FAILED;
#endif
}

w_file_info w_io_get_file_info_from_path(_In_z_ const char* pPath)
{
    w_mem_pool _pool = w_get_default_memory_pool();
    if(!_pool)
    {
        W_ASSERT(false, "could not get default memory. trace info: w_io_get_file_info_from_path");
        return NULL;
    }
    
    apr_finfo_t* _info = apr_palloc(_pool, sizeof(apr_finfo_t));
    if(!_info)
    {
        W_ASSERT(false, "could not allocate memory for apr_finfo_t. trace info: w_io_get_file_info_from_path");
        return NULL;
    }
    
    apr_status_t _ret = apr_stat(_info, pPath, APR_FINFO_NORM, _pool);
    if (_ret == APR_SUCCESS) return _info;
    
    return NULL;
}

/**
 * check file info
 * @param pFile opened file
 * @return file info
*/
w_file_info w_io_get_file_info(_In_ w_file pFile)
{
    w_mem_pool _pool = w_get_default_memory_pool();
    if(!_pool)
    {
        W_ASSERT(false, "could not get default memory. trace info: w_io_get_file_info");
        return NULL;
    }
    
    apr_finfo_t* _info = apr_palloc(_pool, sizeof(apr_finfo_t));
    if(!_info)
    {
        W_ASSERT(false, "could not allocate memory for apr_finfo_t. trace info: w_io_get_file_info");
        return NULL;
    }
    
    apr_status_t _ret = apr_file_info_get(_info, APR_FINFO_NORM, pFile);
    if (_ret == APR_SUCCESS) return _info;

    return NULL;
}

const char* w_io_get_file_extension_from_path(_In_z_ const char* pFilePath)
{
    const char* _dot = strrchr(pFilePath, '.');
    if(!_dot || _dot == pFilePath) return "";
    return _dot + 1;
}

const char* w_io_get_file_extension(_In_ w_file pFile)
{
    apr_finfo_t* _info = w_io_get_file_info(pFile);
    if (!_info) return "";
    return w_io_get_file_extension_from_path(_info->fname);
}

const char* w_io_get_base_file_name_from_path(_In_z_ const char* pPath)
{
    // traverse from right
    size_t _len = strlen(pPath);
    size_t _index = -1;
    for (size_t i = _len - 1; i >= 0; i--)
    {
        if (pPath[i] == '.')
        {
            _index = i;
            break;
        }
    }
    
    if (_index == -1) return "";
    
    char* _dst_str = w_alloc(_index, "w_io_get_base_file_name_from_path");
    apr_cpystrn(_dst_str, pPath,_index);
    _dst_str[_index] = '\0';
    
    return _dst_str;
}

const char* w_io_get_base_file_name(_In_ w_file pFile)
{
    apr_finfo_t* _info = w_io_get_file_info(pFile);
    return _info->fname;
}

const char* w_io_get_file_name_from_path(_In_z_ const char* pFilePath)
{
#ifdef W_PLATFORM_UNIX
    char* _s = strdup(pFilePath);
    return basename(_s);
#else
    
#endif
}

const char*	w_io_get_file_name(_In_ w_file pFile)
{
    apr_finfo_t* _info = w_io_get_file_info(pFile);
    if (!_info) return "";
    return _info->name;
}

void*	w_io_read_file_with_path(_In_z_ const char* pPath)
{
    void* _buf = NULL;
    w_mem_pool _pool = w_get_default_memory_pool();
    if(!_pool)
    {
        W_ASSERT(false, "could not get default memory. trace info: w_io_create_directory");
        return NULL;
    }
    
    apr_file_t* _file = NULL;
    apr_status_t _ret = apr_file_open(&_file,
                       pPath,
                       APR_FOPEN_READ | APR_FOPEN_BUFFERED,
                       0,
                       _pool);
    if (_ret == APR_SUCCESS)
    {
        //read it all
        apr_size_t _size_of_file = apr_file_buffer_size_get(_file);
        apr_size_t _bytes_read = 0;
        apr_file_read_full(_file, _buf, _size_of_file, &_bytes_read);
        apr_file_close(_file);
    }
    return _buf;
}

void*	w_io_read_file_with_file(_In_z_ w_file pFile)
{
    void* _buf = NULL;
    w_mem_pool _pool = w_get_default_memory_pool();
    if(!_pool)
    {
        W_ASSERT(false, "could not get default memory. trace info: w_io_create_directory");
        return NULL;
    }
    
    //read it all
    apr_size_t _bytes_read = 0;
    apr_size_t _size_of_file = apr_file_buffer_size_get(pFile);
    apr_file_read_full(pFile, _buf, _size_of_file, &_bytes_read);
    apr_file_close(pFile);
    
    return _buf;
}

W_RESULT	w_io_delete_file_with_path(_In_ const char* pPath)
{
    if (!pPath)
    {
        W_ASSERT(false, "pPath is NULL. trace info: w_io_delete_file_with_path");
        return W_FAILURE;
    }
    
    w_mem_pool _pool = w_get_default_memory_pool();
    if(!_pool)
    {
        W_ASSERT(false, "could not get default memory. trace info: w_io_delete_file_with_path");
        return W_FAILURE;
    }
    apr_status_t _ret = apr_file_remove(pPath, _pool);
    return _ret == APR_SUCCESS ? W_SUCCESS : W_FAILURE;
}

W_RESULT	w_io_delete_file_with_file(_In_ w_file pFile)
{
    if (!pFile)
    {
        W_ASSERT(false, "could not get default memory. trace info: w_io_delete_file_with_file");
        return W_FAILURE;
    }
    w_mem_pool _pool = w_get_default_memory_pool();
    if(!_pool)
    {
        W_ASSERT(false, "could not get default memory. trace info: w_io_delete_file_with_file");
        return W_FAILURE;
    }
    apr_finfo_t* _file = w_io_get_file_info(pFile);
    apr_status_t _ret = apr_file_remove(_file->fname, _pool);
    return _ret == APR_SUCCESS ? W_SUCCESS : W_FAILURE;
}

char*	w_io_get_current_directory(void)
{
    char* _path = (char*)w_alloc(PATH_MAX, "w_io_get_current_directory");
#ifdef W_PLATFORM_UNIX
    if (getcwd(_path, PATH_MAX) == NULL) return NULL;
#else
    //convert get_current_directoryW(); to char*
#endif
    return _path;
}

W_RESULT	w_io_get_is_directory(_In_z_ const char* pPath)
{
    w_mem_pool _pool = w_get_default_memory_pool();
    if(!_pool)
    {
        W_ASSERT(false, "could not get default memory. trace info: w_io_get_is_directory");
        return W_FAILURE;
    }
    
    //open a directory
    apr_dir_t* _dir = NULL;
    apr_status_t _ret = apr_dir_open(&_dir, pPath, _pool);
    return _ret == APR_SUCCESS ? W_SUCCESS : W_FAILURE;
}

const char* w_io_get_parent_directory(_In_z_ const char* pPath)
{
#ifdef W_PLATFORM_UNIX
    char* _s = strdup(pPath);
    return dirname(_s);
#else
    apr_size_t _size = 0;
    for (int i = (int)strlen(pPath) - 1; i >= 0; i--)
    {
        if (pPath[i] == '\\' || pPath[i] == '/')
        {
            _size = (apr_size_t)i;
            break;
        }
    }
    //copy strings
    if (_size == 0) return "";

    return w_string(pPath);
#endif
}

W_RESULT	w_io_create_directory(_In_z_ const char* pPath)
{
    w_mem_pool _pool = w_get_default_memory_pool();
    if(!_pool)
    {
        W_ASSERT(false, "could not get default memory. trace info: w_io_create_directory");
        return W_FAILURE;
    }
    
    //create a directory
    apr_status_t _ret = apr_dir_make_recursive(pPath, APR_OS_DEFAULT, _pool);
    return _ret == APR_SUCCESS ? W_SUCCESS : W_FAILURE;
}

W_RESULT	w_io_utf8_to_ucs2(
	char* in,
	size_t* inbytes,
	uint16_t* out,
	size_t* outwords)
{
	apr_int64_t newch, mask;
	apr_size_t expect, eating;
	int ch;

	while (*inbytes && *outwords)
	{
		ch = (unsigned char)(*in++);
		if (!(ch & 0200)) {
			/* US-ASCII-7 plain text
			 */
			--* inbytes;
			--* outwords;
			*(out++) = ch;
		}
		else
		{
			if ((ch & 0300) != 0300) {
				/* Multibyte Continuation is out of place
				 */
				return APR_BADARG;
			}
			else
			{
				/* Multibyte Sequence Lead Character
				 *
				 * Compute the expected bytes while adjusting
				 * or lead byte and leading zeros mask.
				 */
				mask = 0340;
				expect = 1;
				while ((ch & mask) == mask) {
					mask |= mask >> 1;
					if (++expect > 3) /* (truly 5 for ucs-4) */
						return APR_BADARG;
				}
				newch = ch & ~mask;
				eating = expect + 1;
				if (*inbytes <= expect)
					return APR_BADARG;
				/* Reject values of excessive leading 0 bits
				 * utf-8 _demands_ the shortest possible byte length
				 */
				if (expect == 1) {
					if (!(newch & 0036))
						return APR_BADARG;
				}
				else {
					/* Reject values of excessive leading 0 bits
					 */
					if (!newch && !((unsigned char)* in & 0077 & (mask << 1)))
						return APR_BADARG;
					if (expect == 2) {
						/* Reject values D800-DFFF when not utf16 encoded
						 * (may not be an appropriate restriction for ucs-4)
						 */
						if (newch == 0015 && ((unsigned char)* in & 0040))
							return APR_BADARG;
					}
					else if (expect == 3) {
						/* Short circuit values > 110000
						 */
						if (newch > 4)
							return APR_BADARG;
						if (newch == 4 && ((unsigned char)* in & 0060))
							return APR_BADARG;
					}
				}
				/* Where the boolean (expect > 2) is true, we will need
				 * an extra word for the output.
				 */
				if (*outwords < (apr_size_t)(expect > 2) + 1)
					break; /* buffer full */
				while (expect--)
				{
					/* Multibyte Continuation must be legal */
					if (((ch = (unsigned char) * (in++)) & 0300) != 0200)
						return APR_BADARG;
					newch <<= 6;
					newch |= (ch & 0077);
				}
				*inbytes -= eating;
				/* newch is now a true ucs-4 character
				 *
				 * now we need to fold to ucs-2
				 */
				if (newch < 0x10000)
				{
					--* outwords;
					*(out++) = (apr_uint16_t)newch;
				}
				else
				{
					*outwords -= 2;
					newch -= 0x10000;
					*(out++) = (apr_uint16_t)(0xD800 | (newch >> 10));
					*(out++) = (apr_uint16_t)(0xDC00 | (newch & 0x03FF));
				}
			}
		}
	}
	/* Buffer full 'errors' aren't errors, the client must inspect both
	 * the inbytes and outwords values
	 */
	return W_SUCCESS;
}

W_RESULT w_io_ucs2_to_utf8(
	uint16_t* in,
	size_t* inwords,
	char* out,
	size_t* outbytes)
{
	apr_int64_t newch, require;
	apr_size_t need;
	char* invout;
	int ch;

	while (*inwords && *outbytes)
	{
		ch = (unsigned short)(*in++);
		if (ch < 0x80)
		{
			--* inwords;
			--* outbytes;
			*(out++) = (unsigned char)ch;
		}
		else
		{
			if ((ch & 0xFC00) == 0xDC00) {
				/* Invalid Leading ucs-2 Multiword Continuation Character
				 */
				return APR_BADARG;
			}
			if ((ch & 0xFC00) == 0xD800) {
				/* Leading ucs-2 Multiword Character
				 */
				if (*inwords < 2) {
					/* Missing ucs-2 Multiword Continuation Character
					 */
					return APR_BADARG;
				}
				if (((unsigned short)(*in) & 0xFC00) != 0xDC00) {
					/* Invalid ucs-2 Multiword Continuation Character
					 */
					return APR_BADARG;
				}
				newch = (ch & 0x03FF) << 10 | ((unsigned short)(*in++) & 0x03FF);
				newch += 0x10000;
			}
			else {
				/* ucs-2 Single Word Character
				 */
				newch = ch;
			}
			/* Determine the absolute minimum utf-8 bytes required
			 */
			require = newch >> 11;
			need = 1;
			while (require)
                (void)(require >>= 5), ++need;
			if (need >= *outbytes)
				break; /* Insufficient buffer */
			*inwords -= (need > 2) + 1;
			*outbytes -= need + 1;
			/* Compute the utf-8 characters in last to first order,
			 * calculating the lead character length bits along the way.
			 */
			ch = 0200;
			out += need + 1;
			invout = out;
			while (need--) {
				ch |= ch >> 1;
				*(--invout) = (unsigned char)(0200 | (newch & 0077));
				newch >>= 6;
			}
			/* Compute the lead utf-8 character and move the dest offset
			 */
			*(--invout) = (unsigned char)(ch | newch);
		}
	}
	/* Buffer full 'errors' aren't errors, the client must inspect both
	 * the inwords and outbytes values
	 */
	return W_SUCCESS;
}

long w_io_to_hex(_In_z_ const char* pHexStr)
{
    return strtol(pHexStr, NULL, 16);
}

W_RESULT w_io_has_string_start_with(_In_z_ const char* pString, _In_z_ const char* pStartWith)
{
    return strncmp(pStartWith, pString, strlen(pStartWith));
}

W_RESULT w_io_has_wstring_start_with(_In_z_ const wchar_t* pString, _In_z_ const wchar_t* pStartWith)
{
    return wcsncmp(pStartWith, pString, wcslen(pStartWith));
}

W_RESULT w_io_has_string_end_with(_In_z_ const char* pString, _In_z_ const char* pEndWith)
{
    size_t _str_size = strlen(pString);
    size_t _str_end_size = strlen(pEndWith);
    
    if (_str_size >= _str_end_size)
    {
        return strncmp(pString + _str_size - _str_end_size, pEndWith, _str_end_size) == 0;
    }
    return W_FAILURE;
}

W_RESULT w_io_has_wstring_end_with(_In_z_ const wchar_t* pString, _In_z_ const wchar_t* pEndWith)
{
    size_t _str_size = wcslen(pString);
    size_t _str_end_size = wcslen(pEndWith);
    
    if (_str_size >= _str_end_size)
    {
        return wcsncmp(pString + _str_size - _str_end_size, pEndWith, _str_end_size) == 0;
    }
    return W_FAILURE;
}

W_RESULT w_io_split_string(_In_z_ char* pString,
                           _In_z_ const char* pSplit,
                           _Out_ w_array* pResults)
{
    w_mem_pool _mem_pool = w_get_default_memory_pool();
    if (!_mem_pool)
    {
        W_ASSERT(false, "could not get default memory pool. trace info: w_io_split_string_then_convert_to_chars");
        return W_FAILURE;
    }
    //create array with default size
    if (*pResults)
    {
        w_free(*pResults);
    }
    *pResults = apr_array_make(_mem_pool, 32, sizeof(const char*));
    if (!*pResults)
    {
        W_ASSERT(false, "could not create array. trace info: w_io_split_string_then_convert_to_chars");
        return W_FAILURE;
    }

    char* _splits = strtok(pString, pSplit);
    // loop through the string and extract all other splits
    while( _splits != NULL )
    {
        *(const char**)apr_array_push(*pResults) = _splits;
       _splits = strtok(NULL, pSplit);
    }
    
    return W_SUCCESS;
}

size_t w_io_to_base_64(_Inout_z_ char** pDestinationBuffer,
                       _In_z_ char* pSourceBuffer,
                       _In_z_ const size_t pSourceBufferLenght,
                       _In_ const enum base_64_mode pEncodeMode)
{
    size_t _encoded_size = 0;
    switch (pEncodeMode)
    {
        case chromium:
            _encoded_size = chromium_base64_encode(
                *pDestinationBuffer,
                pSourceBuffer,
                pSourceBufferLenght);
            break;
        case klomp_avx:
            klomp_avx2_base64_encode(
                pSourceBuffer,
                pSourceBufferLenght,
                *pDestinationBuffer,
                &_encoded_size);
            break;
        case fast_avx:
            _encoded_size = fast_avx2_base64_encode(
                *pDestinationBuffer,
                pSourceBuffer,
                pSourceBufferLenght);
            break;
        case fast_avx512:
#if USE_AVX512 != 0 && ((defined(_MSC_VER) && _MSC_VER >= 1911) || (defined(__INTEL_COMPILER) && __INTEL_COMPILER >= 1600) || (defined(__clang__) && __clang_major__ >= 4) || (defined(__GNUC__) && __GNUC__ >= 5))
            _encoded_size = fast_avx512bw_base64_encode(
                pDestinationBuffer,
                pSourceBuffer,
                pSourceBufferLenght);
#endif
            break;
        case quick_time:
            _encoded_size = (size_t)(quicktime_base64_encode(
                *pDestinationBuffer,
                pSourceBuffer,
                (int)pSourceBufferLenght));
            break;
        case scalar:
            scalar_base64_encode(
                pSourceBuffer,
                pSourceBufferLenght,
                *pDestinationBuffer,
                &_encoded_size);
            break;
    }
    return _encoded_size;
}

//apr_dir_read(<#apr_finfo_t *finfo#>, <#apr_int32_t wanted#>, <#apr_dir_t *thedir#>)
//#if defined(__cpp_lib_filesystem) || defined(__cpp_lib_experimental_filesystem)
//    inline void get_files_folders_in_directoryW(_In_z_ const std::wstring & pDirectoryPath, _Inout_ std::vector<std::wstring> & pPaths)
//    {
//        pPaths.clear();
//
//        for (auto& _file_name : fs::directory_iterator(pDirectoryPath))
//        {
//            const auto _path = _file_name.path().wstring();
//            pPaths.push_back(get_file_nameW(_path));
//        }
//    }
//    inline void get_files_folders_in_directory(_In_z_ const std::string& pDirectoryPath, _Inout_ std::vector<std::string>& pPaths)
//    {
//        pPaths.clear();
//
//        for (auto& _file_name : fs::directory_iterator(pDirectoryPath))
//        {
//            const auto _path = _file_name.path().string();
//            pPaths.push_back(get_file_name(_path));
//        }
//    }
//#else
//    inline void get_files_folders_in_directoryW(_In_z_ const std::wstring& pDirectoryPath, _Inout_ std::vector<std::wstring>& pPaths)
//    {
//        pPaths.clear();
//
//        auto _path = wolf::system::convert::wstring_to_string(pDirectoryPath);
//        DIR* _dir;
//        struct dirent* _ent;
//        if ((_dir = opendir(_path.c_str())) != NULL)
//        {
//            while ((_ent = readdir(_dir)) != NULL)
//            {
//                pPaths.push_back(wolf::system::convert::string_to_wstring(_ent->d_name));
//            }
//            closedir(_dir);
//        }
//    }
//
//    inline void get_files_folders_in_directory(_In_z_ const std::string& pDirectoryPath, _Inout_ std::vector<std::string>& pPaths)
//    {
//        pPaths.clear();
//
//        DIR* _dir;
//        struct dirent* _ent;
//        if ((_dir = opendir(pDirectoryPath.c_str())) != NULL)
//        {
//            while ((_ent = readdir(_dir)) != NULL)
//            {
//                pPaths.push_back(_ent->d_name);
//            }
//            closedir(_dir);
//        }
//    }
//#endif
//    /*
//        Read text file from root path
//        pState indicates to state of file and the permission status, the first integer value means :
//             1 means everything is ok
//            -1 means the file could not be found,
//            -2 means file is exist but it might be corrupted
//            -3 means file is empty
//            -4 means could not open file
//    */
//    inline const char* read_text_file(_In_z_ const char* pPath, _Out_ int& pState)
//    {
//        std::ifstream _file(pPath, std::ios::ate | std::ios::in);
//        if (!_file)
//        {
//            //file is not exist
//            pState = -1;
//            return nullptr;
//        }
//        if (_file.bad())
//        {
//            //file exists but it might be corrupted
//            pState = -2;
//            return nullptr;
//        }
//
//        auto _file_length = static_cast<unsigned long>(_file.tellg());
//        if (_file_length == 0)
//        {
//            //file is empty
//            pState = -3;
//            return "";
//        }
//
//        if (!_file.is_open())
//        {
//            //could not open file
//            pState = -4;
//            return nullptr;
//        }
//
//        auto _source = new char[_file_length + 1];
//        _source[_file_length] = '\0';
//        _file.seekg(0, std::ios::beg);
//
//        unsigned int _i = 0;
//        while (_file.good())
//        {
//            _source[_i] = _file.get();
//            if (!_file.eof())
//            {
//                _i++;
//            }
//        }
//        _source[_i] = '\0';
//
//        //close the file
//        _file.close();
//
//        pState = 1;
//        return _source;
//    }
//
//#ifdef __WIN32
//    /*
//        Read text file from root path
//        pState indicates to state of file and the permission status, the first integer value means :
//             1 means everything is ok
//            -1 means the file could not be found,
//            -2 means file is exist but it might be corrupted
//            -3 means file is empty
//            -4 means could not open file
//    */
//    inline const wchar_t* read_text_fileW(_In_z_ const wchar_t* pPath, _Out_ int& pState)
//    {
//        std::ifstream _file(pPath, std::ios::ate | std::ios::in);
//        if (!_file)
//        {
//            //file is not exist
//            pState = -1;
//            return nullptr;
//        }
//
//        if (_file.bad())
//        {
//            //file exists but it might be corrupted
//            pState = -2;
//            return nullptr;
//        }
//
//        auto _file_length = static_cast<unsigned long>(_file.tellg());
//        if (_file_length == 0)
//        {
//            //file is empty
//            pState = -3;
//            return L"";
//        }
//
//        if (!_file.is_open())
//        {
//            //could not open file
//            pState = -4;
//            return nullptr;
//        }
//
//        auto _source = new wchar_t[_file_length + 1];
//        _source[_file_length] = '\0';
//        _file.seekg(0, std::ios::beg);
//
//        unsigned int _i = 0;
//        while (_file.good())
//        {
//            _source[_i] = _file.get();
//            if (!_file.eof())
//            {
//                _i++;
//            }
//        }
//        _source[_i] = '\0';
//
//        //close the file
//        _file.close();
//
//        pState = 1;
//        return _source;
//    }
//
//    /*
//        Read binary file and return array of uint8_t
//        fileState indicates to state of file
//        1 means everything is ok
//        -1 means the file could not be found,
//        -2 means file is exist but could not open
//    */
//    inline void read_binary_fileW(_In_z_ const wchar_t* pPath, _Inout_ std::vector<uint8_t>& pData,
//        _Out_ int& pFileState)
//    {
//        pFileState = 1;
//        std::ifstream _file(pPath, std::ios::binary);
//        if (!_file)
//        {
//            pFileState = -1;
//            return;
//        }
//
//        if (_file.fail())
//        {
//            pFileState = -2;
//            return;
//        }
//
//        // Stop eating new lines in binary mode!!!
//        _file.unsetf(std::ios::skipws);
//
//        // get its size:
//        //std::streampos _file_size;
//        size_t _data_size = 0;
//
//        _file.seekg(0, std::ios::end);
//        _data_size = static_cast<size_t>(_file.tellg());
//        _file.seekg(0, std::ios::beg);
//
//        // reserve capacity
//        pData.reserve(_data_size);
//
//        // read the data:
//        std::copy(std::istream_iterator<uint8_t>(_file),
//            std::istream_iterator<uint8_t>(),
//            std::back_inserter(pData));
//
//    }
//
//#endif
//    /*
//        Read binary file and return array of uint8_t
//        fileState indicates to state of file
//        1 means everything is ok
//        -1 means the file could not be found,
//        -2 means file is exist but could not open
//    */
//    inline void read_binary_file(_In_z_ const char* pPath, _Inout_ std::vector<uint8_t>& pData,
//        _Out_ int& pFileState)
//    {
//        pFileState = 1;
//        std::ifstream _file(pPath, std::ios::binary);
//        if (!_file)
//        {
//            pFileState = -1;
//            return;
//        }
//
//        if (_file.fail())
//        {
//            pFileState = -2;
//            return;
//        }
//
//        // Stop eating new lines in binary mode!!!
//        _file.unsetf(std::ios::skipws);
//
//        // get its size:
//        //std::streampos _file_size;
//        size_t _data_size = 0;
//
//        _file.seekg(0, std::ios::end);
//        _data_size = static_cast<size_t>(_file.tellg());
//        _file.seekg(0, std::ios::beg);
//
//        // reserve capacity
//        pData.reserve(_data_size);
//
//        // read the data:
//        std::copy(std::istream_iterator<uint8_t>(_file),
//            std::istream_iterator<uint8_t>(),
//            std::back_inserter(pData));
//
//    }


//
//#pragma region PNG Methods
//
//    //is stream contains png data
//    inline W_RESULT is_png_file(_Inout_ std::istream& pStream)
//    {
//        return w_image::is_png_file(pStream);
//    }
//
//    //is file contains png data
//    inline W_RESULT is_png_file(_In_z_ const char* pFilePath)
//    {
//        return w_image::is_png_file(pFilePath);
//    }
//
//    /*
//        read png from stream
//        pState indicates the state
//         0 means the succeded
//         1 means file is not png
//        -1 means the file could not be opened for reading
//        -2 means internal function error
//    */
//    inline uint8_t* read_png_from_stream(_Inout_ std::istream& pStream,
//        _Out_ int& pWidth,
//        _Out_ int& pHeight,
//        _Out_ uint8_t& pColorType,
//        _Out_ uint8_t& pBitDepth,
//        _Out_ int& pNumberOfPasses,
//        _Out_ int& pState,
//        _In_ const w_png_pixel_format& pPixelFormat = w_png_pixel_format::RGBA_PNG)
//    {
//        return w_image::read_png_from_stream(
//            pStream,
//            pWidth,
//            pHeight,
//            pColorType,
//            pBitDepth,
//            pNumberOfPasses,
//            pState,
//            pPixelFormat);
//    }
//
//    /*
//        read png from file
//        pState indicates the state
//         0 means the succeded
//         1 means file is not png
//        -1 means the file could not be opened for reading
//        -2 means internal function error
//    */
//    inline uint8_t* read_png_from_file(
//        _In_z_ const char* pFilePath,
//        _Out_ int& pWidth,
//        _Out_ int& pHeight,
//        _Out_ uint8_t& pColorType,
//        _Out_ uint8_t& pBitDepth,
//        _Out_ int& pNumberOfPasses,
//        _Out_ int& pState,
//        _In_ const w_png_pixel_format& pPixelFormat = w_png_pixel_format::RGBA_PNG)
//    {
//        return w_image::read_png_from_file(
//            pFilePath,
//            pWidth,
//            pHeight,
//            pColorType,
//            pBitDepth,
//            pNumberOfPasses,
//            pState,
//            pPixelFormat);
//    }
//
//#pragma endregion
//
//#pragma region JPEG Methods
//
//    /*
//        read jpg from stream
//        pState indicates the state
//         0 means the succeded
//         1 means file is not jpg
//        -1 means the file could not be opened for reading
//        -2 means internal function error
//    */
//    inline uint8_t* read_jpeg_from_stream(
//        _In_z_ std::istream& pStream,
//        _Out_ int& pWidth,
//        _Out_ int& pHeight,
//        _Out_ int& pSubSample,
//        _Out_ int& pColorSpace,
//        _Out_ int& pNumberOfPasses,
//        _Out_ int& pState,
//        _In_ const w_jpeg_pixel_format& pPixelFormat = w_jpeg_pixel_format::RGB_JPEG)
//    {
//        return w_image::read_jpeg_from_stream(
//            pStream,
//            pWidth,
//            pHeight,
//            pSubSample,
//            pColorSpace,
//            pNumberOfPasses,
//            pState,
//            pPixelFormat);
//    }
//
//    /*
//        read jpg from file
//        pState indicates the state
//         0 means the succeded
//         1 means file is not jpg
//        -1 means the file could not be opened for reading
//        -2 means internal function error
//    */
//    inline uint8_t* read_jpeg_from_file(
//        _In_z_ const char* pFilePath,
//        _Out_ int& pWidth,
//        _Out_ int& pHeight,
//        _Out_ int& pSubSample,
//        _Out_ int& pColorSpace,
//        _Out_ int& pNumberOfPasses,
//        _Out_ int& pState,
//        _In_ const w_jpeg_pixel_format& pPixelFormat = w_jpeg_pixel_format::RGB_JPEG)
//    {
//        return w_image::read_jpeg_from_file(
//            pFilePath,
//            pWidth,
//            pHeight,
//            pSubSample,
//            pColorSpace,
//            pNumberOfPasses,
//            pState,
//            pPixelFormat);
//    }
//
//#pragma endregion
//
//}
//
//#endif //__W_IO_H__
