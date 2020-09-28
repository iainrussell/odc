/*
 * (C) Copyright 1996-2016 ECMWF.
 * 
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0. 
 * In applying this licence, ECMWF does not waive the privileges and immunities 
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */
/// @author Piotr Kuchta, August 2015

#ifndef odc_data_PartFileHandleFactory_H
#define odc_data_PartFileHandleFactory_H

#include "DataHandleFactory.h"

namespace odc {

class PartFileHandleFactory : public DataHandleFactory
{
public:
    PartFileHandleFactory();
protected:
    eckit::DataHandle* makeHandle(const std::string&) const;
};

} // namespace odc

#endif
