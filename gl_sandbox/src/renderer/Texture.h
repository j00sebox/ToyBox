#pragma once

#include <string>

class TextureBase
{
public:
	int get_width() const { return m_width; }
	int get_height() const { return m_height; }

	virtual void bind(int slot = 0) const = 0;
	virtual void unbind() const = 0;

protected:
	unsigned int m_id;
	int m_width, m_height;
};

class Texture2D : public TextureBase
{
public:
	Texture2D(const std::string& file_name);
	~Texture2D();
	
	void bind(int slot = 0) const override;
	void unbind() const override;

private:
	unsigned char* m_data;
	int m_bpp;
};