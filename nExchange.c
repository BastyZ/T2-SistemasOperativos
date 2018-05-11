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

