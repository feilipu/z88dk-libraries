/* Sample HAL for the FTDI FT80x EVE */

#include "GPU_HAL.h"

/* Global used for HAL management */
GPU_HAL_Context_t host;
GPU_HAL_Context_t *phost;

/* API to initialise the SPI interface and enable the Pin 2 Interrupt */
ft_bool_t  GPU_HAL_Open(GPU_HAL_Context_t *host)
{
    spiBegin(Gameduino2);                   // enable the Gameduino V2 SPI interface
    spiSetClockDivider(SPI_CLOCK_DIV8);     // SPI at 1/8 SYSCLK speed (3072kHz on Goldilocks, 2000kHz on Uno)
    spiSetBitOrder(SPI_MSBFIRST);
    spiSetDataMode(SPI_MODE0);              // Enable SPI function in mode 0, CPOL=0 CPHA=0

#if defined(__AVR_ATmega168__) || defined(__AVR_ATmega328P__) || defined(__AVR_ATmega644P__) || defined(__AVR_ATmega1284P__) || defined(__AVR_ATmega324PA__) || defined(__AVR_ATmega644PA__)
    DDRD &= ~_BV(DDD2);                     // Set PD2 (Arduino 2) as an input. Same pin on Goldilocks as in Uno.
    PORTD |= _BV(PORTD2);                   // Enable the pull-up on PD2.
    EICRA &= ~_BV(ISC00);                   // clear the ISC00 pin to enable interrupt on low level
    EIMSK |= _BV(INT0);                     // enable interrupt INT0

    // Semaphores are useful to stop a thread proceeding, where it should be stopped because it is waiting for an interrupt.
    if( host->xINT0Semaphore == NULL )                  // Check to see if the INT0 semaphore has not been created.
    {
        host->xINT0Semaphore = xSemaphoreCreateMutex(); // mutex semaphore for INTO. The ISR(INT0) will "give" the semaphore as needed.
    }
#endif

    host->status = GPU_HAL_OPENED;
    host->ft_cmd_fifo_wp = 0;

    return TRUE;
}

ft_void_t  GPU_HAL_Fast(GPU_HAL_Context_t *host)
{
    spiSetClockDivider(SPI_CLOCK_DIV2);     // SPI at 1/2 SYSCLK speed (12.288MHz on GA, 11.059MHz on Goldilocks, 8.000MHz on Uno)
}

ft_void_t  GPU_HAL_Close(GPU_HAL_Context_t *host)
{
#if defined(__AVR_ATmega168__) || defined(__AVR_ATmega328P__) || defined(__AVR_ATmega644P__) || defined(__AVR_ATmega1284P__) || defined(__AVR_ATmega324PA__) || defined(__AVR_ATmega644PA__)
    DDRD &= ~_BV(DDD2);                     // set PD2 (Arduino 2) as an input. Same pin on Goldilocks as in Uno.
    PORTD &= ~_BV(PORTD2);                  // disable the pull-up on PD2.
    EICRA &= ~_BV(ISC00);                   // clear the ISC00 pin to enable interrupt on low level
    EIMSK &= ~_BV(INT0);                    // disable interrupt INT0

    if( host->xINT0Semaphore != NULL )                  // Check to see if the INT0 semaphore has been created.
    {
        vSemaphoreDelete( host->xINT0Semaphore );       // delete binary semaphore for INTO.
    }
#endif

    host->status = GPU_HAL_CLOSED;
    spiEnd();
}

/*******************************************************************************/
/* Interrupt vector for Arduino Pin 2 - INT0 is the signal pin from Gameduino2 */
/* An interrupt is triggered by a logical low level.                           */
/*******************************************************************************/
#if defined(__AVR_ATmega168__) || defined(__AVR_ATmega328P__)  || defined(__AVR_ATmega644P__)|| defined(__AVR_ATmega1284P__) || defined(__AVR_ATmega324PA__) || defined(__AVR_ATmega644PA__)
ISR( INT0_vect )
{
    static BaseType_t xHigherPriorityTaskWoken __attribute__ ((unused));
    if( ( host.xINT0Semaphore ) != NULL )
        xSemaphoreGiveFromISR( host.xINT0Semaphore, &xHigherPriorityTaskWoken );    // make the INT0 semaphore available to which ever task is waiting for an interrupt.
}
#endif

/*******************************************************************************/
/*These (should be private) APIs for reading/writing transfer continuously with small buffer system*/
/*******************************************************************************/

ft_void_t  GPU_HAL_StartTransfer(GPU_HAL_Context_t *host, GPU_TRANSFERDIR_T rw, ft_const_uint32_t addr)
{
    if(rw == GPU_READ){

        ft_uint8_t Transfer_Array[4];

        /* Compose the read packet */
        Transfer_Array[0] = ((ft_uint8_t)(addr >> 16) & 0x3F);  // we just need the bottom 6 bits (being the upper 6 of the 22 bits of address)
        Transfer_Array[1] = (ft_uint8_t)(addr >> 8);            // the middle 8 bits
        Transfer_Array[2] = (ft_uint8_t)(addr);                 // the lowest 8 bits
        Transfer_Array[3] = 0;                                  // Dummy Read byte

        spiSelect (Gameduino2);
        spiMultiByteTx(Transfer_Array, 4);

        host->status = GPU_HAL_READING;

    }else if(rw == GPU_WRITE){

        ft_uint8_t Transfer_Array[3];

        /* Compose the write packet */
        Transfer_Array[0] = (((ft_uint8_t)(addr >> 16) & 0x3F) | 0x80);// we just need the bottom 6 bits and add the write bit.
        Transfer_Array[1] = (ft_uint8_t)(addr >> 8);            // the middle 8 bits
        Transfer_Array[2] = (ft_uint8_t)(addr);                 // the lowest 8 bits

        spiSelect (Gameduino2);
        spiMultiByteTx(Transfer_Array, 3);

        host->status = GPU_HAL_WRITING;
    }
}

ft_void_t  GPU_HAL_EndTransfer(GPU_HAL_Context_t *host)
{
    spiDeselect (Gameduino2);
    host->status = GPU_HAL_OPENED;
}

ft_uint8_t GPU_HAL_Transfer8(GPU_HAL_Context_t *host, ft_const_uint8_t value)
{
    return spiTransfer( value );
}


ft_uint16_t GPU_HAL_Transfer16(GPU_HAL_Context_t *host, ft_const_uint16_t value)
{
    ft_uint16_t retVal = 0;
    if (host->status == GPU_HAL_WRITING){
        GPU_HAL_Transfer8(host,value & 0xFF);    //LSB first
        GPU_HAL_Transfer8(host,(value >> 8) & 0xFF);
    }else{
        retVal = GPU_HAL_Transfer8(host,0);
        retVal |= (ft_uint16_t)GPU_HAL_Transfer8(host,0) << 8;
    }
    return retVal;
}

ft_uint32_t  GPU_HAL_Transfer32(GPU_HAL_Context_t *host, ft_const_uint32_t value)
{
    ft_uint32_t retVal = 0;
    if (host->status == GPU_HAL_WRITING){
        GPU_HAL_Transfer16(host,value & 0xFFFF);    //LSW first
        GPU_HAL_Transfer16(host,(value >> 16) & 0xFFFF);
    }else{
        retVal = GPU_HAL_Transfer16(host,0);
        retVal |= (ft_uint32_t)GPU_HAL_Transfer16(host,0) << 16;
    }
    return retVal;
}

ft_uint8_t  GPU_HAL_Rd8(GPU_HAL_Context_t *host, ft_const_uint32_t addr)
{
    ft_uint8_t value;
    GPU_HAL_StartTransfer(host,GPU_READ,addr);
    value = GPU_HAL_Transfer8(host,0);
    GPU_HAL_EndTransfer(host);
    return value;
}
ft_uint16_t GPU_HAL_Rd16(GPU_HAL_Context_t *host, ft_const_uint32_t addr)
{
    ft_uint16_t value;
    GPU_HAL_StartTransfer(host,GPU_READ,addr);
    value = GPU_HAL_Transfer16(host,0);
    GPU_HAL_EndTransfer(host);
    return value;
}
ft_uint32_t GPU_HAL_Rd32(GPU_HAL_Context_t *host, ft_const_uint32_t addr)
{
    ft_uint32_t value;
    GPU_HAL_StartTransfer(host,GPU_READ,addr);
    value = GPU_HAL_Transfer32(host,0);
    GPU_HAL_EndTransfer(host);
    return value;
}

ft_void_t GPU_HAL_Wr8(GPU_HAL_Context_t *host, ft_const_uint32_t addr, ft_const_uint8_t v)
{
    GPU_HAL_StartTransfer(host,GPU_WRITE,addr);
    GPU_HAL_Transfer8(host,v);
    GPU_HAL_EndTransfer(host);
}
ft_void_t GPU_HAL_Wr16(GPU_HAL_Context_t *host, ft_const_uint32_t addr, ft_const_uint16_t v)
{
    GPU_HAL_StartTransfer(host,GPU_WRITE,addr);
    GPU_HAL_Transfer16(host,v);
    GPU_HAL_EndTransfer(host);
}
ft_void_t GPU_HAL_Wr32(GPU_HAL_Context_t *host, ft_const_uint32_t addr, ft_const_uint32_t v)
{
    GPU_HAL_StartTransfer(host,GPU_WRITE,addr);
    GPU_HAL_Transfer32(host,v);
    GPU_HAL_EndTransfer(host);
}

/*******************************************************************************/
/*******************************************************************************/


/*The public APIs for writing transfer continuously only*/
ft_void_t  GPU_HAL_StartCmdTransfer(GPU_HAL_Context_t *host,GPU_TRANSFERDIR_T rw)
{
    GPU_HAL_StartTransfer(host, rw, host->ft_cmd_fifo_wp + RAM_CMD);
}

ft_uint32_t  GPU_HAL_TransferCmd(GPU_HAL_Context_t *host, ft_const_uint32_t cmd)
{
    return GPU_HAL_Transfer32(phost, cmd);
}

ft_void_t  GPU_HAL_EndCmdTransfer(GPU_HAL_Context_t *host)
{
    GPU_HAL_EndTransfer(host);
}

ft_uint8_t GPU_HAL_TransferString(GPU_HAL_Context_t *host, ft_const_char8_t *string)
{
    ft_uint16_t length = (ft_uint16_t)strlen((const char *)string);

    spiMultiByteTx( (ft_uint8_t *)string, length);

    //Append one null as ending flag
    return GPU_HAL_Transfer8(host,0);
}

ft_void_t GPU_HostCommand(GPU_HAL_Context_t *host, ft_const_uint8_t cmd)
{
    ft_uint8_t Transfer_Array[3];

    Transfer_Array[0] = cmd;
    Transfer_Array[1] = 0;
    Transfer_Array[2] = 0;

    spiSelect (Gameduino2);
    spiMultiByteTx(Transfer_Array, 3);
    spiDeselect (Gameduino2);
}

ft_void_t GPU_ClockSelect(GPU_HAL_Context_t *host, const GPU_PLL_SOURCE_T pllsource)
{
   GPU_HostCommand(host,pllsource);
}
ft_void_t GPU_PLL_FreqSelect(GPU_HAL_Context_t *host, const GPU_PLL_FREQ_T freq)
{
   GPU_HostCommand(host,freq);
}
ft_void_t GPU_PowerModeSwitch(GPU_HAL_Context_t *host, const GPU_POWER_MODE_T pwrmode)
{
   GPU_HostCommand(host,pwrmode);
}
ft_void_t GPU_CoreReset(GPU_HAL_Context_t *host)
{
   GPU_HostCommand(host,0x68);
}

ft_void_t GPU_HAL_Updatecmdfifo(GPU_HAL_Context_t *host, ft_const_uint16_t count)
{
    host->ft_cmd_fifo_wp  = (host->ft_cmd_fifo_wp + count) & 0xFFF; // 4095 byte fifo.

    //4 byte alignment
    host->ft_cmd_fifo_wp = (host->ft_cmd_fifo_wp + 3) & 0xffc;
    GPU_HAL_Wr16( host,REG_CMD_WRITE,host->ft_cmd_fifo_wp );
}

ft_uint16_t GPU_Cmdfifo_Freespace(GPU_HAL_Context_t *host)
{
    ft_uint16_t fullness;

    fullness = (host->ft_cmd_fifo_wp - GPU_HAL_Rd16(host, REG_CMD_READ)) & 0xFFF; // 4095 byte fifo.;

    return( (CMD_FIFO_SIZE - 4) - fullness);
}

ft_void_t GPU_HAL_WrCmdBuf(GPU_HAL_Context_t *host, ft_const_uint8_t *buffer, ft_uint16_t count)
{
    ft_uint16_t length = 0;
    ft_uint16_t    offset = 0;

    do{
        length = count;
        if (length > GPU_Cmdfifo_Freespace(host)){
            length = GPU_Cmdfifo_Freespace(host);
        }
        GPU_HAL_CheckCmdBuffer(host, length); // this checks the buffer has enough space

        GPU_HAL_StartCmdTransfer(host, GPU_WRITE);
        spiMultiByteTx(&buffer[offset], length);
        GPU_HAL_EndTransfer(host);

        GPU_HAL_Updatecmdfifo(host, length);

        offset += length;
        count -= length;

        if(count != 0) GPU_HAL_WaitCmdfifo_empty(host); // this waits to make sure we're minimising SPI transfers.

    }while( count > 0 );
}

ft_void_t GPU_HAL_CheckCmdBuffer(GPU_HAL_Context_t *host,ft_const_uint16_t count)
{
   ft_uint16_t getfreespace;
   do{
        getfreespace = GPU_Cmdfifo_Freespace(host);
   }while( getfreespace < count );
}

ft_void_t GPU_HAL_WaitCmdfifo_empty(GPU_HAL_Context_t *host)
{
   while( GPU_HAL_Rd16(host, REG_CMD_READ) != GPU_HAL_Rd16(host, REG_CMD_WRITE) );

   host->ft_cmd_fifo_wp = GPU_HAL_Rd16(host, REG_CMD_WRITE);
}

ft_void_t GPU_HAL_WaitLogo_Finish(GPU_HAL_Context_t *host)
{
    ft_int16_t cmdrdptr,cmdwrptr;

    do{
        cmdrdptr = GPU_HAL_Rd16(host,REG_CMD_READ);
        cmdwrptr = GPU_HAL_Rd16(host,REG_CMD_WRITE);
    }while( (cmdwrptr != cmdrdptr) || (cmdrdptr != 0) );
    host->ft_cmd_fifo_wp = 0;
}

ft_void_t GPU_HAL_ResetCmdFifo(GPU_HAL_Context_t *host)
{
    host->ft_cmd_fifo_wp = 0;
}

ft_void_t GPU_HAL_WrCmd32(GPU_HAL_Context_t *host, ft_const_uint32_t cmd)
{
    GPU_HAL_CheckCmdBuffer(host,sizeof(cmd));
    GPU_HAL_Wr32(host, RAM_CMD + host->ft_cmd_fifo_wp, cmd);
    GPU_HAL_Updatecmdfifo(host,sizeof(cmd));
}

ft_void_t GPU_HAL_RdMem(GPU_HAL_Context_t *host, ft_uint32_t addr, ft_uint8_t *buffer, ft_const_uint16_t length)
{
    GPU_HAL_StartTransfer(host, GPU_READ, addr);
    spiMultiByteRx(buffer, length);
    GPU_HAL_EndTransfer(host);
}

ft_void_t GPU_HAL_WrMem(GPU_HAL_Context_t *host, ft_uint32_t addr , ft_const_uint8_t *buffer, ft_const_uint16_t length)
{
    GPU_HAL_StartTransfer(host, GPU_WRITE, addr);
    spiMultiByteTx(buffer, length);
    GPU_HAL_EndTransfer(host);
}

ft_int32_t GPU_HAL_Dec2ASCII(ft_char8_t *pSrc, ft_int32_t value)
{
    ft_int16_t Length;
    ft_char8_t *pDst;
    ft_uint8_t charval;
    ft_int32_t CurrVal = value,tmpval,i;
    ft_uint8_t tmparray[16];
    ft_uint8_t idx = 0;

    Length = strlen((const char*)pSrc);
    pDst = pSrc + Length;

    if( 0 == value )
    {
        *pDst++ = '0';
        *pDst++ = '\0';
        return 0;
    }

    if( CurrVal < 0 )
    {
        *pDst++ = '-';
        CurrVal = - CurrVal;
    }
    /* insert the value */
    while( CurrVal > 0 )
    {
        tmpval = CurrVal;
        CurrVal /= 10;
        tmpval = tmpval - CurrVal*10;
        charval = '0' + tmpval;
        tmparray[idx++] = charval;
    }

    for( i=0; i<idx; i++ )
    {
        *pDst++ = tmparray[idx - i - 1];
    }
    *pDst++ = '\0';

    return 0;
}
