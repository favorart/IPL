#include <stdlib.h>
#include "interpreter.h"
#include "timer.h"

int  main (int arg_c, char** arg_v)
{ double time = 0;
  interpreter Interpreter = {0};

  if( arg_c > 1 ) Interpreter.script = arg_v[1];
  if( arg_c > 2 ) Interpreter.config = arg_v[2];

  // Interpreter.script = "input_prog_solve_simple_linEq.txt";                // FAIL !!! FULL FAIL !!!
  // Interpreter.script = "input_prog_count_in_month.txt";                    // PASS !!!
  // Interpreter.script = "input_prog_work_all.txt";                          // PASS !!!
  // Interpreter.script = "input_prog_work_arithmetic.txt";                   // PASS !!!
  // Interpreter.script = "input_prog_work_concat_escape.txt";                // FAIL !!! FREE BAD ADRESS !!!  
  // Interpreter.script = "input_prog_work_container.txt";                    // PASS !!!
  // Interpreter.script = "input_prog_work_input_const.txt";                  // PASS !!!
  // Interpreter.script = "input_prog_work_length.txt";                       // PASS !!!
  // Interpreter.script = "input_prog_work_while.txt";                        // PASS !!!
  // Interpreter.script = "input_test_err_allow_symbol_incorrect_word.txt";   // FAIL !!!
  // Interpreter.script = "input_test_input_const_err.txt";                   // PASS !!!
  // Interpreter.script = "input_test_mem.txt";                               // PASS !!!
  // Interpreter.script = "input_test_solid.txt";                             // FAIL !!! 
  
  // Interpreter.script = "input.txt";                               // PASS !!!

  // конфигурирование
  // Interpreter.config = "rus";

  // флаги изменения синтаксиса
     Interpreter.Syntax.variables_declaration = IPL_SYNTAX_VAR_DECL_PASCAL;
     Interpreter.Syntax.at_vars_pas_sepr_type = IPL_SYNTAX_VAR_PAS_SEPR_FIELD;
     Interpreter.Syntax.at_vars_cnt_sepr_type = IPL_SYNTAX_VAR_CNT_SEPR_INDEX;

     Interpreter.Syntax.at_func_params_commas = 1; // PASS !!!
     Interpreter.Syntax.at_exp_end_semicolons = 1; // PASS !!!
     Interpreter.Syntax.at_stat_return_result = 1;
  // Interpreter.Syntax.at_func_always_braces = 1; // PASS !!!

     Interpreter.Syntax.at_for_always_new_var = 1; // PASS !!!
     Interpreter.Syntax.at_for_each_container = 1; // PASS !!!
     Interpreter.Syntax.at_for_expr_condition = 1; // PASS !!!
     Interpreter.Syntax.at_for_while_using_do = 1; // PASS !!!

  // Interpreter.Syntax.at_for_c_style_syntax = 1; // PASS !!!
  // Interpreter.Syntax.at_obj_method_use_fld = 1;
     Interpreter.Syntax.at_obj_method_precall = 1;

     Interpreter.Syntax.at_if_else_using_then = 1; // PASS !!!
  // Interpreter.Syntax.at_while_false_condit = 1; // ???

  TIMER_START (time);
   if( Interpret (&Interpreter) ) 
   { printf ("There were some errors.\n"); 
     system ("pause"); 
     return 1;
   }
  TIMER_STOP (time);
  printf ("Time of execution = %.3f sec.\n", time);
  
  system ("pause");
  return 0;
}
