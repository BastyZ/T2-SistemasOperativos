//
// Created by Basián Inostroza.
//

/*
 * La idea aqui es hacer algo parecido a nSend con dos casos:
 *
 * - Soy el ultimo en enviar el mensaje:
 *     En este caso envío el mensaje al primero, lo despierto con el estado READY,
 *     lo saco si tiene timeout, y retorno el mensaje recibido de la fifoqueue.
 *     Si el anterior cumplio Timeout, su mensaje será null, y no le mando niuna wea.
 *
 * - Soy el primero en enviar mensaje:
 *     Me guardo en la cola del receptor, y en send almaceno el mensaje,
 *     pongo al receptor primero en la ready queue y me duermo con WAIT o WAIT TIMEOUT,
 *     de cumplirse el timeout, retorno null.
 */

/*
 * Al enviar un mensaje, la tarea espera con WAIT_EXCHANGE o WAIT_EXCHANGE_TIMEOUT
 * Vuelta a empezar desde que paso el primer test checkedasd
 */

#include "nSysimp.h"
#include "nSystem.h"

void* nExchange(nTask task, void *msg, int timeout) {
    // Para recibir el mensaje
    void *return_msg;
    nTask sender_task = NULL;

    START_CRITICAL();
    {
        nTask this_task = current_task;
        this_task->exchange_task = task;
        // debería ser el segundo, porque espera que le mande una respuesta
        if (task->exchange_task==this_task && (task->status==WAIT_EXCHANGE || task->status==WAIT_EXCHANGE_TIMEOUT)) {
            //nPrintf("2do:   Soy El 2do\n");
            if (task->status==WAIT_EXCHANGE_TIMEOUT)
                CancelTask(task);
            task->status = READY;
            this_task->status = READY;
            PushTask(ready_queue, task); /* primer lugar en la cola ready */
            //nPrintf("2do:   coloque task en la ready queue\n");
            this_task->exchange_msg = msg;
            // seteado el mensaje,y la proxima tarea en despertar es task
            PutObj(task->exchange_queue, task); /* ultimo en cola exchange */
            //nPrintf("2do:   me coloque en la cola de exchange de task \n");
            //nPrintf("2do:   agarro el mensaje\n");
            return_msg = task->exchange_msg==NULL ? NULL : task->exchange_msg;
            END_CRITICAL();
            return return_msg;
        } else if (task->status==ZOMBIE) {
            nFatalError("nExchange", "El receptor es un ZOMBIE");
        } else {
            //nPrintf("Soy el primero\n");
            // en este punto, soy el primero de la vida, o le paso el mensaje al otro
            // para que lo reciba (siendo 2do)
            this_task->exchange_msg = msg;
            if (timeout >= 0) {
                this_task->status = WAIT_EXCHANGE_TIMEOUT;
                this_task->exchange_is_waiting = TRUE;
                ProgramTask(timeout);
            } else this_task->status = WAIT_EXCHANGE;
            //nPrintf("Primero: voy a hacer push");
            PutObj(task->exchange_queue, task);
            //nPrintf("Primero: chao loh vimoh\n");
            ResumeNextReadyTask();
        }
        if (task->exchange_msg == NULL) {
            END_CRITICAL();
            return NULL;
        }
        // Si la cola de envios está vacia el mensaje es nulo
        // Como el segundo hace push de si mismo, sabemos que el es el primero en la lista
        if (!EmptyFifoQueue(task->exchange_queue)) {
            //nPrintf("Hago get de mi exchange queue\n");
            sender_task = GetObj(task->exchange_queue);
            //nPrintf("Lo logre c: \n");
        }
        //nPrintf("agarro el mensaje\n");
        return_msg = sender_task==NULL ? NULL : sender_task->exchange_msg;
        //nPrintf("lo guardé y termino\n");
    }
    END_CRITICAL();
    return return_msg;
}
