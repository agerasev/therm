#pragma once

#include "texture.hpp"

namespace gl {
class FrameBuffer : public Texture {
public:
	FrameBuffer();
	virtual ~FrameBuffer();
};
}
