/******************************************************************************
* Copyright (c) 2011, Howard Butler, hobu.inc@gmail.com
*
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following
* conditions are met:
*
*     * Redistributions of source code must retain the above copyright
*       notice, this list of conditions and the following disclaimer.
*     * Redistributions in binary form must reproduce the above copyright
*       notice, this list of conditions and the following disclaimer in
*       the documentation and/or other materials provided
*       with the distribution.
*     * Neither the name of Hobu, Inc. or Flaxen Geo Consulting nor the
*       names of its contributors may be used to endorse or promote
*       products derived from this software without specific prior
*       written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
* "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
* LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
* FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
* COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
* INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
* BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
* OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
* AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
* OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
* OF SUCH DAMAGE.
****************************************************************************/

#pragma once

#include "OciWrapper.h"

#include <memory>

#include <cpl_port.h>

#include <pdal/PointTable.hpp>
#include <pdal/XMLSchema.hpp>

namespace pdal
{

typedef std::shared_ptr<OWConnection> Connection;
typedef std::shared_ptr<OWStatement> Statement;

class connection_failed : public pdal_error
{
public:
    connection_failed(std::string const& msg)
        : pdal_error(msg)
    {}
};

class Block
{
public:
    Block(Connection connection);
    ~Block();

    point_count_t numRemaining() const
        { return m_num_remaining; }
    void setNumRemaining(point_count_t num_remaining)
        { m_num_remaining = num_remaining; }
    point_count_t numRead() const
        { return num_points - m_num_remaining; }
    point_count_t numPoints() const
        { return num_points; }
    char *data() const
        { return reinterpret_cast<char *>(const_cast<uint8_t *>(chunk.data())); }
    void reset()
        { m_num_remaining = num_points; }
    bool fetched() const
        { return m_fetched; }
    void setFetched()
        { m_fetched = true; }
    void clearFetched()
        { m_fetched = false; }

    int32_t obj_id;
    int32_t blk_id;
    sdo_geometry *blk_extent;
    sdo_orgscl_type *blk_domain;
    double pcblk_min_res;
    double pcblk_max_res;
    int32_t num_points;
    int32_t num_unsorted_points;
    int32_t pt_sort_dim;
    std::vector<uint8_t> chunk;
    OCILobLocator *locator;
    Connection m_connection;
    sdo_pc* pc;
    point_count_t m_num_remaining;
    bool m_fetched;  // Set when fetched but not initialized
};
typedef std::shared_ptr<Block> BlockPtr;

PDAL_DLL Connection connect(std::string connSpec);
PDAL_DLL XMLSchema fetchSchema(Statement stmt, BlockPtr block);

} // namespace pdal
