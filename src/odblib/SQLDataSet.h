/// @file   SQLDataSet.h
/// @author Tomas Kral

#ifndef SQLDATASET_H_
#define SQLDATASET_H_

#include <iosfwd>

#include "odblib/SQLDatabase.h"

namespace odb {
namespace sql {

class SQLDataSet : public SQLDatabase
{
public:
    SQLDataSet();
    SQLDataSet(const std::string& name);
    ~SQLDataSet();

private:
    SQLDataSet(const SQLDataSet&);
    SQLDataSet& operator=(const SQLDataSet&);

    virtual void open();
    virtual void close();

    virtual SQLTable* openDataHandle(DataHandle&, DataFormat = ODA);
    virtual SQLTable* openDataStream(std::istream&, DataFormat = CSV);
};    

} // namespace sql
} // namespace odb

#endif // SQLDATASET_H_