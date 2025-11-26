/*******************************************
  operations
*********************************************/
#define NUM_OF_OP_TYPES 6
enum op_type {ADD, SUB, MUL, DIV, LD, ST};

/* data structure for an instruction */
typedef struct instruction {
  int num; /* number: starting from 1 */
  enum op_type op; /* operation type */
  int rd; /* destination register id */
  int rs; /* source register id or base register for ld/st */
  int rt; /* target register id or addr offset for ld/st */
} INST;


#define NUM_OF_INST 6
extern INST inst[NUM_OF_INST]; /* instruction array */ 

void init_inst();
void print_inst(INST ins);
void print_program();
