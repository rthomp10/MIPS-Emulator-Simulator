#include <iostream>
#include <cstdlib>
#include <iomanip>
#include "CacheStats.h"
using namespace std;

CacheStats::CacheStats() {
  cout << "Cache Config: ";
  if(!CACHE_EN) {
    cout << "cache disabled" << endl;
  } else {
    cout << (SETS * WAYS * BLOCKSIZE) << " B (";
    cout << BLOCKSIZE << " bytes/block, " << SETS << " sets, " << WAYS << " roundRobins)" << endl;
    cout << "  Latencies: Lookup = " << LOOKUP_LATENCY << " cycles, ";
    cout << "Read = " << READ_LATENCY << " cycles, ";
    cout << "Write = " << WRITE_LATENCY << " cycles" << endl;
  }

  loads = 0;
  stores = 0;
  load_misses = 0;
  store_misses = 0;
  writebacks = 0;

  byteOffset = 2; 
  wordOffset = 3;
  indexOffset = 3;
  index = -1;
  activeWay = -1;
}

int CacheStats::access(uint32_t addr, ACCESS_TYPE type) {
	if(!CACHE_EN) { // no cache
		return (type == LOAD) ? READ_LATENCY : WRITE_LATENCY;
	}

	index = ( addr >> byteOffset + wordOffset ) && 0x7;
	passedTag = addr >> (byteOffset + wordOffset + indexOffset);

	switch(type){
		case LOAD:
			loads++;
		    break;
		case STORE:
			stores++;
			break;
	}
  
		//checks for a hit
		for( int thisWay = 0; thisWay < 4; thisWay++ ){
			if( valid[index][thisWay] == 1 && tag[index][thisWay] == passedTag ){ 
				if( type = STORE ){
					modified[index][thisWay] = true;
				}
				return LOOKUP_LATENCY;
			}
		}
  
	  //Operations if the cache misses
	  activeWay = roundRobin[index];
	  valid[index][activeWay] = 1;
	  tag[index][activeWay] = passedTag;
	  switch( type ){
		case LOAD:
			load_misses++;
			if( modified[index][activeWay] ){
				writebacks++;
				modified[index][activeWay] = false;
			}
			tag[index][activeWay] = passedTag;
			valid[index][activeWay] = true;
			break;
		case STORE:
			if( modified[index][activeWay] ){
				writebacks++;
				modified[index][activeWay] = false;
			}
			tag[index][activeWay] = passedTag;
			modified[index][activeWay] = true;
			valid[index][activeWay] = true;
			store_misses++;
			break;
		default:
			break;
      }
	  roundRobin[index] = ( roundRobin[index] + 1 ) % WAYS;
	  return WRITE_LATENCY;
}

void CacheStats::printFinalStats() {
  /* TODO: your code here (don't forget to drain the cache of writebacks) */
	

  int accesses = loads + stores;
  int misses = load_misses + store_misses;
  cout << "Accesses: " << accesses << endl;
  cout << "  Loads: " << loads << endl;
  cout << "  Stores: " << stores << endl;
  cout << "Misses: " << misses << endl;
  cout << "  Load misses: " << load_misses << endl;
  cout << "  Store misses: " << store_misses << endl;
  cout << "Writebacks: " << writebacks << endl;
  cout << "Hit Ratio: " << fixed << setprecision(1) << 100.0 * (accesses - misses) / accesses;
  cout << "%" << endl;
}
