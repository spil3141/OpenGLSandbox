#include "CharacterLibrary.h"

namespace OpenGLSandbox {

	CharacterLibrary::CharacterLibrary()
	{

	}

	CharacterLibrary::~CharacterLibrary()
	{
	}

	void CharacterLibrary::Add(const unsigned char& name, const CharacterSDF& character)
	{
		m_Characters[name] = character;
	}

	const CharacterSDF& CharacterLibrary::Get(const unsigned char& name)
	{
		return m_Characters[name];
	}

	bool CharacterLibrary::Exists(const unsigned char& name) const
	{
		return m_Characters.find(name) != m_Characters.end();
	}
}