/*
* Miscellaneous little utilities similar what can be found int the stl <algorithm> header.
* Some of them simply make the STL versions easier to use entire containers without specifying iterators
*/

#pragma once

namespace cz
{
	/*
	Same as std::find, but uses the full container range.
	*/
	template<typename C, typename T>
	auto find(C& c, const T& v) -> decltype(c.begin())
	{
		return std::find(c.begin(), c.end(), v);
	}
	
	/*
	Same as std::find_if, but uses the full container range.
	*/
	template<typename C, typename F>
	auto find_if(C& c, const F& f) -> decltype(c.begin())
	{
		return std::find_if(c.begin(), c.end(), f);
	}

	template<typename C, typename T>
	bool exists(C& c, const T& v)
	{
		return find(c, v)!=c.end();
	}
	template<typename C, typename F>
	bool exists_if(C& c, const F& f)
	{
		return std::find_if(c.begin(), c.end(), f)!=c.end();
	}

	/*
	Less verbose way to remove items from a container
	*/
	template<typename C, typename T>
	auto remove(C& c, const T& v) -> decltype(c.begin())
	{
		return c.erase(
			std::remove(c.begin(), c.end(), v),
			c.end());
	}

	/*
	Less verbose way to remove items from a container with a predicate
	*/
	template<typename C, typename F>
	auto remove_if(C& c, const F& f) -> decltype(c.begin())
	{
		return c.erase(
			std::remove_if(c.begin(), c.end(), f),
			c.end());
	}

	/*
	* For use with maps only
	*/
	template< typename Container, typename Predicate>
	void map_remove_if(Container& items, const Predicate& predicate)
	{
		for (auto it = items.begin(); it != items.end();) {
			if (predicate(*it))
				it = items.erase(it);
			else
				++it;
		}
	}

	/*
	Returns a new container with items that fulfill the predicate
	*/
	template<typename C, typename F>
	C copyfrom_if(const C& c, const F& f)
	{
		C res;
		for(const auto& i : c)
			if (f(i))
				res.push_back(i);
		return res;
	}

	template<typename T>
	T clip(const T& n, const T& lower, const T& upper)
	{
		return std::max(lower, std::min(n, upper));
	}

} // namespace cz

