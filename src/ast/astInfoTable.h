/**********************************************************************/
//! struct table implementation for NvPcomp
/*!
 * \class structTable
 *
 * Description: The struct Table implementation for NvPcomp
 *
 * \author CMT, DRJ & BFB
 *
 */
/**********************************************************************/
#ifndef ASTINFOTABLE_H_
#define ASTINFOTABLE_H_

#include <string>
#include <sstream>
#include <map>
#include <astNode.h>
#include <iterator>

#define ASTINFO_MAXNAMELENGTH	8

template<typename OBJTYPE>
class astInfoTable {
protected:
	std::map< std::string, OBJTYPE *> _table;
protected:
	std::string mangleName(std::string key, int suffix);
public:
	astInfoTable();
	~astInfoTable();
	std::string insert(std::string key, OBJTYPE *obj);
	bool search(const std::string key, OBJTYPE* &obj);
};

////////////////////////////////////////////////////////////////////////
// Constructor/Desctructor
////////////////////////////////////////////////////////////////////////
template<typename OBJTYPE>
astInfoTable<OBJTYPE>::astInfoTable() {}

template<typename OBJTYPE>
astInfoTable<OBJTYPE>::~astInfoTable() {

	// Delete everything.
	typename std::map< std::string, OBJTYPE* >::iterator map_iter;
	// loop through and delete all of the map symNodes.
	for(map_iter = _table.begin(); map_iter != _table.end(); map_iter++) {
		if((*map_iter).second != NULL) {
			delete (*map_iter).second;
		}
	}	
	// Clear the vector
	_table.clear();
}

////////////////////////////////////////////////////////////////////////
// Public functions.
////////////////////////////////////////////////////////////////////////
template<typename OBJTYPE>
std::string astInfoTable<OBJTYPE>::insert(std::string key, OBJTYPE *obj) {
	std::string retVal = "";
	bool unsuccessful = true;
	std::string newKey;
	int suffix = 1;
		
	while(unsuccessful) {

		newKey = mangleName(key, suffix);		
		
		std::pair< typename std::map<std::string, OBJTYPE *>::iterator, bool> ret;
		ret = _table.insert(make_pair(newKey, obj));
		
		// The node was inserted correctly.
		if(ret.second) {
			unsuccessful = false;
			retVal = newKey;
		} else {
			suffix++;
		}
		
	}
		
	return retVal;
	
	
	
}

template<typename OBJTYPE>
bool astInfoTable<OBJTYPE>::search(const std::string key, OBJTYPE* &obj) {
	
	bool retVal = false;
	typename std::map<std::string, OBJTYPE *>::iterator iter;
 
	iter = _table.find(key);
 
	if(iter != _table.end()) {
		obj = (*iter).second;
		retVal = true;
	}
	
	return retVal;	

}

////////////////////////////////////////////////////////////////////////
// Protected functions.
////////////////////////////////////////////////////////////////////////
template<typename OBJTYPE>
std::string astInfoTable<OBJTYPE>::mangleName(std::string key, int suffix) {
	std::string newKey = key;
	std::stringstream suffixStr;
	std::stringstream retVal;
	int suffixSize;
	
	suffixStr << suffix;	
	suffixSize = suffixStr.str().size();
	
	retVal << key.substr(0, ASTINFO_MAXNAMELENGTH - suffixSize -1 ) << suffixStr.str();
	
	return retVal.str();	
}

#endif /* ASTINFOTABLE_H_ */