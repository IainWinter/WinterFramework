#pragma once

#include "ext/serial/serial.h"

#include "Common.h"
#include "Physics.h"

#include "ext/rendering/Sprite.h"
#include "ext/rendering/Camera.h"

#include <string>
#include <vector>

void write_Color(meta::serial_writer* serial, const Color& instance);
void  read_Color(meta::serial_reader* serial, Color& instance);

void write_Rigidbody2D(meta::serial_writer* serial, const Rigidbody2D& instance);
void  read_Rigidbody2D(meta::serial_reader* serial, Rigidbody2D& instance);

void register_common_types();
