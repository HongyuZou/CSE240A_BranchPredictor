//========================================================//
//  predictor.c                                           //
//  Source file for the Branch Predictor                  //
//                                                        //
//  Implement the various branch predictors below as      //
//  described in the README                               //
//========================================================//
#include <stdio.h>
#include "predictor.h"

//
// TODO:Student Information
//
const char *studentName = "Hongyu Zou, Shicheng Fan";
const char *studentID   = "A14553614, A14824899";
const char *email       = "hoz054@ucsd.edu, shfan@ucsd.edu";

//------------------------------------//
//      Predictor Configuration       //
//------------------------------------//

// Handy Global for use in output routines
const char *bpName[4] = { "Static", "Gshare",
                          "Tournament", "Custom" };

int ghistoryBits; // Number of bits used for Global History
int lhistoryBits; // Number of bits used for Local History
int pcIndexBits;  // Number of bits used for PC index
int bpType;       // Branch Prediction Type
int verbose;

//------------------------------------//
//      Predictor Data Structures     //
//------------------------------------//

//
//TODO: Add your own Branch Predictor data structures here
//

/*
 * Ghare 
 */
uint8_t *gshare_pattern_table = NULL;
int32_t global_branch_history = 0; // shared with tournament, custom
uint32_t bit_mask = 0; // global history bitmask

uint8_t* init_gshare_pattern_table() {
  // calculate history table size
  uint32_t history_table_size = 1;
  for (int i = 0; i < ghistoryBits; i += 1) {
    history_table_size *= 2;
  }

  // init gshare history table
  gshare_pattern_table = (uint8_t*)malloc(sizeof(uint8_t) * history_table_size);
  for (int i = 0; i < history_table_size; i ++) {
    gshare_pattern_table[i] = 1;
  }

  return gshare_pattern_table;
}

uint32_t generate_bit_mask(int bit_count) {
  uint32_t res = 1;
  for (int i = 0; i < bit_count - 1; i += 1) {
    res = (res << 1) + 1;
  }

  return res;
}

/*
 * Tournament
 */
uint8_t *global_pattern_table = NULL; // global pred table

uint8_t *local_pattern_table = NULL;
uint32_t *local_history_table = NULL;

uint8_t *choice_table = NULL;  // shared with custom

uint32_t local_bit_mask = 0;
uint32_t pc_bit_mask = 0;

uint8_t* init_tour_global_table() {
  // calculate history table size
  uint32_t history_table_size = 1;
  for (int i = 0; i < ghistoryBits; i += 1) {
    history_table_size *= 2;
  }

  // init gshare history table
  global_pattern_table = (uint8_t*)malloc(sizeof(uint8_t) * history_table_size);
  for (int i = 0; i < history_table_size; i ++) {
    global_pattern_table[i] = 1; // weakly no
  }

  return global_pattern_table;
}

uint8_t* init_choice_table() {
  // calculate choice table size
  uint32_t choice_table_size = 1;
  for (int i = 0; i < ghistoryBits; i += 1) {
    choice_table_size *= 2;
  }

  // init choice table
  choice_table = (uint8_t*)malloc(sizeof(uint8_t) * choice_table_size);
  for (int i = 0; i < choice_table_size; i ++) {
    choice_table[i] = 2; // weakly global
  }

  return choice_table;
}

uint32_t* init_local_history_table() {
  // calculate table size
  uint32_t table_size = 1;
  for (int i = 0; i < pcIndexBits; i += 1) {
    table_size *= 2;
  }

  local_history_table = (uint32_t*)malloc(sizeof(uint32_t) * table_size);
  for (int i = 0; i < table_size; i ++) {
    local_history_table[i] = 0;
  }

  return local_history_table;
}

uint8_t* init_local_pattern_table() {
  // calculate table size
  uint32_t table_size = 1;
  for (int i = 0; i < lhistoryBits; i += 1) {
    table_size *= 2;
  }

  local_pattern_table = (uint8_t*)malloc(sizeof(uint8_t) * table_size);
  for (int i = 0; i < table_size; i ++) {
    local_pattern_table[i] = 1;
  }

  return local_pattern_table;
}

/*
 * Custom
 */

// bimode hisotry table
uint8_t* global_pattern_t = NULL;
uint8_t* global_pattern_nt = NULL;

void init_custom_tables() {
  ghistoryBits = 11;

  // calculate history table size
  uint32_t history_table_size = 1;
  for (int i = 0; i < ghistoryBits; i += 1) {
    history_table_size *= 2;
  }

  // init history table
  global_pattern_t = (uint8_t*)malloc(sizeof(uint8_t) * history_table_size);
  for (int i = 0; i < history_table_size; i ++) {
    global_pattern_t[i] = 2; // weakly no
  }

  // init history table
  global_pattern_nt = (uint8_t*)malloc(sizeof(uint8_t) * history_table_size);
  for (int i = 0; i < history_table_size; i ++) {
    global_pattern_nt[i] = 1; // weakly no
  }

  // init choice table
  choice_table = (uint8_t*)malloc(sizeof(uint8_t) * history_table_size);
  for (int i = 0; i < history_table_size; i ++) {
    choice_table[i] = 1; // weakly global
  }
}

//------------------------------------//
//        Predictor Functions         //
//------------------------------------//

// Initialize the predictor
//
void
init_predictor()
{
  //
  //TODO: Initialize Branch Predictor Data Structures
  //
  switch (bpType)
  {
    case GSHARE:
      init_gshare_pattern_table();
      break;
    
    case TOURNAMENT:
      init_choice_table();
      init_local_history_table();
      init_local_pattern_table();
      init_tour_global_table();
      break;

    case CUSTOM:
      init_custom_tables();
      break;
  }
}

// Make a prediction for conditional branch instruction at PC 'pc'
// Returning TAKEN indicates a prediction of taken; returning NOTTAKEN
// indicates a prediction of not taken
//
uint8_t
make_prediction(uint32_t pc)
{
  //
  //TODO: Implement prediction scheme
  //
  
  // pred pattern (0-3)
  uint8_t pattern;

  // Make a prediction based on the bpType
  switch (bpType) {
    case STATIC:
      return TAKEN;

    case GSHARE:
      bit_mask = generate_bit_mask(ghistoryBits);
      int32_t pattern_table_idx = ((global_branch_history & bit_mask) ^ (pc & bit_mask));
      pattern = gshare_pattern_table[pattern_table_idx];
      
      // make prediction
      if (pattern == ST || pattern == WT) {
        return TAKEN;
      } else {
        return NOTTAKEN;
      }

      break;
    case TOURNAMENT:
      bit_mask = generate_bit_mask(ghistoryBits);
      uint8_t choice = choice_table[global_branch_history & bit_mask];
      pattern = 0;

      // global prediction vs local prediction
      if (choice >= 2) {
        pattern = global_pattern_table[global_branch_history & bit_mask];  
      } else {
        uint32_t pc_bit_mask = generate_bit_mask(pcIndexBits);
        uint32_t local_bit_mask = generate_bit_mask(lhistoryBits);
        pattern = local_pattern_table[local_history_table[pc & pc_bit_mask] & local_bit_mask];
      }

      // make prediction
      if (pattern == ST || pattern == WT) {
        return TAKEN;
      } else {
        return NOTTAKEN;
      }

      break;
    case CUSTOM:
      bit_mask = generate_bit_mask(ghistoryBits);
      uint32_t history_idx = ((global_branch_history ^ pc) & bit_mask);
      uint32_t choice_idx = (pc & bit_mask);
      pattern = (choice_table[choice_idx] >= 2) ? global_pattern_t[history_idx] : global_pattern_nt[history_idx];
      
      // make prediction
      if (pattern == ST || pattern == WT) {
        return TAKEN;
      } else {
        return NOTTAKEN;
      }

      break;
  }

  // If there is not a compatable bpType then return NOTTAKEN
  return NOTTAKEN;
}

// Train the predictor the last executed branch at PC 'pc' and with
// outcome 'outcome' (true indicates that the branch was taken, false
// indicates that the branch was not taken)
//
void
train_predictor(uint32_t pc, uint8_t outcome)
{
  //
  //TODO: Implement Predictor training
  //

  // Make a prediction based on the bpType
  switch (bpType) {
    case GSHARE:
      bit_mask = generate_bit_mask(ghistoryBits);
      int32_t pattern_table_idx = (global_branch_history & bit_mask) ^ (pc & bit_mask);
      uint32_t pattern = gshare_pattern_table[pattern_table_idx];

      // update global history
      global_branch_history = (global_branch_history << 1) + outcome;

      // update pattern table
      if (outcome == NOTTAKEN && pattern != SN) {
        gshare_pattern_table[pattern_table_idx] -= 1;
      } else if (outcome == TAKEN && pattern != ST) {
        gshare_pattern_table[pattern_table_idx] += 1;
      }
       
      break;
    case TOURNAMENT:

      // update_global_table
      bit_mask = generate_bit_mask(ghistoryBits);
      int32_t global_pattern_idx = (global_branch_history & bit_mask);
      uint8_t global_pattern = global_pattern_table[global_pattern_idx];

      // update global history
      global_branch_history = (global_branch_history << 1) + outcome;

      // update global table
      if (outcome == NOTTAKEN && global_pattern != SN) {
        global_pattern_table[global_pattern_idx] -= 1;
      } else if (outcome == TAKEN && global_pattern != ST) {
        global_pattern_table[global_pattern_idx] += 1;
      }
      
      //update_local_table
      uint32_t pc_bit_mask = generate_bit_mask(pcIndexBits);
      uint32_t local_bit_mask = generate_bit_mask(lhistoryBits);
      int32_t local_pattern_idx = (local_history_table[pc & pc_bit_mask] & local_bit_mask);
      uint8_t local_pattern = local_pattern_table[local_pattern_idx];

      // update local history
      local_history_table[pc & pc_bit_mask] = (local_history_table[pc & pc_bit_mask] << 1) + outcome;

      // update local pattern table
      if (outcome == NOTTAKEN && local_pattern != SN) {
        local_pattern_table[local_pattern_idx] -= 1;
      } else if (outcome == TAKEN && local_pattern != ST) {
        local_pattern_table[local_pattern_idx] += 1;
      }

      //update_choice_table
      uint8_t choice = choice_table[global_pattern_idx];
      uint8_t global_pred = global_pattern >= 2 ? TAKEN : NOTTAKEN;
      uint8_t local_pred = local_pattern >= 2 ? TAKEN : NOTTAKEN;
      
      // if equal then tie, no need to update choice
      if (global_pred != local_pred) {
        uint8_t temp_outcome = (outcome == global_pred);
        if (temp_outcome == 0 && choice != SN) {
          choice_table[global_pattern_idx] -= 1;
        } else if (temp_outcome == 1 && choice != ST) {
          choice_table[global_pattern_idx] += 1;
        }
      }
      break;
    case CUSTOM:
      bit_mask = generate_bit_mask(ghistoryBits);
      uint32_t history_idx = ((pc ^ global_branch_history) & bit_mask);
      uint32_t choice_idx = (pc & bit_mask);
      uint8_t choice_taken = (choice_table[choice_idx] >= 2) ? TAKEN : NOTTAKEN;
      uint32_t custom_pattern = (choice_taken == 1) ? global_pattern_t[history_idx] : global_pattern_nt[history_idx];
      uint8_t pred_res = (custom_pattern >= 2) ? TAKEN : NOTTAKEN; 

      // choice: 0 1 2 3
      // pattern: N, n, t, T

      // update choice table
      if (!(choice_taken != outcome && pred_res == outcome)) {
        // update global table
        if (outcome == NOTTAKEN && choice_table[choice_idx] != SN) {
          choice_table[choice_idx] -= 1;
        } else if (outcome == TAKEN && choice_table[choice_idx] != ST) {
          choice_table[choice_idx] += 1;
        }
      }

      uint8_t *target_table = (choice_taken == 1) ? global_pattern_t : global_pattern_nt;
      // update global table
      if (outcome == NOTTAKEN && custom_pattern != SN) {
        target_table[history_idx] -= 1;
      } else if (outcome == TAKEN && custom_pattern != ST) {
        target_table[history_idx] += 1;
      }

      // update global history
      global_branch_history = (global_branch_history << 1) + outcome;

      break;
    default:
      break;
  }
}

void
clean_up() {
  // gshare
  free(gshare_pattern_table);
  
  // tournament
  free(global_pattern_table);
  free(local_pattern_table);
  free(local_history_table);
  free(choice_table);

  // custom
  free(global_pattern_t);
  free(global_pattern_nt);
}
