/*
 * properties.h
 *
 *  Created on: 2012-3-6
 *      Author: salmon
 */

#ifndef PROPERTIES_H_
#define PROPERTIES_H_
#include <string>
#include <sstream>


namespace simpla
{
template<typename ... Types>
class Properties
{
public:
	template<typename ...Args>
	Properties(Args& ... args)
	{
	}

	void ParseFile();
	void ParseString();

	template<typename T>
	inline T Get(std::string const & key, T const & default_value = T())
	{
		return default_value;

	}
	template<typename T>
	inline void Set(std::string const & key, T const & default_value)
	{
	}

};


//void read_file(std::string const & fname, Properties & pt);
//
//void write_file(std::string const & fname, Properties const & pt);
//class ptree: public boost::property_tree::ptree
//{
//public:
//	typedef boost::property_tree::ptree BaseType;
//	typedef ptree ThisType;
//	template<typename T> T get_value() const
//	{
//		return BaseType::get_value<T>(
//				pt_trans<T, typename BaseType::data_type>());
//	}
//	template<typename T> T get(std::string const & path) const
//	{
//		return BaseType::get<T>(path,
//				pt_trans<T, typename BaseType::data_type>());
//	}
//
//	template<typename T> T get(std::string const & path, T def) const
//	{
//		return BaseType::get(path, def,
//				pt_trans<T, typename BaseType::data_type>());
//	}
//
//	std::string get(std::string const & path, const char* def) const
//	{
//		return BaseType::get(path, def);
//	}
//	const ptree &get_child(const std::string &path) const
//	{
//		return reinterpret_cast<ptree const &>(BaseType::get_child(path));
//	}
////	boost::optional<const ptree> get_child_optional(
////			const std::string &path) const
////	{
////		boost::optional<const boost::property_tree::ptree&> res =
////				BaseType::get_child_optional(path);
////
////		return reinterpret_cast<ptree const&>(*res);
////	}
//};
}// namespace simpla
#endif /* PROPERTIES_H_ */
