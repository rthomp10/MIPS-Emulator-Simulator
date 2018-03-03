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

  for(int i = IF1; i < PIPESTAGES; i++) {
    resultReg[i] = -1;
  }
}

void Stats::clock() {
  cycles++;

  // advance all pipeline flops
  for(int i = WB; i > IF1; i--) {
    resultReg[i] = resultReg[i-1];
  }
  // inject no-op into IF1
  resultReg[IF1] = -1;
}

///////////////////////////////
// Checks for destination registers still being processesed
// in the pipeline identical to the source register
///////////////////////////////
void Stats::registerSrc(int r) {
    //cycles through stages
    for( int i = EXE1; i < WB; i++ ){
        //checks to see if destination stage matches a source stage
        if( resultReg[i] == r && resultReg[i] != -1){
           /* undo for bubble debug
            cout << "Dependent register == " << r << endl;
            cout << "Before: ";
            for(int i = IF1; i < PIPESTAGES; i++) {
                cout << resultReg[i] << " ";
            }
            cout << endl;
            */
            
            //cycles through the rest of the stages with bubbles
            for( int j = i; j < WB; j++ )
            {
                // advance pipeline flops from the detected equivalence
                for(int j = WB; j > i; j--) {
                    resultReg[j] = resultReg[j-1];
                }
                // inject no-op into the ith index
                resultReg[i] = -1;
                bubble();
                
                
                /*printing
                cout << "        ";
                for(int i = IF1; i < PIPESTAGES; i++) {
                    cout << resultReg[i] << " ";
                }
                cout << endl;
                 */
            }
            i = WB; //stops the check loop from looping
            /*Undo for bubble debug
                        cout << "After:  ";
                        for(int i = IF1; i < PIPESTAGES; i++) {
                            cout << resultReg[i] << " ";
                        }
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
    for(int i = IF1; i < EXE1; i++) {
        resultReg[i] = -1;
    }
    
    flushes += count;
    cycles+= count;
}

//Injects a bubbles after the ID stage
void Stats::bubble() {
    
    bubbles++;
    cycles++;
}
