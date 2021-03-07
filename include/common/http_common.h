#pragma once

typedef void (*OnRequestError)();
typedef void (*OnRequestSuccess)();
typedef void (*OnRequestCancellation)();
typedef void (*OnRequestProgress)(void* receiver, double current, double total);
