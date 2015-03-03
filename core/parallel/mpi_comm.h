/*
 * mpi_comm.h
 *
 *  created on: 2014-5-12
 *      Author: salmon
 */

#ifndef MPI_COMM_H_
#define MPI_COMM_H_

#include <stddef.h>
#include <cstdbool>
#include <memory>

#include "../gtl/ntuple.h"

extern "C"
{
#include <mpi.h>
}
#include <algorithm>

#include "../dataset/dataset.h"
#include "../gtl/design_pattern/singleton_holder.h"
#include "../utilities/utilities.h"

namespace simpla
{

class MPIComm
{

public:

	MPIComm();

	MPIComm(int argc, char** argv);

	~MPIComm();

	void init(int argc = 0, char** argv = nullptr);

	void close();

	MPI_Comm comm();
	MPI_Info info();

	bool is_valid() const;

	int process_num() const;

	int num_of_process() const;

//	void set_num_of_threads(int num);
//
//	unsigned int get_num_of_threads() const;

	nTuple<int, 3> const & get_topology() const;

	int get_neighbour(int disp_i, int disp_j = 0, int disp_k = 0) const;

	template<typename TI>
	int get_neighbour(TI const &d) const
	{
		return get_neighbour(d[0], d[1], d[2]);
	}

	nTuple<int, 3> get_coordinate(int rank) const;

	void decompose(int ndims, size_t * count, size_t * offset) const;
private:
	struct pimpl_s;
	std::unique_ptr<pimpl_s> pimpl_;
}
;
#define GLOBAL_COMM   SingletonHolder<simpla::MPIComm>::instance()

}
// namespace simpla

#endif /* MPI_COMM_H_ */
