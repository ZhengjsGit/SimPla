/*
 * physical_constants.h
 *
 *  Created on: 2012-10-17
 *      Author: salmon
 */

#ifndef PHYSICAL_CONSTANTS_H_
#define PHYSICAL_CONSTANTS_H_
#include "include/simpla_defs.h"
#include "constants.h"
namespace simpla
{

class PhysicalConstants
{
public:
	PhysicalConstants();

	template<typename PT>
	PhysicalConstants(PT const & pt) :
			type(pt.template get<std::string>("<xmlattr>.type"))
	{
		if (type == "CUSTOM")
		{
			m = pt.get("m", 1.0d);
			s = pt.get("s", 1.0d);
			kg = pt.get("kg", 1.0d);
			C = pt.get("C", 1.0f);
			K = pt.get("K", 1.0f);
			mol = pt.get("mol", 1.0d);
		}
		Initialize();
	}

	~PhysicalConstants();

	void Initialize();
	std::string Summary() const;

	inline Real operator[](std::string const &s) const
	{

		std::map<std::string, Real>::const_iterator it = q_.find(s);

		if (it != q_.end())
		{
			return it->second;

		}
		else
		{
			ERROR << "Physical quantity " << s << " is not available!";
		}
		return 0;
	}
private:
	std::map<std::string, Real> q_; //physical quantity
	std::map<std::string, std::string> unitSymbol_;
	std::string type;
	//SI base unit

	Real m; //<< length [meter]
	Real s;	//<< time	[second]
	Real kg; //<< mass	[kilgram]
	Real C;	//<< electric charge	[coulomb]
	Real K;	//<< temperature [kelvin]
	Real mol;	//<< amount of substance [mole]

}
;

}  // namespace simpla

#endif /* PHYSICAL_CONSTANTS_H_ */
