/******************************
 * Ryan Thompson
 * CS 3339 - Spring 2018 Section 263
 ******************************/
#include "Stats.h"

Stats::Stats() {
  cycles = PIPESTAGES - 1; // pipeline startup cost
  flushes = 0;
  bubbles = 0;

  memops = 0;
  branches = 0;
  taken = 0;

  //pipeline iitialization
  for(int i = IF1; i < PIPESTAGES; i++) {
    resultReg[i] = -1;
  }
}

void Stats::clock() {
  cycles++;

  // advance all pipeline flops
  for(int i = WB; i > IF1; i--)
	  { resultReg[i] = resultReg[i-1]; }
  resultReg[IF1] = -1;
}

///////////////////////////////////////////////////////////
// Checks for destination registers still being processesed
// in the pipeline identical to the source register
///////////////////////////////////////////////////////////
void Stats::registerSrc(int r) {
    //cycles through stages
    for( int i = EXE1; i < WB; i++ )
	{
		//checks to see if destination stage matches a source stage
       if( resultReg[i] == r)
	   {
		   /*
		   //Pre-bubble pipeline print
		   cout << "Source register == " << r << endl;
	       cout << "Before: ";
		   for(int k = IF1; k < PIPESTAGES; k++) 
				{ cout << resultReg[k] << " "; }
		   cout << endl;
		   */
			
			//injects bubbles
            for( int j = i; j < WB; j++ )
            {
				bubble();
				
				/*
				//prints per bubble injection pipeline
				cout << "        ";
				for(int k = IF1; k < PIPESTAGES; k++) 
					{ cout << resultReg[k] << " "; }
				cout << endl;
				*/
            }
			break; //stops the check loop from looping
			
			/*
			//prints final result
			cout << "After:  ";
			for(int k = IF1; k < PIPESTAGES; k++) 
				{ cout << resultReg[k] << " "; }
			cout << endl << endl;
			*/
        }
    }
}

void Stats::registerDest(int r) {
	resultReg[ID] = r;
}

//Overwrites the the fetch instructions with a noop
void Stats::flush(int count) { // count == how many ops to flush
    for(int i = 0; i < count; i++){ 
		clock();
		flushes++;
	}
}

//Injects a bubbles after the ID stage
void Stats::bubble() {
    cycles++;
    bubbles++;
    // advance all pipeline flops
    for(int i = WB; i > EXE1; i--)
  	  { resultReg[i] = resultReg[i-1]; }
	resultReg[EXE1] = -1;
}
