// Standard headers
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Internal headers
#include "direction.h"
#include "position.h"
#include "spy.h"

// Main header
#include "attacker.h"

// Macros
#define  NULL_POSITION         (position_t)  {-1, -1}

#define  MAX_SAVED_POSITIONS 50

// Private Structs

typedef struct VisitedPosition {
  position_t pos;
  int visits;
}visited_position;

// Internal Functions Declaration
int directions_equal(direction_t, direction_t);

void initialize_position_array(position_t*, int);
short position_array_contains(position_t, position_t*, int);

void initialize_visits_array(visited_position*, int);
int add_visit_to_visits_array(visited_position*, int, int, position_t);
int get_visits_to_pos(visited_position*, int, position_t);

void initialize_tested_directions(direction_t*);
int check_for_tested_direction(direction_t*, direction_t);
void add_tested_direction(direction_t*, direction_t);

direction_t choose_next_direction(
  direction_t*, visited_position*, position_t*, int, position_t, size_t);

void rank_directions (direction_t*, float);

direction_t check_for_deadend(position_t*, position_t);

// State Variables
int tick = 0;
int saved_positions_no = 0;
int known_walls_no = 0;

visited_position visited_positions[MAX_SAVED_POSITIONS];
position_t known_walls[MAX_SAVED_POSITIONS];
position_t last_visited_position;
position_t last_tested_position;

size_t farthest_j_coord = 0;

direction_t tested_directions[8];

/*----------------------------------------------------------------------------*/
/*                              PUBLIC FUNCTIONS                              */
/*----------------------------------------------------------------------------*/

direction_t execute_attacker_strategy(
    position_t attacker_position, Spy defender_spy) {

  short moved = 1;
  direction_t at_deadend = (direction_t) DIR_STAY;
  direction_t nextDir;

  if(!tick){
    initialize_visits_array(visited_positions, MAX_SAVED_POSITIONS);
    initialize_position_array(known_walls, MAX_SAVED_POSITIONS);

    farthest_j_coord = get_spy_position(defender_spy).j;

    srand((unsigned) 11795450 + time(NULL));
  }else{
    moved = !equal_positions(attacker_position, last_visited_position);
  }

  if(moved){
    initialize_tested_directions(tested_directions);

    if(attacker_position.j > farthest_j_coord){
      farthest_j_coord = attacker_position.j;
    }

    add_visit_to_visits_array(visited_positions, MAX_SAVED_POSITIONS,saved_positions_no , attacker_position);

    if(get_visits_to_pos(visited_positions,MAX_SAVED_POSITIONS,attacker_position) == 1){
      saved_positions_no++;
    }
  }else{
    known_walls[known_walls_no % MAX_SAVED_POSITIONS] = last_tested_position;
    known_walls_no++;
    at_deadend = check_for_deadend(known_walls,attacker_position);
  }

  if(directions_equal(at_deadend,(direction_t) DIR_STAY)){

    nextDir = choose_next_direction(
        tested_directions,visited_positions,
        known_walls,known_walls_no,attacker_position, farthest_j_coord);
  
  }else{
    nextDir = at_deadend;
  }

  add_tested_direction(tested_directions,nextDir);
  last_tested_position = move_position(attacker_position,nextDir);
  last_visited_position = attacker_position;

  tick++;

  return nextDir;
}

/*----------------------------------------------------------------------------*/
/*                       PRIVATE / AUXILIARY FUNCTIONS                        */
/*----------------------------------------------------------------------------*/

int directions_equal(direction_t x, direction_t y){
  return (x.i == y.i && x.j == y.j);
}

/*----------------------------------------------------------------------------*/

void initialize_position_array(position_t* arr_start, int arr_length){
  for(int i = 0; i < arr_length; i++){
    arr_start[i] = NULL_POSITION; 
  }
}

short position_array_contains(position_t x, position_t* pos_arr, int arr_length){

  if(equal_positions(NULL_POSITION, x)){
    return 1;
  }

  for(int i = 0; i < arr_length; i++){
    if(equal_positions(pos_arr[i],x)){
      return 1;
    }
  }

  return 0;
}

/*----------------------------------------------------------------------------*/

void initialize_visits_array(visited_position* arr, int arr_length){
  for(int k = 0; k < arr_length;k++){
    arr[k].pos = NULL_POSITION;
    arr[k].visits = 0;
  }
}

int add_visit_to_visits_array(visited_position* arr, int arr_length, int saved_pos_no, position_t pos){
  for(int k = 0; k < arr_length; k++){
    if(equal_positions(arr[k].pos, pos)){
      arr[k].visits++;
      return 0;
    }
  }

  arr[saved_pos_no % arr_length].pos = pos;
  arr[saved_pos_no % arr_length].visits = 1;
  return 1;
}

int get_visits_to_pos(visited_position* arr, int arr_length, position_t pos){
  for(int k = 0; k < arr_length; k++){
    if(equal_positions(arr[k].pos, pos)){
      return arr[k].visits;
    }
  }
  return 0;
}

/*----------------------------------------------------------------------------*/

void initialize_tested_directions(direction_t* arr){
  for(int i = 0; i < 8; i++){
    arr[i] = (direction_t) DIR_STAY;
  }
}

int check_for_tested_direction(direction_t* arr, direction_t x){
  for(int k = 0; k < 8; k++){
    if(arr[k].i == x.i && arr[k].j == x.j){
      return 1;
    }
  }

  return 0;
}

void add_tested_direction(direction_t* arr, direction_t x){
  for(int k = 0; k < 8; k++){
    if(arr[k].i == 0 && arr[k].j == 0){
      arr[k] = x;
      return;
    }
  }
}

/*----------------------------------------------------------------------------*/

direction_t choose_next_direction(
    direction_t* tested_directions, visited_position* visited_positions,
    position_t* known_walls, int known_walls_no, position_t attacker_pos, size_t farthest_j_coord){

  direction_t direction_queue[8];

  // vert_rand_vs_hor_spd === vertical_randomness_vs_horizontal_speed (surely a mouthful !)
  float vert_rand_vs_hor_spd = (float) attacker_pos.j / farthest_j_coord;
  vert_rand_vs_hor_spd = 0.7 * vert_rand_vs_hor_spd + 0.3;

  rank_directions(direction_queue, vert_rand_vs_hor_spd);

  direction_t best_candidate = DIR_STAY;
  size_t visits_to_best_candidate = 99999;

  for(int k = 0; k < 8; k++){
    position_t candidate_pos = move_position(attacker_pos, direction_queue[k]);
    size_t visits_to_candidate_pos = (size_t) get_visits_to_pos(visited_positions,MAX_SAVED_POSITIONS,candidate_pos);

    if(check_for_tested_direction(tested_directions,direction_queue[k]) ||
      position_array_contains(candidate_pos,known_walls, known_walls_no)){
        continue;
    }
    else if( (visits_to_best_candidate +2* (best_candidate.i)) > (visits_to_candidate_pos +2*candidate_pos.i)){
      best_candidate = direction_queue[k];
      visits_to_best_candidate = visits_to_candidate_pos;
    }

  }

  return best_candidate;

}

void rank_directions (direction_t* direction_queue, float coef){
  if(rand() <= coef * RAND_MAX){
    if(rand() % 2){
      direction_queue[0] = (direction_t) DIR_UP_RIGHT;
      direction_queue[1] = (direction_t) DIR_DOWN_RIGHT;
    }else{
      direction_queue[0] = (direction_t) DIR_DOWN_RIGHT;
      direction_queue[1] = (direction_t) DIR_UP_RIGHT;
    }
    direction_queue[2] = (direction_t) DIR_RIGHT;
  }else{
    direction_queue[0] = (direction_t) DIR_RIGHT;
    if(rand() % 2){
      direction_queue[1] = (direction_t) DIR_UP_RIGHT;
      direction_queue[2] = (direction_t) DIR_DOWN_RIGHT;
    }else{
      direction_queue[1] = (direction_t) DIR_DOWN_RIGHT;
      direction_queue[2] = (direction_t) DIR_UP_RIGHT;
    }
  }

  if(rand() % 2){
    direction_queue[3] = (direction_t) DIR_UP;
    direction_queue[4] = (direction_t) DIR_DOWN;
  }else{
    direction_queue[3] = (direction_t) DIR_DOWN;
    direction_queue[4] = (direction_t) DIR_UP;
  }
  
  if(rand() % 2){
    direction_queue[5] = (direction_t) DIR_UP_LEFT;
    direction_queue[6] = (direction_t) DIR_DOWN_LEFT;
  }else{
    direction_queue[5] = (direction_t) DIR_DOWN_LEFT;
    direction_queue[6] = (direction_t) DIR_UP_LEFT;
  }

  direction_queue[7] = (direction_t) DIR_LEFT;
}

/*----------------------------------------------------------------------------*/

direction_t check_for_deadend(position_t* known_walls, position_t attacker_pos){
  int known_exits = 0;
  direction_t possible_exit = DIR_STAY;
  direction_t directions[8] = {DIR_DOWN,DIR_DOWN_LEFT,DIR_DOWN_RIGHT,
                        DIR_UP,DIR_UP_LEFT,DIR_UP_RIGHT,DIR_LEFT,DIR_RIGHT};

  for(int k = 0; k < 8; k++){
    if( !position_array_contains((move_position(attacker_pos, directions[k])), known_walls, MAX_SAVED_POSITIONS )){
      known_exits++;
      if(known_exits > 1){
        return (direction_t) DIR_STAY;
      }else{
        possible_exit = directions[k];
      }
    }
  }

  return possible_exit;
}

/*----------------------------------------------------------------------------*/