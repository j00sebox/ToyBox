#pragma once

#include <string>

enum class ImageFormat
{
    JPG = 0,
    PNG
};


class TextureBase
{
public:
    [[nodiscard]] unsigned int get_id() const { return m_id; }
	virtual void bind(unsigned int slot) const = 0;
	virtual void unbind() const = 0;

    void make_resident() const;
    [[nodiscard]] uint64_t get_handle() const;

protected:
    unsigned int m_id;
};

class Texture2D : public TextureBase
{
public:
	Texture2D(const std::string& file_name, bool gamma_correct = true);
	Texture2D(Texture2D&& t) noexcept;
	~Texture2D();
	
	void bind(unsigned int slot = 0) const override;
	void unbind() const override;


	[[nodiscard]] int get_width() const { return m_width; }
	[[nodiscard]] int get_height() const { return m_height; }

	void operator= (Texture2D&& t) noexcept;

private:
	void move_members(Texture2D&& t) noexcept;

	int m_width, m_height;
	int m_colour_channels;
	unsigned char* m_data;
};

class CubeMap : public TextureBase
{
public:
	CubeMap(const std::string& dir, ImageFormat fmt = ImageFormat::JPG);
    CubeMap(int component_type, unsigned int width, unsigned int height);
	CubeMap(CubeMap&& cb) noexcept;
	~CubeMap();

	void bind(unsigned int slot = 0) const override;
	void unbind() const override;

	void operator= (CubeMap&& cb) noexcept;

private:
	std::string m_faces[6];
};