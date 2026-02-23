//
// Created by dingrui on 2/23/26.
//

#include "x_application.h"

#include "x_log.h"

XApplication::XApplication() {
    XLog::Init();
}

void XApplication::Run() {
    while (true) {
        XLOG_INFO("hello world");
    }
}
