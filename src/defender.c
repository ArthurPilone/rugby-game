// Standard headers
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Internal headers
#include "direction.h"
#include "position.h"
#include "spy.h"

// Main header
#include "defender.h"

// Macros
#define UNUSED(x) (void)(x) // Auxiliary to avoid error of unused parameter

#define  NULL_POSITION         (position_t)  {-1, -1}

#define  MAX_SAVED_POSITIONS 50

// Internal Functions Declaration
void initialize_position_array(position_t*, int);

short position_array_contains(position_t, position_t*, int);

short add_position_to_array(position_t x, position_t*, int, int);

/*----------------------------------------------------------------------------*/
/*                              PUBLIC FUNCTIONS                              */
/*----------------------------------------------------------------------------*/

direction_t execute_defender_strategy(
    position_t defender_position, Spy attacker_spy) {
  // TODO: unused parameters, remove these lines later
  UNUSED(attacker_spy);

  // State Variables
  static int tick = 0;
  static short jiggling = 1;

  static size_t attacker_i;

  direction_t nextDir; 

  if(tick == 0){
    srand((unsigned) 151115 + time(NULL));
  }else if(tick == 3){
    attacker_i = get_spy_position(attacker_spy).i;
    jiggling = 0;
  }

  if(jiggling){
    if(rand() % 2){
      nextDir = (direction_t) DIR_UP;
    }else{
      nextDir = (direction_t) DIR_DOWN;
    }
  }else{
    if(attacker_i > defender_position.i){
      nextDir = (direction_t) DIR_DOWN;
    }else if(attacker_i < defender_position.i){
      nextDir = (direction_t) DIR_UP;
    }else{
      jiggling = 1;
      nextDir = (direction_t) DIR_STAY;
    }
  }
  
  tick++;
  return nextDir;
}