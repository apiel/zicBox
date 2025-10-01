#pragma once

#include "libs/httplib/httplib.h"

void* startHttpServer(void* = NULL)
{
    httplib::Server svr;

    svr.Get("/", [](const httplib::Request&, httplib::Response& res) {
        res.set_content("Hello World!", "text/plain");
    });

    svr.listen("0.0.0.0", 8080);
    return NULL;
}