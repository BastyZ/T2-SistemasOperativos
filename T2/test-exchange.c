#include "nSystem.h"

/* Importante: cuando su programa no funcione y de un error fatal,
 * use ddd (o gdb) poniendo un breakpoint en nFatalError.
 * Esto le permitira determinar el contexto en donde se produjo
 * el error.
 */

int task_proc(nTask target, int delay, int timeout, void *msg, void *expected) {
  void *exch_msg;
  nSleep(delay);
  exch_msg= nExchange(target, msg, timeout);
  if (expected!=exch_msg)
    nFatalError("task_proc", "no se recibio el mensaje esperado\n");
  return 0;
}

int test1() {
  int msg1, msg2, *exch_msg;
  nTask t= nEmitTask(task_proc, nCurrentTask(), 10, -1, &msg1, &msg2);

  exch_msg= nExchange(t, &msg2, -1);
  if (exch_msg != &msg1)
    nFatalError("nMain", "no se recibio el mensaje esperado\n");
  nWaitTask(t);
  return 0;
}

int test2() {
  int msg1_a, msg2_a, msg1_b, msg2_b, *exch_msg_a, *exch_msg_b;
  nTask t_a= nEmitTask(task_proc, nCurrentTask(), 10, -1, &msg1_a, &msg2_a);
  nTask t_b= nEmitTask(task_proc, nCurrentTask(), 20, -1, &msg1_b, &msg2_b);

  exch_msg_b= nExchange(t_b, &msg2_b, -1);
  if (exch_msg_b != &msg1_b)
    nFatalError("nMain", "no se recibio el mensaje esperado\n");

  exch_msg_a= nExchange(t_a, &msg2_a, -1);
  if (exch_msg_a != &msg1_a)
    nFatalError("nMain", "no se recibio el mensaje esperado\n");
  nWaitTask(t_a);
  nWaitTask(t_b);
  return 0;
}

int test3b() {
  int msg1, msg2, *exch_msg;
  nTask t= nEmitTask(task_proc, nCurrentTask(), 100, -1, &msg1, &msg2);

  exch_msg= nExchange(t, &msg2, 50);
  if (exch_msg != NULL) {
    if (exch_msg!=&msg1)
      nFatalError("nMain", "no se recibio NULL\n");
  }
  else {
    exch_msg= nExchange(t, &msg2, -1);
    if (exch_msg != &msg1)
      nFatalError("nMain", "no se recibio el mensaje esperado\n");
    nWaitTask(t);
  }
  return 0;
}

#define NTASKS 10000
#define NITER  100000

int test_parallel() {
  nTask tasks[NTASKS*3];
  int i, k;
  nSetTimeSlice(1);
  for (k=0; k<NTASKS; k++) {
    tasks[3*k]= NULL;
    tasks[3*k+1]= NULL;
    tasks[3*k+2]= NULL;
  }
   
  k= 0;
  for (i=0; i<NITER; i++) {
    if (tasks[3*k]!=NULL)
      nWaitTask(tasks[3*k]);
    tasks[3*k]= nEmitTask(test1);
    if (tasks[3*k+1]!=NULL)
      nWaitTask(tasks[3*k+1]);
    tasks[3*k+1]= nEmitTask(test2);
    if (tasks[3*k+2]!=NULL)
      nWaitTask(tasks[3*k+2]);
    tasks[3*k+2]= nEmitTask(test3b);
    k++;
    if (k==NTASKS)
      k= 0;
  }

  for (k=0; k<NTASKS; k++) {
    nWaitTask(tasks[3*k]);
    nWaitTask(tasks[3*k+1]);
    nWaitTask(tasks[3*k+2]);
  }
  return 0;
}

int nMain(int argc, int argv) {

  nPrintf("Test 1: Verifica que los mensajes intercambiados sean correctos\n");
  test1();

  nPrintf("Test 2: Verifica que el intercambio ocurra con la tarea correcta\n");
  test2();

  nPrintf("Test 3: Verifica que un timeout expire\n");
  {
    int msg1, msg2, *exch_msg;
    nTask t= nEmitTask(task_proc, nCurrentTask(), 100, 100, &msg1, &msg2);

    exch_msg= nExchange(t, &msg2, 50);
    if (exch_msg != NULL)
      nFatalError("nMain", "no se recibio NULL\n");

    exch_msg= nExchange(t, &msg2, 100);
    if (exch_msg != &msg1)
      nFatalError("nMain", "no se recibio el mensaje esperado\n");
    nWaitTask(t);
  }
  nPrintf("Test 3 b: Como el test 3, pero una tarea con timeout\n");
  test3b();

  nPrintf("Test 4: Verifica que ambos timeouts expiren\n");
  {
    int msg1, msg2, *exch_msg;
    nTask t = nEmitTask(task_proc, nCurrentTask(),100,100, &msg1,NULL);

    exch_msg= nExchange(t, &msg2, 50);
    if (exch_msg != NULL)
      nFatalError("nMain", "no se recibio NULL\n");

    nSleep(200);
    exch_msg= nExchange(t, &msg2, 10);
    if (exch_msg != NULL)
      nFatalError("nMain", "no se recibio NULL\n");
    nWaitTask(t);
  }

  nPrintf("Test 5: Test de robustez con nSetTimeSlice(1)\n");
  test_parallel();

  nPrintf("Si no quedaron tareas pendientes, felicitaciones!  ");
  nPrintf("Su tarea paso todos los tests\n");

  return 0;
}
