/*
 * Copyright 2011 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <memory.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// rotors move before the eletrical signal propagates so if it says z it will go through a when you actually press the switch
// to compile:


struct rotor{
  // goes right to left, goes left to right when reflected
  int mapping[26];
  int position; // offsets by distance from a, input forwards and output backwards // this is what moves when stepped
  //ring setting (moves outer alphabet ring and notch) 
  int ringSetting; // offsets alphabet ring backwards and output forward (e.g. 03 (moves 2 because a counts as 1) input a = y output a = c)
  int notch; //denotes when the step of the rotor on the left happens
  // make sure to include double step, happens when notch lines with the slowest rotor
};

struct plugBoard{
  // 0 to 13 different wires
  // mapping is 0 indexed
  int mapping[26]; //placeholder
};

struct reflector{
  // every letter is tied to another one reciprocally meaning they map to each other
  // here I'm just going to do it for each letter so you just return the value at the index
  // e.g. a maps to g and vice verse so if I look up index 'a' it contains 'g' and if I look at index 'g' it contains 'a'
  int mapping[26];
};

int readReflector(struct reflector * reflector, int letterIn){
  if ( 1 <= letterIn && letterIn <= 26) {
    //printf("reflector: %d\n", reflector->mapping[letterIn - 1]);
    return reflector->mapping[letterIn - 1];
  }

  return -1;
}

struct plugBoard * initPlugBoard(int swaps[], int numSwaps){
  struct plugBoard * plugBoard = (struct plugBoard *)malloc(sizeof(struct plugBoard));
  int temp[26] = (int[]){0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25};
  memcpy(plugBoard->mapping, temp, 26 * sizeof(int) );

  // loop through the swaps array 2 at a time
  //printf("numSwaps: %d\n", numSwaps);
  for (int i = 0; i < numSwaps; i++) {
    // have to subtract from the retrieved letter to preserve 0 index
    //printf("swaps[%d], swaps[%d] = %d, %d\n", 2*i, 2*i+1, swaps[2*i], swaps[2*i+1]);
    plugBoard->mapping[swaps[2*i] - 1] = swaps[2*i + 1] - 1;
    plugBoard->mapping[swaps[2*i + 1] - 1] = swaps[2*i] - 1;
    //printf("plugBoard->mapping[%d], plugBoard->mapping[%d] = %d, %d\n", 2*i, 2*i+1, plugBoard->mapping[swaps[2*i] - 1], plugBoard->mapping[swaps[2*i + 1] - 1]);
  }

  return plugBoard;
}

int searchPlugBoard(struct plugBoard * plugBoard, int letterIn) {
  // check between 1-26
  if ( 1 <= letterIn && letterIn <= 26) {
    letterIn -= 1; // 0 indexing
    int outputLetter = plugBoard->mapping[letterIn];
    return outputLetter + 1; //put back to 1 index
  }
  return -1;
}

void stepRotors(struct rotor * l, struct rotor * m, struct rotor * r) {
  // transform with ring setting
  // move from window(position) to notch (+8) e.g. y to q

  // whenever left rotor steps the middle rotor will also step regardless of right motor notch
  bool leftStep = false;
  bool middleStep = false;

  //left
  //printf("r notch to position: %d\n", (((r->notch + (r->ringSetting - 1)) - 8)%26 + 26)%26);
  if (m->position == (((m->notch + (m->ringSetting - 1)) - 8)%26 + 26)%26) {
    leftStep = true;
  }
  //middle
  if (r->position == (((r->notch + (r->ringSetting - 1)) - 8)%26 + 26)%26) {
    middleStep = true;
  }

  // have to figure out if each is gonna step before actually stepping because it happens in parallel
  if (leftStep) {
    middleStep = true; //middle steps when left steps
    l->position = l->position%26 + 1; // keep 1 indexing
  }
  if (middleStep) {
    m->position = m->position%26 + 1; // keep 1 indexing
  }
  // always step right motor
  r->position = r->position%26 + 1; // keep 1 indexing
  //rotor->position = ((rotor->position + 1)%26 + 26)%26 + 1; // 1 indexed

  return;
}

int readRotorFwd( int letterIn, struct rotor * rotor) {
  // subtracting 1 to be able to keep 0 index for array and 1 index for letters
  if (true) {
    //int newLetter = ((letterIn - 1)%26 - 1 + 26)%26 + 1; // -1 for 0 indexing
    int newLetter = (letterIn - 1);
    //printf("newLetter after - 1: %d\n", newLetter);
    // ring position
    newLetter = ((newLetter + (rotor->position - 1))%26 + 26)%26; // %25 not 26 for 0 indexing
    //printf("newLetter after ringpos: %d\n", newLetter);
    // ring setting
    newLetter = ((newLetter - (rotor->ringSetting - 1))%26 + 26)%26;
    //printf("newLetter after ringSetting: %d\n", newLetter);
    // get wiring from mapping, now in 1 indexing
    int mappedChar = rotor->mapping[newLetter];
    //printf("mappedChar: %d\n", mappedChar);
    // now do output moves
    mappedChar = ((mappedChar - rotor->position - 1)%26 + 26)%26 + 1;
    //printf("mappedChar after position: %d\n", mappedChar);
    mappedChar = ((mappedChar + rotor->ringSetting - 1)%26 + 26)%26 + 1;
    //printf("mappedChar after ringSetting: %d\n", mappedChar);
    // if (mappedChar < 0) {
    //   mappedChar += 26;
    // }
    //printf("mappedChar after negative check: %d\n", mappedChar);
    return mappedChar;
    //return rotor->mapping[letterIn - 1]; // testing
  } else
  return -1;
}

int readRotorBwd( int letterIn, struct rotor * rotor) {
  // subtracting 1 to be able to keep 0 index for array and 1 index for letters
  if (true) {
    //int newLetter = ((letterIn)%26 + 26 - 1)%26 + 1; // 1 indexing
    int newLetter = (letterIn);
    //printf("newLetter: %d\n", newLetter);
    // ring setting
    newLetter = ((newLetter - (rotor->ringSetting) - 1)%26 + 26)%26 + 1;
    //printf("newLetter after ringSetting: %d\n", newLetter);
    // ring position
    newLetter = ((newLetter + (rotor->position) - 1)%26 + 26)%26 + 1; // 
    //printf("newLetter after ringpos: %d\n", newLetter);
    // find index instead of lookup
    // maybe error handle this later
    int index = 27;
    for (int i = 0; i < 26; i++) {
      if (rotor->mapping[i] == newLetter) {
        index = i;
        break;
      }
    }
    //printf("index(0 indexed): %d\n", index);
    newLetter = index + 1; // back to 1 indexing
    // ring setting
    newLetter = ((newLetter + (rotor->ringSetting) - 1)%26 + 26)%26 + 1;
    //printf("newLetter after ringSetting: %d\n", newLetter);
    // ring position
    newLetter = ((newLetter - (rotor->position) - 1)%26 + 26)%26 + 1; // %25 not 26 for 0 indexing
    //printf("newLetter after ringpos: %d\n", newLetter);

    return newLetter;

  } else
  return -1;
}

void testComponents() {
  //testing rotor1 mapping
  //EKMFLGDQVZNTOWYHXUSPAIBRCJ
  struct rotor *rotor1 = (struct rotor*)malloc(sizeof(struct rotor));
  memcpy(rotor1->mapping, (int[]){5, 11, 13, 6, 12, 7, 4, 17, 22, 26, 14, 20, 15, 23, 25, 8, 24, 21, 19, 16, 1, 9, 2, 18, 3, 10}, 26 * sizeof(int));
   // for (int i = 0; i < 26; i++){
  //   printf("i = %d, rotor1->mapping[i] = %d\n", i, rotor1->mapping[i]);
  // }
  struct plugBoard * plugBoard = initPlugBoard((int[]){1,4,2,5}, 2); //  int numSwaps = (sizeof(swaps) / sizeof(swaps[0])) / 2; // /2 to get the number of actual iterations

  struct reflector * reflector1 = (struct reflector*)malloc(sizeof(struct reflector)); // YRUHQSLDPXNGOKMIEBFZCWVJAT
  memcpy(reflector1->mapping, (int[]){25, 18, 21, 8, 17, 19, 12, 4, 16, 24, 14, 7, 15, 11, 13, 9, 5, 2, 6, 26, 3, 23, 22, 10, 1, 20}, 26 * sizeof(int));

  //example 1
  rotor1->position = 1; //a
  rotor1-> ringSetting = 1;//a
  int testReadRotor1Fwd = readRotorFwd(1, rotor1);
  printf("testReadRotor1Fwd 1: %d\n\n", testReadRotor1Fwd);
  //example 2
  rotor1->position = 2; //b
  rotor1-> ringSetting = 1;//a
  testReadRotor1Fwd = readRotorFwd(1, rotor1);
  printf("testReadRotor1Fwd 2: %d\n\n", testReadRotor1Fwd);
  //example 3
  rotor1->position = 1; //a
  rotor1-> ringSetting = 2;//b
  testReadRotor1Fwd = readRotorFwd(1, rotor1);
  printf("testReadRotor1Fwd 3: %d\n\n", testReadRotor1Fwd);
  //example 4
  rotor1->position = 25; //y
  rotor1-> ringSetting = 6;//f
  testReadRotor1Fwd = readRotorFwd(1, rotor1);
  printf("testReadRotor1Fwd 4: %d\n\n\n", testReadRotor1Fwd);
  // tests should be e, j, k, w
  
  // now backwards tests
   //example 1
  rotor1->position = 1; //a
  rotor1-> ringSetting = 1;//a
  int testReadRotor1Bwd = readRotorBwd(5, rotor1);
  printf("testReadRotor1Bwd 1: %d\n\n", testReadRotor1Bwd);
  //example 2
  rotor1->position = 2; //b
  rotor1-> ringSetting = 1;//a
  testReadRotor1Bwd = readRotorBwd(10, rotor1);
  printf("testReadRotor1Bwd 2: %d\n\n", testReadRotor1Bwd);
  //example 3
  rotor1->position = 1; //a
  rotor1-> ringSetting = 2;//b
  testReadRotor1Bwd = readRotorBwd(11, rotor1);
  printf("testReadRotor1Bwd 3: %d\n\n", testReadRotor1Bwd);
  //example 4
  rotor1->position = 25; //y
  rotor1-> ringSetting = 6;//f
  testReadRotor1Bwd = readRotorBwd(23, rotor1);
  printf("testReadRotor1Bwd 4: %d\n\n\n", testReadRotor1Bwd);
  // tests should be a, a, a, a

  // test plugboard
  for (int i = 0; i < 26; i++){
    printf("i = %d, plugBoard->mapping[i] = %d\n", i, plugBoard->mapping[i]);
  }
  int testPlugBoard;
  testPlugBoard = searchPlugBoard(plugBoard, 1);
  printf("testPlugBoard 1: %d\n\n", testPlugBoard);
  testPlugBoard = searchPlugBoard(plugBoard, 2);
  printf("testPlugBoard 2: %d\n\n", testPlugBoard);
  testPlugBoard = searchPlugBoard(plugBoard, 4);
  printf("testPlugBoard 3: %d\n\n\n", testPlugBoard);
  // should be 4, 2, 1 because a and d are swapped

  // test reflector
  int testReflector1;
  testReflector1 = readReflector(reflector1, 1); 
  printf("testReflector1 1: %d\n\n", testReflector1);
  testReflector1 = readReflector(reflector1, 25);
  printf("testReflector1 2: %d\n\n", testReflector1);
  testReflector1 = readReflector(reflector1, 2);
  printf("testReflector1 3: %d\n\n", testReflector1);
  testReflector1 = readReflector(reflector1, 18);
  printf("testReflector1 4: %d\n\n", testReflector1);
  testReflector1 = readReflector(reflector1, 26);
  printf("testReflector1 5: %d\n\n", testReflector1);
  testReflector1 = readReflector(reflector1, 20);
  printf("testReflector1 6: %d\n\n\n", testReflector1);
  // should be 25, 1, 18, 2, 20, 26 -- I picked pairs

  // free everything
  free(rotor1);
  free(plugBoard);
  free(reflector1);
}
void testSimpleEncription(){
    //EKMFLGDQVZNTOWYHXUSPAIBRCJ
  struct rotor *rotor1 = (struct rotor*)malloc(sizeof(struct rotor));
  memcpy(rotor1->mapping, (int[]){5, 11, 13, 6, 12, 7, 4, 17, 22, 26, 14, 20, 15, 23, 25, 8, 24, 21, 19, 16, 1, 9, 2, 18, 3, 10}, 26 * sizeof(int));
  rotor1->position = 1;
  rotor1->ringSetting = 1;
  rotor1->notch = 25; //25
  struct rotor *rotor2 = (struct rotor*)malloc(sizeof(struct rotor));
  memcpy(rotor2->mapping, (int[]){1, 10, 4, 11, 19, 9, 18, 21, 24, 2, 12, 8, 23, 20, 13, 3, 17, 7, 26, 14, 16, 25, 6, 22, 15, 5}, 26 * sizeof(int));
  rotor2->position = 1;
  rotor2->ringSetting = 1;
  rotor2->notch = 13; //13
  struct rotor *rotor3 = (struct rotor*)malloc(sizeof(struct rotor));
  memcpy(rotor3->mapping, (int[]){2, 4, 6, 8, 10, 12, 3, 16, 18, 20, 24, 22, 26, 14, 25, 5, 9, 23, 7, 1, 11, 13, 21, 19, 17, 15}, 26 * sizeof(int));
  rotor3->position = 26;
  rotor3->ringSetting = 1;
  rotor3->notch = 4; //4

  struct plugBoard * plugBoard = initPlugBoard((int[]){26,20}, 1); //  int numSwaps = (sizeof(swaps) / sizeof(swaps[0])) / 2; // /2 to get the number of actual iterations

  struct reflector * reflector1 = (struct reflector*)malloc(sizeof(struct reflector)); // YRUHQSLDPXNGOKMIEBFZCWVJAT
  memcpy(reflector1->mapping, (int[]){25, 18, 21, 8, 17, 19, 12, 4, 16, 24, 14, 7, 15, 11, 13, 9, 5, 2, 6, 26, 3, 23, 22, 10, 1, 20}, 26 * sizeof(int));

  // rotors 1, 2, 3 from left to right, ring settings 1,1,1 pos 1,1,1 (technically 1,1,26 before stepping), plugboard swap z and t, reflector B
  // expected z -> f(6)
  //printf("rotors positions before loop: %d, %d, %d\n", rotor1->position, rotor2->position, rotor3->position);
  printf("begin message: \n\n\t");
  for(int i = 0; i < 30; i++) {
    stepRotors(rotor1, rotor2, rotor3);
    //printf("rotors positions: %d, %d, %d\n", rotor1->position, rotor2->position, rotor3->position);
    int letter = searchPlugBoard(plugBoard, 26);

    letter = readRotorFwd(letter, rotor3);
    letter = readRotorFwd(letter, rotor2);
    letter = readRotorFwd(letter, rotor1);
    
    letter = readReflector(reflector1, letter);

    letter = readRotorBwd(letter, rotor1);
    letter = readRotorBwd(letter, rotor2);
    letter = readRotorBwd(letter, rotor3);

    letter = searchPlugBoard(plugBoard, letter);

    printf("%c", letter+64);// turning to ascii
    if((i + 1)%5 == 0){
      printf(" ");
    }
  }

  printf("\n\nfinished decryption\n\n");
  // should print FOTHF XRHRH AMENA UWEIG SUXEW KKGGP
  //              FOTHF XRHRH AMENA UWEIG SUXEW KKGGP
  free(rotor1);
  free(rotor2);
  free(rotor3);
  free(plugBoard);
  free(reflector1);
  return;
}

int main() {
  printf("\n\nenigma emulator\n\n\n");
  // using a = 1 z = 26

  //testComponents();
  testSimpleEncription();

  // defining each rotor and reflector and the plugboard, setting defaults to 1 except notch which is baked in
  struct rotor *rotor1 = (struct rotor*)malloc(sizeof(struct rotor));
  memcpy(rotor1->mapping, (int[]){5, 11, 13, 6, 12, 7, 4, 17, 22, 26, 14, 20, 15, 23, 25, 8, 24, 21, 19, 16, 1, 9, 2, 18, 3, 10}, 26 * sizeof(int));
  rotor1->position = 1;
  rotor1->ringSetting = 1;
  rotor1->notch = 25;
  struct rotor *rotor2 = (struct rotor*)malloc(sizeof(struct rotor));
  memcpy(rotor2->mapping, (int[]){1, 10, 4, 11, 19, 9, 18, 21, 24, 2, 12, 8, 23, 20, 13, 3, 17, 7, 26, 14, 16, 25, 6, 22, 15, 5}, 26 * sizeof(int));
  rotor2->position = 1;
  rotor2->ringSetting = 1;
  rotor2->notch = 13;
  struct rotor *rotor3 = (struct rotor*)malloc(sizeof(struct rotor));
  memcpy(rotor3->mapping, (int[]){2, 4, 6, 8, 10, 12, 3, 16, 18, 20, 24, 22, 26, 14, 25, 5, 9, 23, 7, 1, 11, 13, 21, 19, 17, 15}, 26 * sizeof(int));
  rotor3->position = 1;
  rotor3->ringSetting = 1;
  rotor3->notch = 4;

    // reflector
  struct reflector * reflector1 = (struct reflector*)malloc(sizeof(struct reflector)); // YRUHQSLDPXNGOKMIEBFZCWVJAT
  memcpy(reflector1->mapping, (int[]){25, 18, 21, 8, 17, 19, 12, 4, 16, 24, 14, 7, 15, 11, 13, 9, 5, 2, 6, 26, 3, 23, 22, 10, 1, 20}, 26 * sizeof(int));

    //plugboard
  //struct plugBoard * plugBoard = initPlugBoard((int[]){26,20}, 1); //  int numSwaps = (sizeof(swaps) / sizeof(swaps[0])) / 2; // /2 to get the number of actual iterations
  struct plugBoard * plugBoard;

  // variables defined by the text file input
  struct rotor * LRotor = (struct rotor*)malloc(sizeof(struct rotor)); 
  struct rotor * MRotor = (struct rotor*)malloc(sizeof(struct rotor));
  struct rotor * RRotor = (struct rotor*)malloc(sizeof(struct rotor));
  int LPos, MPos, RPos;
  int LNotch, MNotch, RNotch;
  int ReflectorInput; 
  struct plugBoard * PlugboardBuffer;
  char * Message = (char *)malloc(sizeof(char)*2048); // can change size later
  
  FILE *fptr;
  fptr = fopen("enigma_msg_test.txt", "r");
  
  if (fptr) {
    // Read the file contents
    printf("file opened\n");
    
    char *buffer; // the actual contents
    size_t bufsize = 4096;
    size_t characters; //number of characters grabbed

    buffer = (char *)malloc(bufsize * sizeof(char));
    if( buffer == NULL)
    {
        perror("Unable to allocate buffer");
        exit(1);
    }

    int lineNum = 0;
    while (fgets(buffer,bufsize,fptr)){
      printf("%s", buffer);

      if (lineNum == 0) {
        char* rotorNums = strtok(buffer, " ");
        int i = 0;
        while( rotorNums != NULL) {
          int rotorNum = atoi(rotorNums);
          printf("rotorNums %i\n", rotorNum);
          if (i == 0) {
            if (rotorNum == 1) {
              LRotor = memcpy(LRotor, rotor1, sizeof(struct rotor));
            } else if (rotorNum == 2) {
              LRotor = memcpy(LRotor, rotor2, sizeof(struct rotor));
            } else if (rotorNum == 3) {
              LRotor = memcpy(LRotor, rotor3, sizeof(struct rotor));
            }
          }
          if (i == 1) {
            if (rotorNum == 1) {
              MRotor = memcpy(MRotor, rotor1, sizeof(struct rotor));
            } else if (rotorNum == 2) {
              MRotor = memcpy(MRotor, rotor2, sizeof(struct rotor));
            } else if (rotorNum == 3) {
              MRotor = memcpy(MRotor, rotor3, sizeof(struct rotor));
            }
          }
          if (i == 2) {
            if (rotorNum == 1) {
              RRotor = memcpy(RRotor, rotor1, sizeof(struct rotor));
            } else if (rotorNum == 2) {
              RRotor = memcpy(RRotor, rotor2, sizeof(struct rotor));;
            } else if (rotorNum == 3) {
              RRotor = memcpy(RRotor, rotor3, sizeof(struct rotor));;
            }
          }
          rotorNums = strtok(NULL, " ");
          i += 1;
        }
        free(rotorNums);
      }

      if (lineNum == 1) { }
  
      printf("lineNum %i \n", lineNum);
      lineNum += 1;
    }

    fclose(fptr);

    printf("\n\ndone reading file \n\n");
    free(buffer);
  } else {
      printf("Error opening file\n\n");
  }

  // reading state of the enigma machine
  printf("rotor1(position, ringSetting, notch): %i, %i, %i\n", rotor1->position, rotor1->ringSetting, rotor1->notch);
  printf("rotor2(position, ringSetting, notch): %i, %i, %i\n", rotor2->position, rotor2->ringSetting, rotor2->notch);
  printf("rotor3(position, ringSetting, notch): %i, %i, %i\n", rotor3->position, rotor3->ringSetting, rotor3->notch);
  printf("LRotor(position, ringSetting, notch): %i, %i, %i\n", LRotor->position, LRotor->ringSetting, LRotor->notch);
  printf("MRotor(position, ringSetting, notch): %i, %i, %i\n", MRotor->position, MRotor->ringSetting, MRotor->notch);
  printf("RRotor(position, ringSetting, notch): %i, %i, %i\n", RRotor->position, RRotor->ringSetting, RRotor->notch);
  //printf("");

  printf("\n");
  //exit

  free(rotor1);
  free(rotor2);
  free(rotor3);
  free(plugBoard);
  free(reflector1);
  free(Message);
  free(LRotor);
  free(MRotor);
  free(RRotor);
  return 0;
}