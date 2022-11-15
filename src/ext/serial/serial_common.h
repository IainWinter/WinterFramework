#pragma once

#include "ext/serial/serial.h"

#include "Common.h"
#include "Physics.h"
#include "ext/rendering/Sprite.h"

#include <string>
#include <vector>

namespace meta
{
	template<> void serial_write(serial_writer* serial, const Color& instance);
	template<> void serial_write(serial_writer* serial, const Rigidbody2D& instance);

	template<> void serial_read(serial_reader* serial, Color& instance);
	template<> void serial_read(serial_reader* serial, Rigidbody2D& instance);

	void register_common_types();
}