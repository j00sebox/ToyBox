#pragma once

#include <string>

class TextureBase
{
public:

	virtual void bind(int slot = 0) const = 0;
	virtual void unbind() const = 0;
};

class Texture2D : public TextureBase
{
public:
	Texture2D(const std::string& file_name);
	~Texture2D();
	
	void bind(int slot = 0) const override;
	void unbind() const override;

	int get_width() const { return m_width; }
	int get_height() const { return m_height; }

private:
	unsigned int m_id;
	int m_width, m_height;
	int m_bpp;
	unsigned char* m_data;
};

class CubeMap : public TextureBase
{
public:
	CubeMap(const std::string& dir);
	~CubeMap();

	void bind(int slot = 0) const override;
	void unbind() const override;

private:
	unsigned int m_id;
	std::string m_faces[6];
};