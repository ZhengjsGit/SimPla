/*
 * demo_em.cpp
 *
 *  Created on: 2014年11月28日
 *      Author: salmon
 */

#include <memory>
#include <string>
#include "../../../core/utilities/utilities.h"
#include "../../../core/io/io.h"
#include "../../../core/manifold/fetl.h"
#include "../../../core/field/load_field.h"

#include "../../../core/application/use_case.h"

using namespace simpla;

USE_CASE(em)
{

	size_t num_of_steps = 1000;
	size_t strides = 10;
	Real dt = 0.001;

	options.register_cmd_line_option<size_t>("NUMBER_OF_STEPS", "n");

	options.register_cmd_line_option<size_t>("STRIDES", "s");

	options.register_cmd_line_option<Real>("DT", "dt");

	if (options["SHOW HELP"])
	{
		SHOW_OPTIONS("-n,--number_of_steps <NUMBER_OF_STEPS>",
				"number of steps = <NUMBER_OF_STEPS> ,default="
						+ ToString(num_of_steps));
		SHOW_OPTIONS("-s,--strides <STRIDES>",
				" dump record per <STRIDES> steps, default="
						+ ToString(strides));
		SHOW_OPTIONS("-dt  <DT>",
				" value of time step,default =" + ToString(dt));

		return;
	}

	options["NUMBER_OF_STEPS"].as(&num_of_steps);

	options["STRIDES"].as<size_t>(&strides);

	auto manifold = make_manifold<CartesianMesh>();

	manifold->load(options["Mesh"]);

	manifold->update();

	if (options["DT"].as<Real>(&dt))
	{
		manifold->dt(dt);
	}

	// Load initialize value

	auto J = make_form<EDGE, Real>(manifold);
	auto E = make_form<EDGE, Real>(manifold);
	auto B = make_form<FACE, Real>(manifold);

	VERBOSE_CMD(load(options["InitValue"]["B"], &B));
	VERBOSE_CMD(load(options["InitValue"]["E"], &E));
	VERBOSE_CMD(load(options["InitValue"]["J"], &J));

	cd("/Input/");

	VERBOSE << SAVE(E);
	VERBOSE << SAVE(B);
	VERBOSE << SAVE(J);

	STDOUT << std::endl;
	STDOUT << "======== Summary ========" << std::endl;
	RIGHT_COLUMN(" mesh" ) << " = {" << *manifold << "}" << std::endl;
	RIGHT_COLUMN(" time step" ) << " = " << num_of_steps << std::endl;
	RIGHT_COLUMN(" dt" ) << " = " << manifold->dt() << std::endl;
	STDOUT << "=========================" << std::endl;

	cd("/Save/");

	if (!options["JUST A TEST"])
	{
		for (size_t s = 0; s < num_of_steps; s += strides)
		{

			VERBOSE_CMD(load(options["Constraint"]["B"], &B));
			VERBOSE_CMD(load(options["Constraint"]["E"], &E));
			VERBOSE_CMD(load(options["Constraint"]["J"], &J));

			E += curl(B) * dt - J;
			B += -curl(E) * dt;
		}

		VERBOSE << SAVE(E);
		VERBOSE << SAVE(B);

	}

	cd("/Output/");

	VERBOSE << SAVE(E);
	VERBOSE << SAVE(B);
	VERBOSE << SAVE(J);

}

