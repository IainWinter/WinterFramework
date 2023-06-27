#include "v2/Render/MeshLayout.h"

int MeshLayout::NumberOfBytes() const
{
    return elementCount * bytesPerElement
         + indexCount * bytesPerIndex;
}
