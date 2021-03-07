#pragma once

#include <curl/curl.h>
#include <stdint.h>
#include <tinycthread.h>
#include "common/http_common.h"

typedef struct Request {
    void* receiver;
    void* request_parameter;
    void** request_handler;
    OnRequestError error_callback;
    OnRequestSuccess success_callback;
    OnRequestCancellation cancel_callback;
    OnRequestProgress progress_callback;
} Request;

typedef struct RequestContext {
    Request* request;
    char* url;
    char* download_directory;
    int header_only;
    int64_t range_start;
    int64_t range_end;
    int is_running;
    CURL* curl;
    CURLcode curl_code;
    long response_code;
    char* output_filename;
    int64_t file_size;
    FILE* output_stream;
} RequestContext;

RequestContext* CreateRequestContext(Request* request, char const* url,
                                     char const* directory);
void DestroyRequestContext(RequestContext* request_context);
void SendRequest(RequestContext* request_context);

size_t CurlHeaderFunction(char* buffer, size_t size, size_t nitems,
                          RequestContext* request_context);

size_t CurlBodyFunction(char* buffer, size_t size, size_t nitems,
                        RequestContext* request_context);
