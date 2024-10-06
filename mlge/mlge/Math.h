#pragma once

namespace mlge
{

namespace details
{
	template<typename T>
	struct MathTraits;

	template<>
	struct MathTraits<float>
	{
		inline static constexpr float Epsilon = 0.001f;
	};

	template<>
	struct MathTraits<double>
	{
		inline static constexpr double Epsilon = 0.00001f;
	};
}

struct Vector
{
	float x;
	float y;
	float z;
};

constexpr inline Vector operator+(const Vector& lhs, const Vector& rhs)
{
	return {lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z};
}

/////////////////////////////////////////////////////////////
//          Point
/////////////////////////////////////////////////////////////

struct Rect;

struct Point : public SDL_Point
{
	Point() = default;

	constexpr Point(int x, int y)
	{
		this->x = x;
		this->y = y;
	}

	/**
	 * Returns a rectangle centered at the point position, and with the specified width and height
	 */
	Rect createRect(int width, int height) const;
};

constexpr bool operator==(const Point& lhs, const Point& rhs)
{
	return lhs.x == rhs.x && lhs.y == rhs.y;
}

constexpr Point operator+(const Point& lhs, const Point& rhs)
{
	return {lhs.x + rhs.x, lhs.y + rhs.y};
}

/////////////////////////////////////////////////////////////
//          FPoint
/////////////////////////////////////////////////////////////

struct FPoint : public SDL_FPoint
{
	FPoint() = default;

	constexpr FPoint(float x, float y)
	{
		this->x = x;
		this->y = y;
	}

	bool isEqual(const FPoint& other, float epsilon = details::MathTraits<float>::Epsilon) const
	{
		return (fabs(x - other.x) < epsilon) && (fabs(y - other.y) < epsilon);
	}

	/**
	 * Converts to integer coordinates, rounding halfway cases away from zero
	 */
	Point toPoint() const
	{
		return {std::lround(x), std::lround(y)};
	}

	FPoint& operator+=(const FPoint& rhs)
	{
		x += rhs.x;
		y += rhs.y;
		return *this;
	}
};

constexpr FPoint operator+(const FPoint& lhs, const FPoint& rhs)
{
	return {lhs.x + rhs.x, lhs.y + rhs.y};
}


struct Size
{
	int w;
	int h;

	constexpr bool operator==(const Size& other) const
	{
		return w==other.w && h==other.h;
	}

	static Size fromFloat(float w, float h)
	{
		return Size(static_cast<int>(w), static_cast<int>(h));
	}
};

struct Rect : public SDL_Rect
{
	constexpr Rect() = default;

	constexpr Rect(int x, int y, int width, int height)
	{
		this->x = x;
		this->y = y;
		this->w = width;
		this->h = height;
	}

	constexpr Rect(const Point& origin, int width, int height)
	{
		this->x = origin.x;
		this->y = origin.y;
		this->w = width;
		this->h = height;
	}

	constexpr Rect(const Point& origin, const Size& size)
	{
		this->x = origin.x;
		this->y = origin.y;
		this->w = size.w;
		this->h = size.h;
	}

	constexpr Size size() const
	{
		return {w, h};
	}

	/**
	 * Returns true if the rectangle has no area
	 */
	bool isEmpty() const
	{
		return w <= 0 && h <= 0;
	}

	constexpr int left() const
	{
		return x;
	}

	constexpr int right() const
	{
		return x + w - 1;
	}

	constexpr int top() const
	{
		return y;
	}

	constexpr int bottom() const
	{
		return y + h - 1;
	}

	/** Top left corner (inclusive) */
	constexpr Point topLeft() const
	{
		return {x, y};
	}

	/** Top right corner (inclusive) */
	constexpr Point topRight() const
	{
		return {right(), top()};
	}

	/** Bottom left corner (inclusive) */
	constexpr Point bottomLeft() const
	{
		return { left(), bottom() };
	}

	/** Bottom right corner (inclusive) */
	constexpr Point bottomRight() const
	{
		return { right(), bottom() };
	}

	constexpr bool contains(int x_, int y_) const
	{
		return ((x_ >= this->x) && (x_ < (this->x + this->w)) && (y_ >= this->y) && (y_ < (this->y + this->h)));
	}

	/** Checks if the specified point is inside the rectangle */
	constexpr bool contains(const Point& pos) const
	{
		return contains(pos.x, pos.y);
	}

	/** Returns true if the specified rectangle is fully contained by "this" */
	constexpr bool contains(const Rect& other) const
	{
		return other.left() >= left() && other.right() <= right() && other.top() >= top() && other.bottom() <= bottom();
	}


	/** Returns true if the two rectangle intersect */
	constexpr bool intersect(const Rect& other, Rect* intersection /*= nullptr*/) const
	{
		// Find overlapping range for x-axis
		int x_overlap = std::max(0, std::min(x + w, other.x + other.w) - std::max(x, other.x));

		// Find overlapping range for y-axis
		int y_overlap = std::max(0, std::min(y + h, other.y + other.h) - std::max(y, other.y));

		if (x_overlap > 0 && y_overlap > 0)
		{
			if (intersection)
			{
				intersection->x = std::max(x, other.x);
				intersection->y = std::max(y, other.y);
				intersection->w = x_overlap;
				intersection->h = y_overlap;
			}
			return true;
		}

		return true;
	}

	/**
	 * Translate the position by the specified delta
	 */
	constexpr void translate(int deltaX, int deltaY)
	{
		this->x += deltaX;
		this->y += deltaY;
	}

	/**
	 * Translate the position by the specified delta
	 */
	constexpr void translate(const Point& deltaPos)
	{
		this->x += deltaPos.x;
		this->y += deltaPos.y;
	}

	constexpr void move(const Point& pos)
	{
		this->x = pos.x;
		this->y = pos.y;
	}

	/**
	 * Expands rectangle by the specified ammount.
	 */
	constexpr void expand(int delta)
	{
		x -= delta;
		y -= delta;
		w += (delta*2);
		h += (delta*2);
	}

	constexpr void contract(int delta)
	{
		expand(-delta);
	}

	#if 0
	/**
	 * Convert to a SDL_Rect
	 */
	constexpr SDL_Rect toSDL() const
	{
		return {x, y, w, h};
	}
	#endif

	constexpr bool operator==(const Rect& other) const
	{
		return
			x == other.x && y == other.y &&
			w == other.w && h == other.h;
	}
};

struct Color : public SDL_Color
{
	Color() = default;

	constexpr explicit Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
	{
		this->r = r;
		this->g = g;
		this->b = b;
		this->a = a;
	}

	constexpr explicit Color(uint8_t r, uint8_t g, uint8_t b)
	{
		this->r = r;
		this->g = g;
		this->b = b;
		this->a = 255;
	}

#if MLGE_EDITOR
	ImVec4 toImGui() const
	{
		return ImColor(r,g,b,a);
	}
#endif


	static Color Red;
	static Color Green;
	static Color DarkGreen;
	static Color Blue;
	static Color White;
	static Color Black;
	static Color Orange;
	static Color Yellow;
	static Color Cyan;
	static Color Magenta;
	static Color Grey;
	static Color Pink;
	static Color Teal;
};



template<typename T>
bool isEqual(T a, T b, T epsilon = details::MathTraits<T>::Epsilon)
{
	return (fabs(a - b) < epsilon);
}

} // namespace

