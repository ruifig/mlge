#include "mlge/Math.h"

namespace mlge
{

Color Color::Red    (255  , 0   , 0   , 255);
Color Color::Green  (0    , 255 , 0   , 255);
Color Color::Blue   (0    , 0   , 255 , 255);
Color Color::White  (255  , 255 , 255 , 255);
Color Color::Black  (0    , 0   , 0   , 255);

Color Color::Orange (255 , 128  , 0   , 255);
Color Color::Yellow (255  , 255 , 0   , 255);
Color Color::Cyan   (0    , 255 , 255 , 255);
Color Color::Magenta(0xFF , 0x00, 0xFF, 255);
Color Color::Grey   (0x7F , 0x7F, 0x7F, 255);

Color Color::Pink   (0xFF , 0x00, 0x7F, 255);

Color Color::Teal   (0x48 , 0xAA, 0xAD, 255);

mlge::Rect Point::createRect(int width, int height) const
{
	Rect rect;
	rect.x = x - width / 2;
	rect.w = width;
	rect.y = y - height / 2;
	rect.h = height;
	return rect;
}

} // namespace

