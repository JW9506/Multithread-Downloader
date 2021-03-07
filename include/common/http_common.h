#pragma once
#include <curl/curl.h>
#include "model/task_info.h"

typedef void (*OnRequestError)(void* receiver, CURLcode curl_code,
                               const char* err_msg);
typedef void (*OnRequestSuccess)(void* receiver, void* data);
typedef void (*OnRequestCancellation)();
typedef void (*OnRequestProgress)(void* receiver, uint64_t current, uint64_t total);
