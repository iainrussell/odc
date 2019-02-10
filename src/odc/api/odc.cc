/*
 * (C) Copyright 2019- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include <functional>
#include <cstdint>

#include "eckit/exception/Exceptions.h"
#include "eckit/maths/Functions.h"
#include "eckit/runtime/Main.h"

#include "odc/api/odc.h"

#include "odc/api/Odc.h"

using namespace odc::api;

extern "C" {

//----------------------------------------------------------------------------------------------------------------------

// DOCUMENTATION comments

// i) By default this API throws exceptions. This will result in hard termination
//    of applications on failure. The routine:
//
//      void odc_error_handling(handling_type);
//
//    changes the behaviour with three options:
//
//      ODC_THROW          - Throws exceptions
//      ODC_ERRORS_CHECKED - Enforces that error values are checked. If an API call
//                           is made and the previous error has not been reset, then
//                           std::terminate will be called.
//      ODC_ERRORS_REPORT  - All exceptions will be caught and odc_errno set
//                           accordingly.

// ii) odc_encode accepts a description of the data structured in memory.

// TODO:
//  - Encoding/decoding integers sensibly (rather than as doubles)
//  - Remove the MARS functionality to MARS
//  - Open memory buffers rather than files
//  - Bitfield descriptions in the API
//  - Sensible behaviour with missing values. The current version is SHIT.
//  - Move odc::ODBAPISettings and odc::MDI somewhere sensible

//----------------------------------------------------------------------------------------------------------------------

struct odb_t {
    odb_t(const char* f) : internal(f) {}
    Odb internal;
};

struct odb_table_t {
    odb_table_t(const Table& t) : internal(t) {}
    const Table internal;
};

//----------------------------------------------------------------------------------------------------------------------

// Types for lookup

const int ODC_NUM_TYPES = NUM_TYPES;

const char* ODC_TYPE_NAMES[] = {
    OdbTypes<ColumnType(0)>::name,
    OdbTypes<ColumnType(1)>::name,
    OdbTypes<ColumnType(2)>::name,
    OdbTypes<ColumnType(3)>::name,
    OdbTypes<ColumnType(4)>::name,
    OdbTypes<ColumnType(5)>::name
};

//----------------------------------------------------------------------------------------------------------------------

/* Error handling */

static std::string g_current_error_str;
static int g_odc_error_behaviour = ODC_THROW;
int odc_errno = 0;

void odc_reset_error() {
    g_current_error_str.clear();
    odc_errno = 0;
}

void odc_error_handling(int handling_type) {
    ASSERT(handling_type >= ODC_THROW && handling_type <= ODC_ERRORS_REPORT);
    g_odc_error_behaviour = handling_type;
    odc_reset_error();
}

const char* odc_error_string() {
    if (g_current_error_str.empty()) {
        return 0;
    } else {
        return g_current_error_str.c_str();
    }
}

static void set_error(const std::string& s, int err) {
    g_current_error_str = s;
    odc_errno = err;
}

} // extern "C"

// Template can't have C linkage

namespace {

template <typename T>
T DefaultConstructed() { return T(); }

template <>
void DefaultConstructed<void>() {}

template <typename FN>
auto wrapApiFunction(FN f) -> decltype(f()) {

    if (g_odc_error_behaviour == ODC_ERRORS_REPORT) {
        odc_reset_error();
    }
    if (g_odc_error_behaviour == ODC_ERRORS_CHECKED && odc_errno != 0) {
        std::stringstream ss;
        ss << "APi call being made after unchecked error: ("
           << odc_errno << "): " << odc_error_string()
           << ". SeriousBug in calling code";
        throw eckit::SeriousBug(ss.str(), Here());
    }

    try {
        return f();
    } catch (eckit::Exception& e) {
        eckit::Log::error() << "Caught exception on C-C++ API boundary: " << e.what() << std::endl;
        if (g_odc_error_behaviour == ODC_THROW) throw;
        set_error(e.what(), 1);
    } catch (std::exception& e) {
        eckit::Log::error() << "Caught exception on C-C++ API boundary: " << e.what() << std::endl;
        if (g_odc_error_behaviour == ODC_THROW) throw;
        set_error(e.what(), 2);
    } catch (...) {
        eckit::Log::error() << "Caught unknown exception on C-C++ API boundary: " << std::endl;
        if (g_odc_error_behaviour == ODC_THROW) throw;
        set_error("Unexpected exception caught", 3);
    }

    // Return default constructed value.
    return DefaultConstructed<decltype(f())>();
}

}

extern "C" {

//----------------------------------------------------------------------------------------------------------------------

// TODO: In a sensible, templated, way catch all exceptions.
//       --> We ought to have a standardised error return process.


/*
 * Initialise API
 * @note This is only required if being used from a context where eckit::Main()
 *       is not otherwise initialised
*/

void odc_initialise_api(int integerBehaviour) {
    return wrapApiFunction([integerBehaviour] {
        static bool initialised = false;

        if (initialised) {
            eckit::Log::warning() << "Initialising ODC library twice" << std::endl;
        }

        if (!initialised) {
            const char* argv[2] = {"odc-api", 0};
            eckit::Main::initialise(1, const_cast<char**>(argv));
            initialised = true;
        }

        if (integerBehaviour != ODC_INTEGERS_AS_DOUBLES && integerBehaviour != ODC_INTEGERS_AS_LONGS) {
            throw eckit::SeriousBug("ODC integer behaviour must be either ODC_INTEGERS_AS_DOUBLES or ODC_INTEGERS_AS_LONGS", Here());
        }
        Settings::treatIntegersAsDoubles(integerBehaviour == ODC_INTEGERS_AS_DOUBLES);
    });
}

/* Basic READ objects */

odb_t* odc_open_for_read(const char* filename) {
    return wrapApiFunction([filename] {
        return new odb_t {filename};
    });
}


void odc_close(odb_t* o) {
    return wrapApiFunction([o]{
        ASSERT(o);
        delete o;
    });
}

/*
 * Table handling
 */

odb_table_t* odc_next_table(odb_t* o) {
    return wrapApiFunction([o] {
        if (Optional<Table> t = o->internal.next()) {
            return new odb_table_t(t.get());
        }
        return static_cast<odb_table_t*>(nullptr);
    });
}

void odc_free_table(odb_table_t* t) {
    return wrapApiFunction([t] {
        ASSERT(t);
        delete(t);
    });
}

int odc_table_num_rows(struct odb_table_t* t) {
    return wrapApiFunction([t] {
        ASSERT(t);
        return t->internal.numRows();
    });
}

int odc_table_num_columns(struct odb_table_t* t) {
    return wrapApiFunction([t] {
        ASSERT(t);
        return t->internal.numColumns();
    });
}

int odc_table_column_type(struct odb_table_t* t, int col) {
    return wrapApiFunction([t, col] {
        ASSERT(t);
        return t->internal.columnType(col);
    });
}

const char* odc_table_column_name(struct odb_table_t* t, int col) {
    return wrapApiFunction([t, col] {
        ASSERT(t);
        return t->internal.columnName(col).c_str();
    });
}

const odb_decoded_t* odc_table_decode_all(const odb_table_t* t) {
    return wrapApiFunction([t] {

        const Table& tbl(t->internal);

        size_t nrows = tbl.numRows();
        size_t ncols = tbl.numColumns();

        // Allocate the object to return

        odb_decoded_t* dt = new odb_decoded_t;

        dt->ncolumns = ncols;
        dt->nrows = nrows;
        dt->columns = 0;
        dt->columnData = new odb_strided_data_t[ncols];

        // Fill in column details

        uintptr_t totalRowSize = 0;
        for (size_t col = 0; col < ncols; ++col) {
            dt->columnData[col].elemSize = tbl.columnDecodedSize(col);
//            eckit::Log::info() << "Setting facade: " << col << " - " << nrows << std::endl;
            dt->columnData[col].nelem = nrows;
            dt->columnData[col].data = reinterpret_cast<char*>(totalRowSize); // Store offset. Update with ptr later.
            totalRowSize += eckit::round(dt->columnData[col].elemSize, sizeof(double));
        }

        // Allocate the storage (and assign the strided data to the columns)

        dt->ownedData = new char[totalRowSize * nrows];
        for (size_t col = 0; col < ncols; ++col) {
            dt->columnData[col].data = dt->ownedData + reinterpret_cast<uintptr_t>(dt->columnData[col].data);
            dt->columnData[col].stride = totalRowSize;
        }

        // And do the actual decoding!

        odc_table_decode(t, dt);

        return dt;
    });
}

void odc_table_decode(const struct odb_table_t* t, struct odb_decoded_t* dt) {
    return wrapApiFunction([t, dt] {

        // Sanity checking

        size_t nrows = t->internal.numRows();
        size_t ncols = t->internal.numColumns();

        ASSERT(dt->ncolumns == long(ncols));
        ASSERT(dt->nrows >= long(nrows));
        ASSERT(dt->columnData);

        // Construct C++ API adapter

        std::vector<StridedData> dataFacade;
        dataFacade.reserve(ncols);

        for (size_t i = 0; i < ncols; i++) {
            auto& col(dt->columnData[i]);
//            eckit::Log::info() << "Facade (" << i << "): " << col.nelem << " -- " << nrows << std::endl;
            ASSERT(col.nelem >= long(nrows));
            dataFacade.emplace_back(col.data, col.nelem, col.elemSize, col.stride);
        }

        DecodeTarget target(dataFacade);

        // Do the decoder

        t->internal.decode(target);

        // And return the values

        dt->nrows = nrows;

        for (size_t i = 0; i < ncols; i++) dt->columnData[i].nelem = nrows;
    });
}

void odc_free_odb_decoded(const odb_decoded_t* dt) {
    return wrapApiFunction([dt] {
        ASSERT(dt);
        ASSERT(dt->columnData);
        if (dt->ownedData) delete [] dt->ownedData;
        delete [] dt->columnData;
        delete dt;
    });
}

//----------------------------------------------------------------------------------------------------------------------

} // extern "C"
