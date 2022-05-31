#pragma once
#include <glm/glm.hpp>
#include <unordered_map>

namespace OpenGLSandbox {

	struct Character {
		unsigned int TextureID; // ID handle of the glyph texture
		glm::ivec2   Size;      // Size of glyph
		glm::ivec2   Bearing;   // Offset from baseline to left/top of glyph
		unsigned int Advance;   // Horizontal offset to advance to next glyph
	};


	class CharacterSDF {
	public:
		int x0, y0, x1, y1;	// coords of glyph in the texture atlas
		glm::ivec2 m_Size;
		glm::ivec2 m_Bearing;   // left & top bearing when rendering
		int m_Advance;        // x advance when rendering
	};

	class CharacterLibrary
	{
	public:
		CharacterLibrary();
		~CharacterLibrary();

		const CharacterSDF& Get(const unsigned char& name);
		const int GetCount() { return (int)m_Characters.size(); }
		void Add(const unsigned char& name, const CharacterSDF& character);

		bool Exists(const unsigned char& name) const;

		/*void Add(const Ref<Character>& shader);
		Ref<Character> Load(const char& filepath);
		Ref<Character> Load(const char& name, const std::string& filepath);

		Ref<Character> Get(const char& name);

		bool Exists(const char& name) const;*/

	private:
		std::unordered_map<unsigned char, CharacterSDF> m_Characters;
	};
}