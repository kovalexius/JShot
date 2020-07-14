#include "geometry_types.h"


std::ostream& operator << (std::ostream& _os, const Vector2& _obj)
{
	_os << "m_x: " << _obj.m_x << " m_y: " << _obj.m_y << std::endl;
	return _os;
}

std::ostream& operator << (std::ostream& _os, const CRectangle& _obj)
{
	_os << "m_leftBottomCorner: " << _obj.m_leftBottomCorner << std::endl <<
		"m_size: " << _obj.m_size << std::endl <<
		"m_bytesPerPixel: " << _obj.m_bytesPerPixel << std::endl <<
		"m_bitsPerPixel: " << _obj.m_bitsPerPixel << std::endl;
return _os;
}
