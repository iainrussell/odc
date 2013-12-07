/*
 * (C) Copyright 1996-2012 ECMWF.
 * 
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0. 
 * In applying this licence, ECMWF does not waive the privileges and immunities 
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

///
/// \file Select.cc
///
/// @author Piotr Kuchta, April 2010

#include "eckit/io/FileHandle.h"
#include "odblib/Select.h"

using namespace std;
using namespace eckit;

namespace odb {

Select::Select(const std::string& selectStatement, DataHandle &dh)
: dataHandle_(&dh),
  deleteDataHandle_(false),
  istream_(0),
  deleteIStream_(true),
  selectStatement_(selectStatement)
{}

Select::Select(const std::string& selectStatement, std::istream &is, const std::string& delimiter)
: dataHandle_(0),
  deleteDataHandle_(true),
  istream_(&is),
  deleteIStream_(false),
  selectStatement_(selectStatement),
  delimiter_(delimiter)
{}

Select::Select(const std::string& selectStatement)
: dataHandle_(0),
  deleteDataHandle_(true),
  istream_(0),
  deleteIStream_(true),
  selectStatement_(selectStatement)
{}

Select::Select()
: dataHandle_(0),
  deleteDataHandle_(true),
  istream_(0),
  deleteIStream_(true),
  selectStatement_()
{}

Select::Select(const std::string& selectStatement, const std::string& path)
: dataHandle_(new FileHandle(path)),
  deleteDataHandle_(true),
  // TODO: depending on file extension?
  istream_(0),
  deleteIStream_(true),
  selectStatement_(selectStatement)
{}

Select::~Select()
{
        if (deleteDataHandle_) delete dataHandle_;
        if (deleteIStream_) delete istream_;
}

SelectIterator* Select::createSelectIterator(std::string sql)
{
        return new SelectIterator(*this, sql);
}

const Select::iterator Select::end() { return iterator(0); }

Select::iterator Select::begin()
{
        SelectIterator* it = new SelectIterator(*this, selectStatement_);
        ASSERT(it);
        it->next();
        return iterator(it);
}

} // namespace odb