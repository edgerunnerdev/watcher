#include <array>

#pragma once

#include "SDL_opengl.h"

namespace Watcher
{

enum class IconId
{
	Pin,
	TaskDisabled,
	TaskError,

	Count
};

class Icons
{
public:
	static void Load();
	static GLuint GetIcon(IconId id);

private:
	static std::array<GLuint, static_cast<size_t>(IconId::Count)> m_Icons;
};

}