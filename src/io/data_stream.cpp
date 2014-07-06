/*
 * data_stream.cpp
 *
 *  Created on: 2013-12-12
 *      Author: salmon
 */

extern "C"
{
#include <hdf5.h>
#include <hdf5_hl.h>
}

#include "hdf5_datatype.h"
#include "data_stream.h"
#include "../parallel/parallel.h"
#include "../parallel/mpi_datatype.h"

#define H5_ERROR( _FUN_ ) if((_FUN_)<0){ H5Eprint(H5E_DEFAULT, stderr);}

namespace simpla
{

struct DataStream::pimpl_s
{
	hid_t file_;
	hid_t group_;
};
DataStream::DataStream()
		: prefix_("simpla_unnamed"), filename_("unnamed"), grpname_(""),

		suffix_width_(4),

		LIGHT_DATA_LIMIT_(20),

		enable_compact_storable_(false),

		enable_xdmf_(false),

		pimpl_(new pimpl_s( { -1, -1 }))

{
	hid_t error_stack = H5Eget_current_stack();
	H5Eset_auto(error_stack, NULL, NULL);
}

DataStream::~DataStream()
{
	Close();
	delete pimpl_;
}

void DataStream::Init(int argc, char** argv)
{

	ParseCmdLine(argc, argv,

	[&,this](std::string const & opt,std::string const & value)->int
	{
		if(opt=="o"||opt=="output"||opt=="p"||opt=="prefix")
		{
			this->OpenFile(value);
		}
		return CONTINUE;
	}

	);

}
bool DataStream::IsOpened() const
{
	return pimpl_->file_ > 0;
}
void DataStream::OpenGroup(std::string const & gname)
{
	if (gname == "")
		return;

	hid_t h5fg = pimpl_->file_;

	CloseGroup();

	if (gname[0] == '/')
	{
		grpname_ = gname;
	}
	else
	{
		grpname_ += gname;
		if (pimpl_->group_ > 0)
			h5fg = pimpl_->group_;
	}

	if (grpname_[grpname_.size() - 1] != '/')
	{
		grpname_ = grpname_ + "/";
	}

	auto res = H5Lexists(h5fg, grpname_.c_str(), H5P_DEFAULT);

	if (grpname_ == "/" || res != 0)
	{
		H5_ERROR(pimpl_->group_ = H5Gopen(h5fg, grpname_.c_str(), H5P_DEFAULT));
	}
	else
	{
		H5_ERROR(pimpl_->group_ = H5Gcreate(h5fg, grpname_.c_str(), H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT));
	}
	if (pimpl_->group_ <= 0)
	{
		RUNTIME_ERROR("Can not open group " + grpname_ + " in file " + prefix_);
	}

}

void DataStream::OpenFile(std::string const &fname)
{

	CloseFile();

	GLOBAL_COMM.Barrier();

	int name_len;

	if (GLOBAL_COMM.GetRank()==0)
	{
		if (fname != "")
		prefix_ = fname;

		if (fname.size() > 3 && fname.substr(fname.size() - 3) == ".h5")
		{
			prefix_ = fname.substr(0, fname.size() - 3);
		}

		/// @todo auto mkdir directory

		filename_ = prefix_ +

		AutoIncrease(

		[&](std::string const & suffix)->bool
		{
			std::string fname=(prefix_+suffix);
			return
			fname==""
			|| *(fname.rbegin())=='/'
			|| (CheckFileExists(fname + ".h5"));
		}

		) + ".h5";

		name_len=filename_.size();
	}
	if (GLOBAL_COMM.IsInitilized())
	MPI_Bcast(&name_len, 1, MPI_INT, 0, GLOBAL_COMM.GetComm());

	std::vector<char> buffer(name_len);

	if (GLOBAL_COMM.GetRank()==0)
	{
		std::copy(filename_.begin(),filename_.end(),buffer.begin());
	}
	if (GLOBAL_COMM.IsInitilized())
	MPI_Bcast((&buffer[0]), name_len, MPI_CHAR, 0, GLOBAL_COMM.GetComm());

	if (GLOBAL_COMM.GetRank()!=0)
	{
		filename_=&buffer[0];
	}

	if (GLOBAL_COMM.IsInitilized())
	{

		hid_t plist_id = H5Pcreate(H5P_FILE_ACCESS);

		H5Pset_fapl_mpio(plist_id, GLOBAL_COMM.GetComm(), GLOBAL_COMM.GetInfo());

		H5_ERROR(pimpl_->file_ = H5Fcreate(filename_.c_str(), H5F_ACC_EXCL, H5P_DEFAULT, plist_id));

		H5Pclose(plist_id);
	}
	else
	{
		H5_ERROR(pimpl_->file_ = H5Fcreate(filename_.c_str(), H5F_ACC_EXCL, H5P_DEFAULT, H5P_DEFAULT));
	}

	GLOBAL_COMM.Barrier();

	if (pimpl_->file_ < 0)
	{
		RUNTIME_ERROR("Create HDF5 file " + filename_ + " failed!");
	}

	OpenGroup("/");
}

void DataStream::CloseGroup()
{
	if (pimpl_->group_ > 0)
	{
		H5Gclose(pimpl_->group_);
	}
	pimpl_->group_ = -1;
}
void DataStream::CloseFile()
{
	CloseGroup();

	if (pimpl_->file_ > 0)
	{
		H5Fclose(pimpl_->file_);
	}
	pimpl_->file_ = -1;
}

std::string DataStream::WriteHDF5(std::string const & name, void const *v,

size_t t_idx,  unsigned int  type_rank, size_t const * type_dims,

int rank,

size_t const *p_global_start,

size_t const *p_global_count,

size_t const *p_local_outer_start,

size_t const *p_local_outer_count,

size_t const *p_local_inner_start,

size_t const *p_local_inner_count,

bool is_append

) const
{

	auto dsname = name;

	if (v == nullptr)
	{
		WARNING << dsname << " is empty!";
		return "";
	}

	if (pimpl_->group_ <= 0)
	{
		WARNING << "HDF5 file is not opened! No data is saved!";
	}
	hsize_t g_start[rank + type_rank + 1];
	hsize_t g_shape[rank + type_rank + 1];
	hsize_t f_start[rank + type_rank + 1];
	hsize_t m_shape[rank + type_rank + 1];
	hsize_t m_start[rank + type_rank + 1];
	hsize_t m_count[rank + type_rank + 1];

	for (int i = 0; i < rank; ++i)
	{
		g_start[i] = (p_global_start != nullptr) ? p_global_start[i] : 0;
		g_shape[i] = (p_global_count != nullptr) ? p_global_count[i] : 1;
		f_start[i] = (p_local_inner_start != nullptr) ? (p_local_inner_start[i] - g_start[i]) : 0;
		m_shape[i] = (p_local_outer_count != nullptr) ? p_local_outer_count[i] : g_shape[i];
		m_start[i] =
		        (p_local_inner_start != nullptr && p_local_outer_start != nullptr) ?
		                (p_local_inner_start[i] - p_local_outer_start[i]) : 0;
		m_count[i] = (p_local_inner_count != nullptr) ? p_local_inner_count[i] : g_shape[i];
	}

	if (type_rank > 0)
	{
		for (int j = 0; j < type_rank; ++j)
		{

			g_shape[rank + j] = type_dims[j];
			f_start[rank + j] = 0;
			m_shape[rank + j] = type_dims[j];
			m_start[rank + j] = 0;
			m_count[rank + j] = type_dims[j];

			++rank;
		}
	}
	hid_t m_type = GLOBAL_HDF5_DATA_TYPE_FACTORY.Create(t_idx);

	hid_t dset;

	hid_t file_space, mem_space;

	if (!(enable_compact_storable_ || is_append))
	{

		dsname = dsname +

		AutoIncrease([&](std::string const & s )->bool
		{
			return H5Lexists(pimpl_->group_, (dsname + s ).c_str(), H5P_DEFAULT) > 0;
		}, 0, 4);

		file_space = H5Screate_simple(rank, g_shape, nullptr);

		dset = H5Dcreate(pimpl_->group_, dsname.c_str(), m_type, file_space,
		H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);

		H5_ERROR(H5Sclose(file_space));

		H5_ERROR(H5Fflush(pimpl_->group_, H5F_SCOPE_GLOBAL));

		file_space = H5Dget_space(dset);

		H5_ERROR(H5Sselect_hyperslab(file_space, H5S_SELECT_SET, f_start, NULL, m_count, NULL));

		mem_space = H5Screate_simple(rank, m_shape, NULL);

		H5Sselect_hyperslab(mem_space, H5S_SELECT_SET, m_start, NULL, m_count, NULL);

	}
	else
	{
		g_shape[rank] = 1;
		f_start[rank] = 0;
		m_shape[rank] = 1;
		m_start[rank] = 0;
		m_count[rank] = 0;

		if (H5Lexists(pimpl_->group_, dsname.c_str(), H5P_DEFAULT) == 0)
		{
			hsize_t max_dims[rank + 1];

			std::copy(g_shape, g_shape + rank, max_dims);

			max_dims[rank] = H5S_UNLIMITED;

			hid_t space = H5Screate_simple(rank + 1, g_shape, max_dims);

			hid_t dcpl_id = H5Pcreate(H5P_DATASET_CREATE);

			H5_ERROR(H5Pset_chunk(dcpl_id, rank + 1, g_shape));

			dset = H5Dcreate(pimpl_->group_, dsname.c_str(), m_type, space, H5P_DEFAULT, dcpl_id, H5P_DEFAULT);

			H5_ERROR(H5Sclose(space));

			H5_ERROR(H5Pclose(dcpl_id));

		}
		else
		{

			dset = H5Dopen(pimpl_->group_, dsname.c_str(), H5P_DEFAULT);

			file_space = H5Dget_space(dset);

			H5Sget_simple_extent_dims(file_space, g_shape, nullptr);

			H5Sclose(file_space);

			++g_shape[rank];

			H5Dset_extent(dset, g_shape);
		}

//		CHECK(name);
//		CHECK(rank);
//		CHECK(g_shape[0]) << " " << g_shape[1] << " " << g_shape[2] << " " << g_shape[3];
//		CHECK(f_start[0]) << " " << f_start[1] << " " << f_start[2] << " " << f_start[3];
//		CHECK(m_shape[0]) << " " << m_shape[1] << " " << m_shape[2] << " " << m_shape[3];
//		CHECK(m_start[0]) << " " << m_start[1] << " " << m_start[2] << " " << m_start[3];
//		CHECK(m_count[0]) << " " << m_count[1] << " " << m_count[2] << " " << m_count[3];

		file_space = H5Dget_space(dset);

		H5_ERROR(H5Sget_simple_extent_dims(file_space, g_shape, nullptr));

		f_start[rank] = g_shape[rank] - 1;

		m_count[rank] = 1;

		H5_ERROR(H5Sselect_hyperslab(file_space, H5S_SELECT_SET, f_start, nullptr, m_count, nullptr));

		mem_space = H5Screate_simple(rank + 1, m_shape, nullptr);

		H5_ERROR(H5Sselect_hyperslab(mem_space, H5S_SELECT_SET, m_start, NULL, m_count, NULL));

	}

	// Create property list for collective dataset write.
	if (GLOBAL_COMM.IsInitilized())
	{
		hid_t plist_id = H5Pcreate(H5P_DATASET_XFER);
		H5_ERROR(H5Pset_dxpl_mpio(plist_id, H5FD_MPIO_INDEPENDENT));
		H5_ERROR(H5Dwrite(dset,m_type, mem_space, file_space, plist_id, v));
		H5_ERROR(H5Pclose(plist_id));
	}
	else
	{
		H5_ERROR(H5Dwrite(dset,m_type , mem_space, file_space, H5P_DEFAULT, v));
	}

	H5_ERROR(H5Dclose(dset));

	H5_ERROR(H5Sclose(mem_space));

	H5_ERROR(H5Sclose(file_space));

	if (H5Tcommitted(m_type) > 0)
		H5Tclose(m_type);

	return "\"" + GetCurrentPath() + dsname + "\"";
}

void sync_location(hsize_t count[2], MPI_Comm comm)
{

	int size = 1;
	int rank = 0;

	MPI_Comm_size(comm, &size);
	MPI_Comm_rank(comm, &rank);

	if (size <= 1)
	{
		return;
	}

	MPIDataType<hsize_t> m_type;

	std::vector<hsize_t> buffer;

	if (rank == 0)
		buffer.resize(size);

	MPI_Gather(&count[1], 1, m_type.type(), &buffer[0], 1, m_type.type(), 0, comm);

	MPI_Barrier(comm);
	if (rank == 0)
	{
		for (int i = 1; i < size; ++i)
		{
			buffer[i] += buffer[i - 1];
		}
		buffer[0] = count[1];
		count[1] = buffer[size - 1];

		for (int i = size - 1; i > 0; --i)
		{
			buffer[i] = buffer[i - 1];
		}
		buffer[0] = 0;
	}
	MPI_Barrier(comm);
	MPI_Scatter(&buffer[0], 1, m_type.type(), &count[0], 1, m_type.type(), 0, comm);
	MPI_Bcast(&count[1], 1, m_type.type(), 0, comm);

}

std::string DataStream::UnorderedWriteHDF5(std::string const &name, void const *v, size_t t_idx,  unsigned int  type_rank,
        size_t const * type_dims, size_t count) const

{

	auto dsname = name;

	if (v == nullptr)
	{
		WARNING << dsname << " is empty!";
		return "";
	}

	if (pimpl_->group_ <= 0)
	{
		WARNING << "HDF5 file is not opened! No data is saved!";
	}

	hsize_t pos[2] = { 0, count };

	if (GLOBAL_COMM.IsInitilized())
	{
		sync_location(pos, GLOBAL_COMM.GetComm());
	}

	int rank = type_rank + 1;

	hsize_t f_count[rank];
	hsize_t f_start[rank];
	hsize_t m_count[rank];

	f_start[0] = pos[0];
	f_count[0] = pos[1];
	m_count[0] = count;

	if (type_rank > 0)
	{
		for (int j = 0; j < type_rank; ++j)
		{

			f_count[1 + j] = type_dims[j];

			f_start[1 + j] = 0;

			m_count[1 + j] = type_dims[j];

		}
	}

	hid_t m_type = GLOBAL_HDF5_DATA_TYPE_FACTORY.Create(t_idx);

	hid_t dset;

	hid_t file_space, mem_space;

	dsname = dsname +

	AutoIncrease([&](std::string const & s )->bool
	{
		return H5Lexists(pimpl_->group_, (dsname + s ).c_str(), H5P_DEFAULT) > 0;
	}, 0, 4);

	file_space = H5Screate_simple(rank, f_count, nullptr);

	dset = H5Dcreate(pimpl_->group_, dsname.c_str(), m_type, file_space, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);

	H5_ERROR(H5Sclose(file_space));

	H5_ERROR(H5Fflush(pimpl_->group_, H5F_SCOPE_GLOBAL));

	file_space = H5Dget_space(dset);

	H5_ERROR(H5Sselect_hyperslab(file_space, H5S_SELECT_SET, f_start, NULL, m_count, NULL));

	mem_space = H5Screate_simple(rank, m_count, NULL);

	// Create property list for collective dataset write.
	if (GLOBAL_COMM.IsInitilized())
	{
		hid_t plist_id = H5Pcreate(H5P_DATASET_XFER);
		H5_ERROR(H5Pset_dxpl_mpio(plist_id, H5FD_MPIO_INDEPENDENT));
		H5_ERROR(H5Dwrite(dset,m_type, mem_space, file_space, plist_id, v));
		H5_ERROR(H5Pclose(plist_id));
	}
	else
	{
		H5_ERROR(H5Dwrite(dset,m_type , mem_space, file_space, H5P_DEFAULT, v));
	}

	H5_ERROR(H5Dclose(dset));

	H5_ERROR(H5Sclose(mem_space));

	H5_ERROR(H5Sclose(file_space));

	if (H5Tcommitted(m_type) > 0)
		H5Tclose(m_type);

	return "\"" + GetCurrentPath() + dsname + "\"";
}
}
// namespace simpla
