//
// Created by dingrui on 2/23/26.
//

#include "x_application.h"

#include "x_log.h"
#include "events/application_event.h"

void XApplication::Run() {
    WindowResizeEvent e(1280, 720);
    X_TRACE(e.ToString());
    while (true) {
    }
}
