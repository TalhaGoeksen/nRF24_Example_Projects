/*
 * 01_Echo_Tx
 * 
 * The nrf24 radio is configured to transmit 1 byte and receive the same byte via
 * ACK + payload from the Rx radio.
 * The data sent is the char received with the UART Rx, when the receiver
 * radio send reply with payload, the transmitter radio will print it on the UART.
 */
#include "project.h"
#include <stdbool.h>

volatile bool irq_flag = false;
volatile bool uart_flag = false;

volatile unsigned char data;

// We got a byte via UART
CY_ISR(UART_Handler)
{
    uart_flag = true;
    data = UART_GetChar();
    isr_UART_ClearPending();
}

// the IRQ pin triggered an interrupt
CY_ISR(IRQ_Handler)
{
    LED_Write(~LED_Read());
    irq_flag = true;
    IRQ_ClearInterrupt();
}

int main(void)
{
    CyGlobalIntEnable;
    
    const uint8_t TX_ADDR[5]= {0xBA, 0xAD, 0xC0, 0xFF, 0xEE};
    
    // Set the interrupts handlers
    isr_IRQ_StartEx(IRQ_Handler);
    isr_UART_StartEx(UART_Handler);
    
    UART_Start();
    UART_PutChar(0x0C);
    UART_PutString("Echo test project\r\n");
    
    nRF24_start();
    nRF24_setTxAddress(TX_ADDR, 5);

    while (1) {
        
        if (true == irq_flag) {
            
            unsigned char received;
            
            // Get and clear the flag that caused the IRQ interrupt
            NrfIRQ flag = nRF24_getIRQFlag();
            nRF24_clearIRQFlag(flag);
            
            // check what caused the interrupt
            switch(flag) {
            case NRF_RX_DR_IRQ: // interrupt caused by receiving data
                UART_PutString("\r\nData received\r\n");
                // Get the data sent from the Rx
                nRF24_getRxPayload(&received, 1);
                // print the received char
                UART_PutChar(received);
                break;
            case NRF_TX_DS_IRQ: //  interrupt caused by sending data
                UART_PutString("\r\nData transmitted\r\n");
                break;
            case NRF_MAX_RT_IRQ:
                UART_PutString("\r\nCan't send data\r\n");
                break;
            default:
                break;
            }
            
            irq_flag = false;
        }
        
        if (true == uart_flag) {
            
            // Transmit the data from UART RX to nrf24 Rx
            nRF24_PTX_Transmit(&data, 1);
            
            uart_flag = false;
        }
        
    }
}

/* [] END OF FILE */