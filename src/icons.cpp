#include <SDL.h>
#include "icons.h"
#include "textureloader.h"

namespace Watcher
{

std::array<GLuint, static_cast<size_t>(IconId::Count)> Icons::m_Icons;

void Icons::Load()
{
	m_Icons[static_cast<size_t>(IconId::Pin)] = TextureLoader::LoadTexture("textures/pin.png");
	m_Icons[static_cast<size_t>(IconId::TaskDisabled)] = TextureLoader::LoadTexture("textures/disabled.png");
	m_Icons[static_cast<size_t>(IconId::TaskError)] = TextureLoader::LoadTexture("textures/error.png");
}

GLuint Icons::GetIcon(IconId id)
{
	SDL_assert(id != IconId::Count);
	return m_Icons[static_cast<size_t>(id)];
}

} // namespace Watcher