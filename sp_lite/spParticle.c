//
// Created by salmon on 16-7-20.
//
#include "sp_lite_def.h"
#include "../src/sp_capi.h"

#include <string.h>
#include <assert.h>

#include "spObject.h"
#include "spMesh.h"
#include "spParticle.h"
#include "spParallel.h"

#include "spRandom.h"

#ifndef SP_MAX_NUMBER_OF_PARTICLE_ATTR
#    define SP_MAX_NUMBER_OF_PARTICLE_ATTR 16
#endif


typedef struct spParticleFiber_s
{
    SP_PARTICLE_HEAD
    byte_type __other[];
} spParticleFiber;

typedef struct spParticleAttrEntity_s
{
    spDataType *data_type;
    size_type offset;
    char name[255];
    void *data;
} spParticleAttrEntity;

/**
 *   Particle (phase space distribution function):
 *     - '''fiber bundle (P)''' on the '''base manifold (M)'''
 *
 *   fiber :
 *     - particles in a same cell(simplex) on '''M'''
 *     - link of '''page'''s
 *   page :
 *     - a group of particle ;
 *     - number of particles in group is a constant SP_NUMBER_OF_ENTITIES_IN_PAGE (128)
 *
 */
struct spParticle_s
{
    SP_MESH_ATTR_HEAD

    Real mass;
    Real charge;

    size_type m_pic_;
    size_type m_max_pic_;
    int m_num_of_attrs_;
    spParticleAttrEntity m_attrs_[SP_MAX_NUMBER_OF_PARTICLE_ATTR];

    size_type m_page_size_;
    size_type m_number_of_pages_;
    size_type *m_page_head_;

    void **m_current_data_;
    void **m_next_data_;

};

int spParticleCreate(spParticle **sp, const spMesh *mesh)
{
    SP_CALL(spMeshAttributeCreate((spMeshAttribute **) sp, sizeof(spParticle), mesh, VOLUME));

    (*sp)->m_max_pic_ = SP_DEFAULT_NUMBER_OF_ENTITIES_IN_PAGE;
    (*sp)->m_pic_ = SP_DEFAULT_NUMBER_OF_ENTITIES_IN_PAGE * 2 / 3;
    (*sp)->m_num_of_attrs_ = 0;
    (*sp)->charge = 1;
    (*sp)->mass = 1;

    return SP_SUCCESS;

}

int spParticleDestroy(spParticle **sp)
{
    if (*sp != NULL)
    {
        for (int i = 0; i < (*sp)->m_num_of_attrs_; ++i)
        {
            spParallelDeviceFree(&((*sp)->m_attrs_[i].data));
            SP_CALL(spDataTypeDestroy(&((*sp)->m_attrs_[i].data_type)));
        }

    }
    SP_CALL(spMeshAttributeDestroy((spMeshAttribute **) sp));


    return SP_SUCCESS;
}

int spParticleAddAttribute(spParticle *sp, char const name[], int tag, size_type size, size_type offset)
{
    SP_CALL(spDataTypeCreate(&(sp->m_attrs_[sp->m_num_of_attrs_].data_type), tag, size));

    sp->m_attrs_[sp->m_num_of_attrs_].offset = offset;
    strcpy(sp->m_attrs_[sp->m_num_of_attrs_].name, name);
    sp->m_attrs_[sp->m_num_of_attrs_].data = NULL;

    ++(sp->m_num_of_attrs_);
}

int spParticleGetNumberOfAttributes(spParticle const *sp) { return sp->m_num_of_attrs_; }

int spParticleGetAttributeName(spParticle *sp, int i, char *name)
{
    strcpy(name, sp->m_attrs_[i].name);
    return SP_SUCCESS;
};

size_type spParticleGetAttributeTypeSizeInByte(spParticle *sp, int i)
{
    return spDataTypeSizeInByte(sp->m_attrs_[i].data_type);
};

void *spParticleGetAttributeData(spParticle *sp, int i) { return sp->m_attrs_[i].data; }

int spParticleDeploy(spParticle *sp)
{
    size_type number_of_entities = spParticleGetNumberOfEntities(sp);

    assert (sp->m_max_pic_ > 0);
    for (int i = 0; i < sp->m_num_of_attrs_; ++i)
    {
        spParallelDeviceAlloc(&(sp->m_attrs_[i].data),
                              spDataTypeSizeInByte(sp->m_attrs_[i].data_type) * number_of_entities);

//        spParallelMemset((sp->m_attrs_[i].data), 0,
//                         spDataTypeSizeInByte(sp->m_attrs_[i].data_type) * number_of_entities);
    }

    return SP_SUCCESS;
}

size_type spParticleGetNumberOfEntities(spParticle const *sp)
{
    return spMeshGetNumberOfEntities(spMeshAttributeGetMesh((spMeshAttribute *) (sp)), SP_DOMAIN_ALL,
                                     spMeshAttributeGetForm((spMeshAttribute *) (sp)))
        * spParticleGetMaxPIC(sp);
}

int spParticleInitialize(spParticle *sp, int const *dist_types)
{

    spMesh const *m = spMeshAttributeGetMesh((spMeshAttribute *) sp);

    int iform = spMeshAttributeGetForm((spMeshAttribute *) sp);

    size_type num_of_pic = spParticleGetPIC(sp);

    size_type max_number_of_entities = spParticleGetNumberOfEntities(sp);

    int num_of_dimensions = spParticleGetNumberOfAttributes(sp);

    int l_dist_types[num_of_dimensions];

    for (int i = 0; i < 6; ++i) { l_dist_types[i] = dist_types == NULL ? SP_RAND_UNIFORM : dist_types[i]; }

    void *data[spParticleGetNumberOfAttributes(sp)];

    SP_CALL(spParticleGetAllAttributeData(sp, data));

    SP_CALL(spParallelMemset(((spParticleFiber *) data)->id, -1, max_number_of_entities * sizeof(int)));

    size_type x_min[3], x_max[3], strides[3];
    SP_CALL(spMeshGetArrayShape(m, SP_DOMAIN_CENTER, x_min, x_max, strides));
    strides[0] *= spParticleGetMaxPIC(sp);
    strides[1] *= spParticleGetMaxPIC(sp);
    strides[2] *= spParticleGetMaxPIC(sp);

    size_type offset = spMeshGetNumberOfEntities(m, SP_DOMAIN_CENTER, iform) * num_of_pic;

    spParallelScan(&offset, 1);

    spRandomGenerator *sp_gen;

    SP_CALL(spRandomGeneratorCreate(&sp_gen, SP_RAND_GEN_SOBOL, 6, offset));

    SP_CALL(spRandomMultiDistributionInCell(sp_gen, l_dist_types,
                                            (Real **) (data + 1),
                                            x_min, x_max, strides, num_of_pic));

    SP_CALL(spRandomGeneratorDestroy(&sp_gen));

}

int spParticleSetPIC(spParticle *sp, size_type pic, size_type max_pic)
{
    sp->m_pic_ = pic;

    if (max_pic == 0)
    {
        sp->m_max_pic_ = 256;
        //(size_type) (pic / SP_DEFAULT_NUMBER_OF_ENTITIES_IN_PAGE + 1) * SP_DEFAULT_NUMBER_OF_ENTITIES_IN_PAGE;
    }
    return SP_SUCCESS;
}

size_type spParticleGetPIC(spParticle const *sp) { return sp->m_pic_; }

size_type spParticleGetMaxPIC(const spParticle *sp) { return sp->m_max_pic_; }

int spParticleGetAllAttributeData(spParticle *sp, void **res)
{
    for (int i = 0, ie = spParticleGetNumberOfAttributes(sp); i < ie; ++i)
    {
        res[i] = spParticleGetAttributeData(sp, i);
    }
    return SP_SUCCESS;
};

int spParticleGetAllAttributeData_device(spParticle *sp, void ***current_data, void ***next_data)
{
//    void *data[spParticleGetNumberOfAttributes(sp)];
//    SP_CALL(spParticleGetAllAttributeData(sp, data));
//    SP_CALL(spParallelDeviceAlloc((void **) current_data, spParticleGetNumberOfAttributes(sp) * sizeof(void *)));
//    SP_CALL(spParallelMemcpy(*current_data, data, spParticleGetNumberOfAttributes(sp) * sizeof(void *)));

    if (current_data != NULL) { *current_data = sp->m_current_data_; }

    if (next_data != NULL) { *next_data = sp->m_next_data_; }
}

spMesh const *spParticleMesh(spParticle const *sp) { return sp->m; };

int spParticleSetMass(spParticle *sp, Real m)
{
    sp->mass = m;
    return SP_SUCCESS;
}

int spParticleSetCharge(spParticle *sp, Real e)
{
    sp->charge = e;
    return SP_SUCCESS;
}

Real spParticleGetMass(spParticle const *sp) { return sp->mass; }

Real spParticleGetCharge(spParticle const *sp) { return sp->charge; }

int spParticleUpdate(spParticle *sp)
{
    void **tmp = sp->m_current_data_;
    sp->m_current_data_ = sp->m_next_data_;
    sp->m_next_data_ = tmp;

    return spParticleSync(sp);

}
/**
 *
 * @param sp
 */
int spParticleSync(spParticle *sp)
{


    spMesh const *m = spMeshAttributeGetMesh((spMeshAttribute const *) sp);
    int iform = spMeshAttributeGetForm((spMeshAttribute const *) sp);
    int ndims = spMeshGetNDims(m);
    int array_ndims, mesh_start_dim;

    size_type l_dims[ndims + 1];
    size_type l_start[ndims + 1];
    size_type l_count[ndims + 1];

    size_type num_of_entities = spParticleGetMaxPIC(sp);

    SP_CALL(spMeshGetGlobalArrayShape(m, SP_DOMAIN_CENTER, 1, &num_of_entities,
                                      &array_ndims, &mesh_start_dim, NULL, NULL, l_dims, l_start, l_count, SP_FALSE));


    for (int i = 0; i < sp->m_num_of_attrs_; ++i)
    {
        SP_CALL(spParallelUpdateNdArrayHalo(sp->m_attrs_[i].data, sp->m_attrs_[i].data_type,
                                            array_ndims, l_dims, l_start, NULL, l_count, NULL, 0));
    }
    return SP_SUCCESS;

}

int
spParticleWrite(spParticle const *sp, spIOStream *os, const char *name, int flag)
{
    if (sp == NULL) { return SP_FAILED; }

    char curr_path[2048];
    char new_path[2048];
    strcpy(new_path, name);
    new_path[strlen(name)] = '/';
    new_path[strlen(name) + 1] = '\0';


    SP_CALL(spIOStreamPWD(os, curr_path));
    SP_CALL(spIOStreamOpen(os, new_path));


    spMesh const *m = spMeshAttributeGetMesh((spMeshAttribute const *) sp);

    int iform = spMeshAttributeGetForm((spMeshAttribute const *) sp);

    int ndims = spMeshGetNDims(m);

    int array_ndims, mesh_start_dim;

    size_type l_dims[ndims + 1];
    size_type l_start[ndims + 1];
    size_type l_count[ndims + 1];

    size_type g_dims[ndims + 1];
    size_type g_start[ndims + 1];

    size_type num_of_entities = spParticleGetMaxPIC(sp);

    spMeshGetGlobalArrayShape(m, SP_DOMAIN_CENTER, 1, &num_of_entities,
                              &array_ndims, &mesh_start_dim, g_dims, g_start, l_dims, l_start, l_count,
                              SP_FALSE);

    num_of_entities *= spMeshGetNumberOfEntities(m, SP_DOMAIN_ALL, iform);

    for (
        int i = 0;
        i < sp->
            m_num_of_attrs_;
        ++i)
    {
        void *buffer = NULL;

        size_type size_in_byte = spDataTypeSizeInByte(sp->m_attrs_[i].data_type) * num_of_entities;

        spParallelHostAlloc(&buffer, size_in_byte);

        spParallelMemcpy(buffer, sp
            ->m_attrs_[i].data, size_in_byte);

        SP_CALL(spIOStreamWriteSimple(os,
                                      sp->m_attrs_[i].name,
                                      sp->m_attrs_[i].data_type,
                                      buffer,
                                      array_ndims,
                                      l_dims,
                                      l_start,
                                      NULL,
                                      l_count,
                                      NULL,
                                      g_dims,
                                      g_start,
                                      flag));

        spParallelHostFree(&buffer);
    }

    SP_CALL(spIOStreamOpen(os, curr_path));
    return SP_SUCCESS;

}

int spParticleRead(struct spParticle_s *f, spIOStream *os, const char *url, int flag)
{
    return SP_SUCCESS;
}

