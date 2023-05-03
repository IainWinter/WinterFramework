#pragma once

// The old rendering attempt was actually very usable and provided some nice control over where memory lived,
// but it fell apart with memory ownership and serialization.

// This set of classes attempts to give full control over the ownership and location of memory. Instead of each class
// being responsible for loading from disk / allocating memory, that has been moved to a platform_host/platform_gl file.
// What makes this approach nice is that the platform code only works with 'views' / 'handles' which are non-owning containers
// for the memory. The HostXXXX and DeviceXXX classes then own these views. This makes is easy to make sure that there are no
// unnessesary copies of data.

//
//    [Host Data]    [Data Layout]      [Device Handle]       [Data Layout]         (data)
//          |          |                       |                   |
//          +----------+                       +---------+---------+
//                     |                       |
//                 [View ]                  [Handle]                                (non-owning)
//                    |                        |
//             [HostXXX class]            [DeviceXXX class]                         (owns view & data)
//			          |                        |		
//                    +------------+-----------+
//                                 |
//                             [XXX class]                                          (owns Host/DeviceXXX)
//