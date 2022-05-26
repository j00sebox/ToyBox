#pragma once

#include <string>

class TextureBase
{
public:

	virtual void bind(unsigned int slot = 0) const = 0;
	virtual void unbind() const = 0;
};

class Texture2D : public TextureBase
{
public:
	Texture2D(const std::string& file_name);
	Texture2D(Texture2D&& t) noexcept;
	~Texture2D();
	
	void bind(unsigned int slot = 0) const override;
	void unbind() const override;

	[[nodiscard]] unsigned int get_id() const { return m_id; }
	[[nodiscard]] int get_width() const { return m_width; }
	[[nodiscard]] int get_height() const { return m_height; }

	void operator= (Texture2D&& t) noexcept;

private:
	void move_members(Texture2D&& t) noexcept;

	unsigned int m_id;
	int m_width, m_height;
	int m_colour_channels;
	unsigned char* m_data;
};

class CubeMap : public TextureBase
{
public:
	CubeMap(const std::string& dir);
	CubeMap(CubeMap&& cb) noexcept;
	~CubeMap();

	void bind(unsigned int slot = 0) const override;
	void unbind() const override;

	void operator= (CubeMap&& cb) noexcept;

private:
	unsigned int m_id;
	std::string m_faces[6];
};