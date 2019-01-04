/*
 * (C) Copyright 1996-2012 ECMWF.
 * 
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0. 
 * In applying this licence, ECMWF does not waive the privileges and immunities 
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// \file ODAColumn.h

#ifndef ODAColumn_H
#define ODAColumn_H

#include "eckit/sql/SQLColumn.h"

namespace odc {
namespace sql {


class ODAColumn : public eckit::sql::SQLColumn {
public:
    ODAColumn(const eckit::sql::type::SQLType&, eckit::sql::SQLTable&, const std::string&, int, bool hasMissingValue, double missingValue, const BitfieldDef&, double*);
    ODAColumn(const eckit::sql::type::SQLType&, eckit::sql::SQLTable&, const std::string&, int, bool hasMissingValue, double missingValue, double*);
	~ODAColumn();

    // BUYER BEWARE
    // value(double*) is used to override the double* passed into the constructor.

	void value(double* p) { value_ = p; }
	double * value() const { return value_; }

private:
	ODAColumn(const ODAColumn&);
	ODAColumn& operator=(const ODAColumn&);

	double* value_;
	double  missing_;

	virtual void rewind();
	virtual double next(bool& missing);
	virtual void advance(unsigned long);

// -- Friends
	//friend std::ostream& operator<<(std::ostream& s,const ODAColumn& p)
	//	{ p.print(s); return s; }
};

} // namespace sql 
} // namespace odc 

#endif