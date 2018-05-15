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
        // debería ser el segundo, porque espera que le mande una respuesta
        if (task->status==WAIT_EXCHANGE || task->status==WAIT_EXCHANGE_TIMEOUT) {
            if (task->status==WAIT_EXCHANGE_TIMEOUT)
                CancelTask(task);
            task->status = READY;
            PushTask(ready_queue, task); /* primer lugar en la cola ready */
            this_task->exchange_msg = msg;
            // seteado el mensaje,y la proxima tarea en despertar es task
            PutObj(task->exchange_queue, this_task); /* primero en cola exchange */
        } else if (task->status==ZOMBIE) {
            nFatalError("nExchange", "El receptor es un ZOMBIE");
        } else {
            // en este punto, soy el primero de la vida, o le paso el mensaje al otro
            // para que lo reciba (siendo 2do)
            this_task->exchange_msg = msg;
            if (timeout > 0) {
                this_task->status = WAIT_EXCHANGE_TIMEOUT;
                ProgramTask(timeout);
            } else this_task->status = WAIT_EXCHANGE;
            PushObj(task->exchange_queue, this_task);
            ResumeNextReadyTask();
        }
        // Si la cola de envios está vacia el mensaje es nulo
        // Como el segundo hace push de si mismo, sabemos que el es el primero en la lista
        if (!EmptyFifoQueue(this_task->exchange_queue)) {
            sender_task = GetObj(this_task->exchange_queue);
        }
        return_msg = sender_task==NULL ? NULL : sender_task->exchange_msg;
    }
    END_CRITICAL();

    return return_msg;
}
