//
// Created by dingrui on 2/23/26.
//

#pragma once

// For use by engine application

#include "x/core/application.h"
#include "x/core/assert.h"
#include "x/core/base.h"
#include "x/core/input.h"
#include "x/core/input_codes.h"
#include "x/core/layer.h"
#include "x/core/timestep.h"
#include "x/debug/instrumentor.h"
#include "x/events/event.h"
#include "x/imgui/imgui_layer.h"
#include "x/log/log.h"
#include "x/scene/component.h"
#include "x/scene/entity.h"
#include "x/scene/scene.h"
#include "x/scene/scriptable_entity.h"

// ---Renderer---
#include "x/renderer/2d/renderer_2D.h"
#include "x/renderer/2d/renderer_2D_primitives.h"
#include "x/renderer/3d/renderer_3D.h"
#include "x/renderer/buffer/buffer.h"
#include "x/renderer/buffer/frame_buffer.h"
#include "x/renderer/buffer/vertex_array.h"
#include "x/renderer/camera/orthographic_camera.h"
#include "x/renderer/camera/orthographic_camera_controller.h"
#include "x/renderer/render_command.h"
#include "x/renderer/renderer.h"
#include "x/renderer/shader.h"
#include "x/renderer/texture.h"
// ---Renderer---