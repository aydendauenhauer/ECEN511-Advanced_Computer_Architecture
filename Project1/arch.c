#include <stdio.h>
#include "arch.h"

REG regs[NUM_REGS];
int mem_arr[MEM_SIZE];
RS rs_array[NUM_RS_ENTRIES];

bool is_add_available;
bool is_mul_available;
bool is_mem_available;

bool is_rs_active()
{
  int i;
  for(i=0;i<NUM_RS_ENTRIES;i++)
    if(rs_array[i].is_busy) return true;
  return false;
}

void set_mem(int addr, int val)
{
  mem_arr[addr] = val;
  return;
}

int get_mem(int addr)
{
  return mem_arr[addr];
}

void init_mem()
{
  int i;
  for(i=0;i<MEM_SIZE;i++) mem_arr[i] = i%16;
  return;
}

void init_fu()
{
  is_add_available=true;
  is_mul_available=true;
  is_mem_available=true;
  return;
}

void init_regs()
{
  int i=0;
  for(i=0;i<NUM_REGS;i++)
  {
    regs[i].num = i;
    regs[i].val = i;
    regs[i].Qi = 0;
  }
}

void print_regs() 
{
  int i=0;
  printf("=====================================================================\n");
  printf("Registers\n");
  printf("---------------------------------------------------------------------\n");
  for(i=0;i<NUM_REGS;i++)
  {
    if(i%8==7) { 
      printf(" \tr%d\tr%d\tr%d\tr%d\tr%d\tr%d\tr%d\tr%d\n",
              regs[i-7].num,
              regs[i-6].num,
              regs[i-5].num,
              regs[i-4].num,
              regs[i-3].num,
              regs[i-2].num,
              regs[i-1].num,
              regs[i].num);
      printf("---------------------------------------------------------------------\n");
      printf("val\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\n",
              regs[i-7].val,
              regs[i-6].val,
              regs[i-5].val,
              regs[i-4].val,
              regs[i-3].val,
              regs[i-2].val,
              regs[i-1].val,
              regs[i].val);
      printf("Qi\t#%d\t#%d\t#%d\t#%d\t#%d\t#%d\t#%d\t#%d\n",
              regs[i-7].Qi,
              regs[i-6].Qi,
              regs[i-5].Qi,
              regs[i-4].Qi,
              regs[i-3].Qi,
              regs[i-2].Qi,
              regs[i-1].Qi,
              regs[i].Qi);
      printf("---------------------------------------------------------------------\n");
    }
  }
  printf("=====================================================================\n");
  return;
  
}

void reset_rs_entry(RS * t)
{ 
    t->is_busy = false;
    t->op = -1;
    t->Qj = 0;
    t->Qk = 0;
    t->Vj = -1;
    t->Vk = -1;
    t->A = -1;
    t->exec_cycles = -1;
    t->result = -1;
    t->in_exec = false;
    t->is_result_ready = false;
    t->inst_num = -1;
    return;
}
/**************************************
* initialize RS 
***************************************/
void init_rs()
{
  int i;
  int curr_ind=0;
  RS * curr_entry;
  
  /* common for all */
  for(i=0;i<NUM_RS_ENTRIES;i++) {
    curr_entry = &(rs_array[i]);
    curr_entry->id = i+1;
    curr_entry->is_busy = false;
    curr_entry->op = -1;
    curr_entry->Qj = 0;
    curr_entry->Qk = 0;
    curr_entry->Vj = -1;
    curr_entry->Vk = -1;
    curr_entry->A = -1;
    curr_entry->exec_cycles = -1;
    curr_entry->result = -1;
    curr_entry->in_exec = false;
    curr_entry->is_result_ready = false;
    curr_entry->inst_num = -1;
  }
  /* types */
  for(i=0;i<NUM_LD_BUF;i++) rs_array[curr_ind++].type=LD_BUF;
  for(i=0;i<NUM_ST_BUF;i++) rs_array[curr_ind++].type=ST_BUF;
  for(i=0;i<NUM_ADD_RS;i++) rs_array[curr_ind++].type=ADD_RS;
  for(i=0;i<NUM_MUL_RS;i++) rs_array[curr_ind++].type=MUL_RS;
}

/**************************************
* print RS 
***************************************/
void print_rs()
{
  int i;

  printf("==================================================================================================\n");
  printf("RS_id\ttype\tBusy\tinst#\tOp\tVj\tVk\tQj\tQj\tA\tExec\tDone\n");
  printf("--------------------------------------------------------------------------------------------------\n");
  for(i=0;i<NUM_RS_ENTRIES;i++) {
    printf("#%d\t",rs_array[i].id); // id
    if(rs_array[i].type==LD_BUF) printf("LD\t"); // RS buff type
    else if(rs_array[i].type==ST_BUF) printf("ST\t"); 
    else if(rs_array[i].type==ADD_RS) printf("ADD\t"); 
    else if(rs_array[i].type==MUL_RS) printf("MUL\t"); 
    else printf("NONE\t");
    if(rs_array[i].is_busy) printf("Yes\t"); else printf("No\t"); // Busy
    printf("I#%d\t",rs_array[i].inst_num);
    if(rs_array[i].op==ADD) printf("add\t"); // instr type
    else if(rs_array[i].op==SUB) printf("sub\t"); 
    else if(rs_array[i].op==MUL) printf("mul\t"); 
    else if(rs_array[i].op==DIV) printf("div\t"); 
    else if(rs_array[i].op==LD) printf("ld\t"); 
    else if(rs_array[i].op==ST) printf("st\t"); 
    else printf("NONE\t");
    printf("%d\t%d\t#%d\t#%d\t%d\t",rs_array[i].Vj,rs_array[i].Vk,rs_array[i].Qj,rs_array[i].Qk,rs_array[i].A);
    if(rs_array[i].in_exec) printf("Yes\t"); else printf("No\t"); // in Execution?
    if(rs_array[i].is_result_ready) printf("Yes\n"); else printf("No\n"); // done Execution?
    
  }
  printf("==================================================================================================\n");
  
  return;
}


int obtain_available_rs(enum rs_type t)
{
  int i;
  for(i=0;i<NUM_RS_ENTRIES;i++) {
    if((rs_array[i].type==t) && !rs_array[i].is_busy) return rs_array[i].id;
  }
  return -1; /* no available RS entry */
}

RS * get_rs(int id)
{
  int i=0;
  for(i=0;i<NUM_RS_ENTRIES;i++) {
    if(rs_array[i].id == id) return &(rs_array[i]);
  }
  return NULL;
}
