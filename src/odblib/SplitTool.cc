/*
 * (C) Copyright 1996-2012 ECMWF.
 * 
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0. 
 * In applying this licence, ECMWF does not waive the privileges and immunities 
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include "odblib/oda.h"

#include "odblib/Tool.h"
#include "odblib/ToolFactory.h"
#include "Comparator.h"
#include "SplitTool.h"

namespace odb {
namespace tool {

SplitTool::SplitTool (int argc, char *argv[]) : Tool(argc, argv) { }

void SplitTool::run()
{
	if (parameters().size() != 3)
	{
		Log::error() << "Usage: ";
		usage(parameters(0), Log::error());
		Log::error() << endl;
		return;
	}

	PathName inFile = parameters(1);
	PathName outFile = parameters(2);

	odb::Reader in(inFile);
	odb::DispatchingWriter out(outFile);

	odb::DispatchingWriter::iterator outIt = out.begin();
	outIt->pass1(in.begin(), in.end());

	odb::Reader input(inFile);
	odb::Reader::iterator begin(input.begin());
	odb::Reader::iterator end(input.end());
	(**outIt).verify(begin, end);
}

} // namespace tool 
} // namespace odb 

