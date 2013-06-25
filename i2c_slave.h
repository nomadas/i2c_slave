/*
 * i2cslave.h
 *
 *  Created on: 21/01/2010
 *      Author: Adrian Enrique Cardenas Campos
 *              
 *
 */

#ifndef I2CSLAVE_H_
#define I2CSLAVE_H_


#include <inttypes.h>
#include <compat/twi.h>
#include <avr/interrupt.h>

//habilita ack en modo escalvo
#define TWI_ON()  {  TWCR=0x45; }
//apaga twi
#define TWI_OFF() {  TWCR=0x00; }
//twi on pero sin ack, es decir que no responde a peticiones del master
#define TWI_PAUSE() { TWCR=0x05; }

// el buffer no puede ser mayor a 255
#define MAX_I2C_BUFFER 32
static unsigned char i2c_slave_buffer[MAX_I2C_BUFFER];
static unsigned char i2c_pointer;
static unsigned char i2c_is_pointer=0;


/*
 * Inicializa el TWI como esclavo y con la direccion establecida
 * Si el bit menos significativo es escrito con 1 habilita el reconocimiento de General Call
 */
void i2c_slave_ini(unsigned char address_gce)
{
 TWAR = address_gce;
 TWI_ON();
}

/*
 *
 */
ISR(TWI_vect)
{
 //LED_ON;
 switch(TW_STATUS)
     {
      // *****  Seccion SLAVE RECEIVER *****

      case TW_SR_SLA_ACK:              // SLA+W recivido, ACK enviado
      case TW_SR_ARB_LOST_SLA_ACK:     // Arbitration lost, SLA+W recivido, ACK enviado
                                 // habilita la recepcion del byte con ACK
	  TWCR = 0b11000101;           // TWINT, TWEA, TWEN, TWIE (libera banderas)

	  i2c_is_pointer = 1;
	  

          break;

      case TW_SR_DATA_ACK:             // SLA+W previo, dato recivido, ACK devuelto

	  if(i2c_is_pointer)           // si es el primer dato despues de direccionado,
	      {
	      i2c_pointer = TWDR;      // es el puntero al dato en buffer
	      i2c_is_pointer = 0;      // el siguiente dato se guarda en el buffer
	      }
	  else {                       // si no, pon el dato en la seccion apuntada previamnete
	      i2c_slave_buffer[i2c_pointer++] = TWDR;
	  }
	  TWCR = 0b11000101;           // TWINT, TWEA, TWEN, TWIE (libera banderas)

	  break;

      case TW_SR_STOP:                 // STOP o REP_START detectado mientras estava en SLV RECEIVE
	                               // reinicia TWI (libera baderas)
	  TWCR = 0b11000101;           // TWINT, TWEA, TWEN, TWIE

	  break;

      // ***** Seccion SLAVE TRANSMITTER *****

      case TW_ST_SLA_ACK:              // SLA+R recivido, ACK enviado
      case TW_ST_ARB_LOST_SLA_ACK:     // Arbitration lost, SLA+R recivido, ACK enviado

	  TWDR = i2c_slave_buffer[i2c_pointer++]; // saca dato del buffer, y prepara para enviarlo (espera ACK)

	  TWCR = 0b11000101;           // TWINT, TWEA, TWEN, TWIE (libera banderas)

	  break;

      case TW_ST_DATA_ACK:             // dato transmitido, ACK devuelto

	  TWDR = i2c_slave_buffer[i2c_pointer++]; // saca dato de la cola, y prepara para enviarlo (espera ACK)

      case TW_ST_DATA_NACK:            // dato transmitido, NACK devuelto (no quiere mas datos el master)

	  TWCR = 0b11000101;           // TWINT, TWEA, TWEN, TWIE (libera banderas)

	  break;

      case TW_BUS_ERROR:               // Error en el bus TWI

	  TWCR = 0b11010101;           // TWINT, TWEA, TWSTO, TWEN, TWIE
	                               // El STOP no se envia al bus, solo afecta al hardware interno.
	                               // el bus es liberado y TWSTO es limpiado
	  break;

      default:
	  TWCR = 0b11000101;           // TWINT, TWEA, TWEN, TWIE (libera banderas)

     }
	 
 if(i2c_pointer >= MAX_I2C_BUFFER)
     i2c_pointer = 0;

}







#endif /* I2CSLAVE_H_ */
