#include "common/http_manager.h"
#include "utils/object_util.h"
#include "utils/string_utils.h"
#include "utils/file_utils.h"
#include "basics.h"

#define CONTENT_DISPOSITION_KEY        "Content-disposition:"
#define CONTENT_DISPOSITION_KEY_LENGTH 20
#define FILENAME_KEY                   "filename="
#define FILENAME_KEY_LENGTH            9

// locate filename in the buffer, copy it to the context
static int
RetrieveOutputNameFromContentDisposition(RequestContext* request_context,
                                         char const* content_disposition,
                                         size_t length) {
    content_disposition =
        strncasestr(content_disposition, FILENAME_KEY, length);
    if (!content_disposition) { return RESULT_FAILURE; }
    content_disposition += FILENAME_KEY_LENGTH;
    length -= FILENAME_KEY_LENGTH;

    if (*content_disposition == '"') {
        ++content_disposition;
        --length;
    }

    char output_filename[length];
    int i = 0;
    while (i < length && content_disposition[i] != '\r' &&
           content_disposition[i] != ';' && content_disposition[i] != '\0' &&
           content_disposition[i] != '"') {
        output_filename[i] = content_disposition[i];
        ++i;
    }
    output_filename[i] = '\0';
    request_context->output_filename = strdup(output_filename);
    return RESULT_OK;
}

static int RetrieveOutputNameFromUrl(RequestContext* context) {
    int result = RESULT_OK;
    char output_filename[1024];
    char const* url = context->url;
    url = strrchr(url, '/');
    ++url;
    int i = 0;
    while (*url != '\0' && *url != '?' && *url != '#') {
        output_filename[i++] = *url++;
    }
    output_filename[i] = '\0';
    char* url_decoded_filename =
        curl_easy_unescape(context->curl, output_filename, 0, NULL);
    context->output_filename = strdup(url_decoded_filename);
    curl_free(url_decoded_filename);
    return result;
}

static int CurlProgressFunction(RequestContext* context, double download_total,
                                double download_now, double upload_total,
                                double upload_now) {
    int result = CURLE_OK;
    // pre-flight "get file info" does not have a progress function
    if (download_total > 0 && context->response_code > 0) {
        context->request->progress_callback(context->request->receiver,
                                            download_now, download_total);
        if (!context->is_running) {
            printf("Paused: download: %.2f%%(%.0f/%.0f)\n",
                   download_now * 100 / download_total, download_now,
                   download_total);
            result = CURLE_ABORTED_BY_CALLBACK;
        }
    }
    return result;
}

static size_t CurlHeaderFunction(char* buffer, size_t size, size_t nitems,
                                 RequestContext* request_context) {
    size_t buffer_size = size * nitems;
    printf("%.*s", (int)buffer_size, buffer);
    if (buffer_size >= CONTENT_DISPOSITION_KEY_LENGTH &&
        !strncasecmp(buffer, CONTENT_DISPOSITION_KEY,
                     CONTENT_DISPOSITION_KEY_LENGTH)) {
        int result_code = RetrieveOutputNameFromContentDisposition(
            request_context, buffer + CONTENT_DISPOSITION_KEY_LENGTH,
            buffer_size - CONTENT_DISPOSITION_KEY_LENGTH);
        if (result_code == RESULT_FAILURE) {
            printf("error: bad remote name\n");
        }
    }
    return buffer_size;
}

static size_t CurlBodyFunction(char* buffer, size_t size, size_t nitems,
                               RequestContext* request_context) {
    if (!request_context->output_stream) {
        if (!request_context->output_filename) {
            RetrieveOutputNameFromUrl(request_context);
        }
        if (request_context->output_filename) {
            curl_easy_getinfo(request_context->curl, CURLINFO_RESPONSE_CODE,
                              &request_context->response_code);
            if (request_context->response_code == 206) {
                request_context->output_stream = SmartOpenFileInDirectory(
                    request_context->download_directory,
                    request_context->output_filename, "ab");
            } else {
                request_context->output_stream = SmartOpenFileInDirectory(
                    request_context->download_directory,
                    request_context->output_filename, "wb");
            }
            if (!request_context->output_stream) {
                printf("Failed to open file: %s/%s\n",
                       request_context->download_directory,
                       request_context->output_filename);
                return -1;
            }
        } else {
            return -1;
        }
    }
    if (request_context->output_stream) {
        fwrite(buffer, size, nitems, request_context->output_stream);
    }
    return size * nitems;
}

// called inside io thread
void SendRequest(RequestContext* request_context) {
    if (request_context->is_running) {
        printf("the request has already started");
        return;
    }
    CURL* curl = curl_easy_init();
    if (!curl) {
        request_context->curl_code = -1;
        return;
    }
    curl_easy_setopt(curl, CURLOPT_URL, request_context->url);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
    curl_easy_setopt(curl, CURLOPT_NOBODY, request_context->header_only);

    curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, CurlHeaderFunction);
    curl_easy_setopt(curl, CURLOPT_HEADERDATA, request_context);

    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CurlBodyFunction);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, request_context);

    if (request_context->request->progress_callback) {
        // allow custum progress function
        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
        curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, CurlProgressFunction);
        curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, request_context);
    }

    char content_range[40];
    if (request_context->range_start >= 0 && request_context->range_end >= 0) {
        snprintf(content_range, 40, "%llu-%llu", request_context->range_start,
                 request_context->range_end);
    } else if (request_context->range_start >= 0) {
        snprintf(content_range, 40, "%llu-", request_context->range_start);
    } else if (request_context->range_end >= 0) {
        snprintf(content_range, 40, "-%llu", request_context->range_end);
    } else {
        content_range[0] = 0;
    }
    if (content_range[0] > 0) {
        printf("content range: %s\n", content_range);
        CURLcode code = curl_easy_setopt(curl, CURLOPT_RANGE, content_range);
    }
    curl_easy_setopt(curl, CURLOPT_CAINFO, "cacert.pem");
    curl_easy_setopt(curl, CURLOPT_CAPATH, "cacert.pem");
    // 1. what is this?
    request_context->curl = curl;
    request_context->is_running = 1;
    request_context->curl_code = curl_easy_perform(curl);
    // 2. what is this?
    request_context->curl = NULL;
    request_context->is_running = 0;
    curl_easy_getinfo(curl, CURLINFO_CONTENT_LENGTH_DOWNLOAD_T,
                      &request_context->file_size);
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE,
                      &request_context->response_code);
    if (request_context->header_only && !request_context->output_filename) {
        RetrieveOutputNameFromUrl(request_context);
    }
    if (request_context->output_stream) {
        fclose(request_context->output_stream);
        request_context->output_stream = NULL;
    }
    curl_easy_cleanup(curl);
}

RequestContext* CreateRequestContext(Request* request, char const* url,
                                     char const* directory) {
    CREATE_OBJECT_CLEANED(RequestContext, context);
    context->request = request;
    context->url = strdup(url);
    context->download_directory = strdup(directory);
    context->file_size = 0;
    context->range_start = -1;
    context->range_end = -1;
    context->is_running = 0;
    return context;
}

void DestroyRequestContext(RequestContext* context) {
    if (context) {
        free(context->url);
        free(context->download_directory);
        free(context->output_filename);
        if (context->output_stream) { fclose(context->output_stream); }
        free(context);
    }
}
