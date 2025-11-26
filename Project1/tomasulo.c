#include <stdio.h>
#include <stdlib.h>
#include "arch.h"


/*******************************
main
*******************************/
int main()
{
  int i,j,k;
  int done = 0;
  int cycle = 0;
  int num_issued_inst = 0;

  init_inst();
  init_fu();

  printf("============== TEST INSTRUCTION SEQUENCE ===========\n");
  print_program();

  init_rs();	// initialize RS entries
  init_regs();	// initialize registers
  init_mem();	// initialize memory

  printf("* CYCLE %d (initial state)\n",cycle);
  print_rs();	// print initial RS state
  print_regs();	// print initial register state

  /* simulation loop main */
  while(!done){

    /* increment the cycle */
    cycle++;
    
    /********************************
     *     Step III: Write result 
     ********************************/
    for(i=0;i<NUM_RS_ENTRIES;i++) {
      // complete STEP III here
      RS * curr_rs = get_rs(i+1);
      if(curr_rs->is_result_ready == true) {
        if(curr_rs->op==ADD || curr_rs->op==SUB || curr_rs->op==MUL ||  curr_rs->op==DIV || curr_rs->op==LD) {
          for(j=0;j<NUM_REGS;j++) {
            if(regs[j].Qi == curr_rs->id) {
              regs[j].val = curr_rs->result;
              regs[j].Qi = 0;
            }
          }
          for(k=0;k<NUM_RS_ENTRIES;k++) {
            RS * dependent_rs = get_rs(k+1);
            if(dependent_rs->Qj == curr_rs->id) {
              dependent_rs->Vj = curr_rs->result;
              dependent_rs->Qj = 0;
            }
            if(dependent_rs->Qk == curr_rs->id) {
              dependent_rs->Vk = curr_rs->result;
              dependent_rs->Qk = 0;
            }
          }
        }
        else if(curr_rs->op==ST) {
          if(curr_rs->Qk == 0) {
            set_mem(curr_rs->Vj + curr_rs->A, curr_rs->Vk);
          }
        }
        reset_rs_entry(curr_rs);
      }
    }

    /********************************
     *     Step II: Execute 
     ********************************/
    for(i=0;i<NUM_RS_ENTRIES;i++) {
      // complete STEP II here
      RS * curr_rs = get_rs(i+1);
      if(curr_rs->is_busy == true){
        if(curr_rs->op==ADD || curr_rs->op==SUB || curr_rs->op==MUL ||  curr_rs->op==DIV) {
          if(curr_rs->Qj == 0 && curr_rs->Qk == 0) {
            curr_rs->in_exec = true;
            if(curr_rs->exec_cycles==1) {
              if(curr_rs->op==ADD) {
                curr_rs->result = curr_rs->Vj + curr_rs->Vk;
              }
              else if(curr_rs->op==SUB) {
                curr_rs->result = curr_rs->Vj - curr_rs->Vk;
              }
              else if(curr_rs->op==MUL) {
                curr_rs->result = curr_rs->Vj * curr_rs->Vk;
              }
              else if(curr_rs->op==DIV) {
                curr_rs->result = curr_rs->Vj / curr_rs->Vk;
              }
              curr_rs->is_result_ready = true;
            }
            else {
              curr_rs->exec_cycles = curr_rs->exec_cycles - 1;
            }
          }
        }
        else if(curr_rs->op==LD || curr_rs->op==ST) {
          if(curr_rs->Qj == 0) {
            curr_rs->in_exec = true;
            if(curr_rs->exec_cycles==1) {
              if(curr_rs->op==LD) {
                curr_rs->result = get_mem(curr_rs->Vj + curr_rs->A);
              }
              if(curr_rs->op==ST) {
                //set_mem(curr_rs->Vj + curr_rs->A, curr_rs->Vk);
              }
              curr_rs->is_result_ready = true;
            }
            else {
              curr_rs->exec_cycles = curr_rs->exec_cycles - 1;
            }
          }
        }
      }
    }

    /********************************
     *     Step I: Issue 
     ********************************/

    /*  wait if no RS entry is available */
    if(num_issued_inst < NUM_OF_INST) {
      int cand_rs_id; 
      if(inst[num_issued_inst].op==ADD) cand_rs_id = obtain_available_rs(ADD_RS);
      else if(inst[num_issued_inst].op==SUB) cand_rs_id = obtain_available_rs(ADD_RS);
      else if(inst[num_issued_inst].op==MUL) cand_rs_id = obtain_available_rs(MUL_RS);
      else if(inst[num_issued_inst].op==DIV) cand_rs_id = obtain_available_rs(MUL_RS);
      else if(inst[num_issued_inst].op==LD) cand_rs_id = obtain_available_rs(LD_BUF);
      else if(inst[num_issued_inst].op==ST) cand_rs_id = obtain_available_rs(ST_BUF);

      /* if there is an available RS entry */
      if(cand_rs_id!=-1) {
        /* issue the instruction: See Fig. 3.13 */
	    RS * curr_rs = get_rs(cand_rs_id);
        if(curr_rs==NULL) {
          printf("NO RS found with the given id\n");
          exit(1);
        }

        /* normal ALU operations */
        if(inst[num_issued_inst].op==ADD || inst[num_issued_inst].op==SUB ||
          inst[num_issued_inst].op==MUL ||  inst[num_issued_inst].op==DIV) {
          int rd, rs, rt;
          rd = inst[num_issued_inst].rd;
          rs = inst[num_issued_inst].rs;
          rt = inst[num_issued_inst].rt;

          /* Rs */
          if(regs[rs].Qi!=0) curr_rs->Qj = regs[rs].Qi;
          else curr_rs->Vj = regs[rs].val;

          /* Rt */
          if(regs[rt].Qi!=0) curr_rs->Qk = regs[rt].Qi;
          else curr_rs->Vk = regs[rt].val;

          /* set busy */
          curr_rs->is_busy = true;
          curr_rs->op = inst[num_issued_inst].op;

          /* register update */
          regs[rd].Qi = curr_rs->id;

          /* set exec cycles */
          if(inst[num_issued_inst].op==ADD) curr_rs->exec_cycles=LAT_ADD;
          else if(inst[num_issued_inst].op==SUB) curr_rs->exec_cycles=LAT_SUB;
          else if(inst[num_issued_inst].op==MUL) curr_rs->exec_cycles=LAT_MUL;
          else if(inst[num_issued_inst].op==DIV) curr_rs->exec_cycles=LAT_DIV;

          /* num issued ++ */
          num_issued_inst++;
        }
        else if(inst[num_issued_inst].op==LD) {
          int rd, rs, imm;
          rd = inst[num_issued_inst].rd;
          rs = inst[num_issued_inst].rs;
          imm = inst[num_issued_inst].rt;

          /* Rs */
          if(regs[rs].Qi!=0) curr_rs->Qj = regs[rs].Qi;
          else curr_rs->Vj = regs[rs].val;

          /* addr */
          curr_rs->A = imm;

          /* set busy */
          curr_rs->is_busy = true;
          curr_rs->op = inst[num_issued_inst].op;

          /* set exec cycles */
          curr_rs->exec_cycles=LAT_LD;

          /* register update */
          regs[rd].Qi = curr_rs->id;

          /* num issued ++ */
          num_issued_inst++;
        }
        else if(inst[num_issued_inst].op==ST) {
          int rd, rs, imm;
          rd = inst[num_issued_inst].rd;
          rs = inst[num_issued_inst].rs;
          imm = inst[num_issued_inst].rt;

          /* Rs */
          if(regs[rs].Qi!=0) curr_rs->Qj = regs[rs].Qi;
          else curr_rs->Vj = regs[rs].val;

          /* Rd */
          if(regs[rd].Qi!=0) curr_rs->Qk = regs[rd].Qi;
          else curr_rs->Vk = regs[rd].val;

          /* addr */
          curr_rs->A = imm;

          /* set busy */
          curr_rs->is_busy = true;
          curr_rs->op = inst[num_issued_inst].op;

          /* set exec cycles */
          curr_rs->exec_cycles=LAT_ST;

          /* num issued ++ */
          num_issued_inst++;
        }
      }
    }

    /* print out the result */
    printf("* CYCLE %d\n",cycle);
    print_rs();
    print_regs();
   
    /* check the termination condition */ 
    if((num_issued_inst>=NUM_OF_INST) && !is_rs_active())
      done =1;
  }
  return 0;
}
