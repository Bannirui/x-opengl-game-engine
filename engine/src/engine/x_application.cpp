//
// Created by dingrui on 2/23/26.
//

#include "engine/x_application.h"

#include "engine/x_log.h"
#include "engine/events/application_event.h"

void XApplication::Run() {
    WindowResizeEvent e(1280, 720);
    X_TRACE(e.ToString());
    while (true) {
    }
}
