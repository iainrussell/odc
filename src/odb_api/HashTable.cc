/*
 * (C) Copyright 1996-2012 ECMWF.
 * 
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0. 
 * In applying this licence, ECMWF does not waive the privileges and immunities 
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include "eckit/eckit.h"
#include "eckit/exception/Exceptions.h"
#include "eckit/log/Log.h"
#include "odb_api/HashTable.h"


using namespace eckit;

namespace odb {
namespace codec {

HashTable::HashTable()
: nextIndex_(0),
  strings_(),
  cloned_(false)
{
	//Log::info() << "HashTable@" << this << "::HashTable()" << std::endl;
	for (size_t i = 0; i < sizeof(table) / sizeof(hashrec *); i++)
		table[i] = 0;
}

HashTable::HashTable(const HashTable& other)
: nextIndex_(other.nextIndex_),
  strings_(other.strings_),
  cloned_(true)
{
	for (size_t i = 0; i < sizeof(table) / sizeof(hashrec *); i++)
		table[i] = other.table[i] ? new hashrec(*other.table[i]) : 0;
}

HashTable& HashTable::operator=(const HashTable& other)
{
	cloned_ = true;
	strings_.clear();

	nextIndex_ = other.nextIndex_;
	for (size_t i = 0; i < sizeof(table) / sizeof(hashrec *); i++)
	{
		delete table[i];
		table[i] = other.table[i] ? new hashrec(*other.table[i]) : 0;
	}
	
	strings_ = other.strings_;
	return *this;
}

HashTable* HashTable::clone() {
	HashTable *h = new HashTable;
	*h = *this;
	return h;
}

HashTable::~HashTable()
{
	for (size_t i = 0; i < sizeof(table) / sizeof(hashrec *); i++)
		delete table[i];
}

int64_t msb(int64_t x) { return (*reinterpret_cast<uint64_t*>(&x)) & 0xffffffff; }

int HashTable::hash(const char *name)
{
    ASSERT(name);

	int64_t n (0);

	while (*name)
		n = msb(n + msb(int64_t(*name++ - 'A') + int64_t(msb(n << 5))));

	if (n < 0)
	{
		int64_t m (int64_t( -n ) / SIZE);
		n = msb(n + msb(msb((m + 1)) * SIZE));
	}

	return n % SIZE;
}

void HashTable::store(const char *name)
{
	if (cloned_)
	{
		// TODO: leave the current (last seen by the CharCodec) value ?
		for (size_t i = 0; i < sizeof(table) / sizeof(hashrec *); i++)
		{
			delete table[i];
			table[i] = 0;
		}
		strings_.clear();
		nextIndex_ = 0;
		cloned_ = false;
	}

	int32_t n = hash(name);
	hashrec *h = table[n];

	while(h)
	{
		if(h->name == name)
		{
			h->cnt++;
			return;
		}
		h = h->next;
	}

	h = new hashrec(table[n], name, 1, nextIndex_++);
	table[n] = h;

	//if (nextIndex_ - 1 > strings_.size())

	strings_.push_back(name);
	//ASSERT(strings_.size() == nextIndex_);
	//Log::info() << "HashTable@" << this << "::store(" << name << "): n = " << n << ", nextIndex = " << nextIndex_  << std::endl;
}

int32_t HashTable::findIndex(const char *name)
{
	hashrec *h;
	int32_t n;

	n = hash(name);
	h = table[n];

	while(h)
	{
		if(h->name == name)
			return h->index;
		h = h->next;
	}

	Log::error() << "[" << name << "] not in hash" << std::endl;
	return -1;
}

void HashTable::dumpTable(std::ostream &out) const
{
	out << "HashTable@" << this << "::dumpTable: begin" << std::endl;

	for(int32_t i = 0; i < SIZE; i++)
	{
		hashrec *h  = table[i];

		while(h)
		{
			out << "[" << h->name << "] -> " << h->cnt << " " << h->index << std::endl;
			h = h->next;
		}
	}

	out << "HashTable@" << this << "::dumpTable: end" << std::endl;
}

template<typename BYTEORDER> 
void HashTable::save(DataStream<BYTEORDER> &f)
{
	//Log::info() << "HashTable@" << this << "::save BEGIN" << std::endl;

	int32_t n = 0;

	f.writeInt32(nextIndex_);

	for(int32_t i = 0; i < SIZE; i++)
	{
		hashrec *h  = table[i];

		while(h)
		{
			//Log::info() << "HashTable::save: '" << h->name << "'" << std::endl;

			f.writeString(h->name);
			f.writeInt32(h->cnt);
			f.writeInt32(h->index);
			n++;
			h = h->next;
		}
	}

	ASSERT(n == nextIndex_);

	//Log::info() << "HashTable@" << this << "::save END" << std::endl;
}

template<typename BYTEORDER> 
void HashTable::load(DataStream<BYTEORDER> &f)
{
	//Log::info() << "HashTable@" << this << "::load BEGIN" << std::endl;

	f.readInt32(nextIndex_);

	strings_.resize(nextIndex_);

	for(int32_t i = 0; i < nextIndex_; i++)
	{
		std::string s;
		f.readString(s);

		int32_t cnt;
		f.readInt32(cnt);

		int32_t index;
		f.readInt32(index);

		//Log::info() << "HashTable::load: " << index << " : '" << s << "'" << std::endl;

		ASSERT(index < nextIndex_);
		strings_[index] = std::string(s);

	}
	//Log::info() << "HashTable@" << this << "::load END" << std::endl;
}

template void HashTable::save<SameByteOrder>(DataStream<SameByteOrder>&);
template void HashTable::save<OtherByteOrder>(DataStream<OtherByteOrder>&);

template void HashTable::load<OtherByteOrder>(DataStream<OtherByteOrder>&);
template void HashTable::load<SameByteOrder>(DataStream<SameByteOrder>&);

} // namespace codec
} // namespace odb 
