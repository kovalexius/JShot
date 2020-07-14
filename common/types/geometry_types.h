#ifndef __GEOMETRY_TYPES__H
#define __GEOMETRY_TYPES__H

#include <sstream>
#include <iostream>

struct Vector2
{
public:
	explicit Vector2(const int _x, const int _y) : m_x(_x),
		m_y(_y)
	{}

	bool operator == (const Vector2& _other)
	{
		return (m_x == _other.m_x) && (m_y == _other.m_y);
	}

	friend std::ostream& operator << (std::ostream& _os, const Vector2& _obj);

	std::ostream& operator << (std::ostream& _os) const
	{
		_os << "m_x: " << m_x << " m_y: " << m_y << std::endl;
		return _os;
	}

	int m_x;
	int m_y;
};


struct CRectangle
{
public:
	CRectangle() : m_leftBottomCorner(0,0),
		m_size(0,0),
		m_bytesPerPixel(4),
		m_bitsPerPixel(m_bytesPerPixel*8)
	{}

	explicit CRectangle(const Vector2& _leftBottomCorner, 
						const Vector2& _size,
						short _bytesPerPixel) :
											m_leftBottomCorner(_leftBottomCorner),
											m_size(_size),
											m_bytesPerPixel(_bytesPerPixel),
											m_bitsPerPixel(_bytesPerPixel*8)
	{}

	bool operator == (const CRectangle& _other)
	{
		return (m_leftBottomCorner == _other.m_leftBottomCorner) && (m_size == _other.m_size);
	}

	bool operator != (const CRectangle& _other)
	{
		return !(operator==(_other));
	}

	Vector2& getSize()
	{
		return m_size;
	}
	const Vector2& getSize() const
	{
		return m_size;
	}

	short getBytesPerPixel() const
	{
		return m_bytesPerPixel;
	}

	void setBytesPerPixel(short _bytesPerPixel)
	{
		m_bytesPerPixel = _bytesPerPixel;
		m_bitsPerPixel = _bytesPerPixel * 8;
	}

	short getBitsPerPixel() const
	{
		return m_bitsPerPixel;
	}

	void setBitsPerPixel(short _bitsPerPixel)
	{
		m_bitsPerPixel = _bitsPerPixel;
		m_bytesPerPixel = _bitsPerPixel / 8;
	}

	Vector2& getLeftBottom()
	{
		return m_leftBottomCorner;
	}

	const Vector2& getLeftBottom() const
	{
		return m_leftBottomCorner;
	}

	friend std::ostream& operator << (std::ostream& _os, const CRectangle& _obj);

	std::ostream& operator << (std::ostream& _os) const
	{
		_os << "m_leftBottomCorner: " << m_leftBottomCorner << std::endl <<
			"m_size: " << m_size << std::endl <<
			"m_bytesPerPixel: " << m_bytesPerPixel << std::endl <<
			"m_bitsPerPixel: " << m_bitsPerPixel << std::endl;

		return _os;
	}

private:
	Vector2 m_leftBottomCorner;
	Vector2 m_size;

	short	m_bytesPerPixel;
	short	m_bitsPerPixel;
};


#endif