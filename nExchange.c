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
 * Al enviar un mensaje, la tarea espera con WAIT_SEND o WAIT_SEND_TIMEOUT
 */

#include "nSysimp.h"
#include "nSystem.h"

void* nExchange(nTask task, void *msg, int timeout) {
    // Para recibir el mensaje
    void *return_msg;
    nTask sender_task;

    START_CRITICAL();
    {
        nTask this_task = current_task;
        // debería ser el segundo, porque espera que le mande una respuesta
        if (task->status==WAIT_SEND || task->status==WAIT_SEND_TIMEOUT) {
            // Si tiene timeout, cancelo el despertador
            if (task->status==WAIT_COND_TIMEOUT) CancelTask();
            // acá debo primero recibir el mensaje del otro culiao como en nSend
            sender_task = GetTask(this_task->send_queue);
            // TODO verificar que recibo de tesk (?)
            // Si la cola de envios está vacia el mensaje es nulo
            return_msg = sender_task==NULL ? NULL : sender_task->send.msg;
        }
        // en este punto, soy el primero de la vida, o le paso el mensaje al otro para que lo reciba (siendo 2do)
        PutTask(task->send_queue, this_task);
        this_task->send.msg = msg;
        if (timeout > 0) {
            this_task->status = WAIT_SEND_TIMEOUT;
            ProgramTask(timeout);
        } else this_task->status = WAIT_SEND;
        ResumeNextReadyTask();
    }
    END_CRITICAL();

    return return_msg;
}
