#pragma once

// The old rendering attempt was actually very usable and provided some nice control over where memory lived,
// but it fell apart with memory ownership and serialization.

// This set of classes attempts to give full control over the ownership and location of memory. Instead of each class
// being responsible for loading from disk / allocating memory, that has been moved to a platform_host/platform_gl file.
// What makes this approach nice is that the platform code only works with 'views' / 'handles' which are non-owning containers
// for the memory. The HostXXXX and DeviceXXX classes then own these views. This makes is easy to make sure that there are no
// unnessesary copies of data.

//
//    [Host Data]    [Data Layout]      [Device Handle]       [Data Layout]         (non-owning)
//          |          |                       |                   |
//          +----------+                       +---------+---------+
//                     |                       |
//                 [View ]                  [Handle]                                (non-owning)
//                    |                        |
//             [HostXXX class]            [DeviceXXX class]                         (owns view)
//			          |                        |		
//                    +------------+-----------+
//                                 |
//                          [SharedXXX class]                                       (owns Host/DeviceXXX)


#include "v2/Render/Texture.h"

inline void NewRenderingTest()
{
	// read a texture from memory and copy it into a device only SharedTexture

	TextureLayout layout;
	layout.width = 100;
	layout.height = 100;
	layout.format = fRGBA;

	Texture_New texture = wCreateTexture(layout, aHost);
	TextureView view = texture.instance->View();

	Color t = view.At(0);

	view.Set(0, Color(0, 255, 0));


	//Texture_New fromFile = wCreateTexture(_A("level.png"), aHostDevice);


}
