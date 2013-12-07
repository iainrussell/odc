/*
 * (C) Copyright 1996-2012 ECMWF.
 * 
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0. 
 * In applying this licence, ECMWF does not waive the privileges and immunities 
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// \file UnitTest.cc
///
/// @author ECMWF, July 2010

#include "eckit/utils/Timer.h"
#include "odblib/Select.h"

#include "odblib/Writer.h"
#include "UnitTest.h"

using namespace std;
using namespace eckit;

namespace odb {
namespace tool {
namespace test {



(int argc, char **argv)
: UnitTest(argc, argv)
{}

() { }


void UnitTest::test()
{
	testReaderIterator();
}

void UnitTest::setUp()
{
	Timer t("Test TypeConversion function");
	odb::Writer<> oda("test_type_conversion.odb");

	odb::Writer<>::iterator row = oda.begin();
	row->columns().setSize(1);

	row->setColumn(0, "obsvalue", odb::REAL);
	
	row->writeHeader();

	(*row)[0] = 247.53;

    ++row;
}

void UnitTest::tearDown() 
{ 
	PathName("test_type_conversion.odb").unlink();
}

void UnitTest::testReaderIterator()
{
    const string sql = "select ceil(obsvalue),floor(obsvalue), trunc(obsvalue),int(obsvalue),nint(obsvalue) from \"test_type_conversion.odb\";";

	Log::info() << "Executing: '" << sql << "'" << std::endl;

	odb::Select oda(sql);
	odb::Select::iterator it = oda.begin();

	ASSERT((*it)[0] == 248); // 
	ASSERT((*it)[1] == 247); // 
	ASSERT((*it)[2] == 247); // 
	ASSERT((*it)[3] == 247); // 
	ASSERT((*it)[4] == 248); // 

}

} // namespace test 
} // namespace tool 
} // namespace odb 


