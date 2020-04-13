/* Sample HAL for the FTDI FT80x EVE */

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <arch/yaz180.h>
#include <arch/yaz180/i2c.h>

#include "GPU_HAL.h"

/*******************************************************************************/
/*******************************************************************************/

/* API to initialise the I2C interface and enable the Interrupt */
ft_bool_t  GPU_HAL_Open(GPU_HAL_Context_t *host)
{
    i2c_reset( host->port);

    if ( host->port == I2C1_PORT)
        i2c_interrupt_attach( host->port, &i2c1_isr);
    if ( host->port == I2C2_PORT)
        i2c_interrupt_attach( host->port, &i2c2_isr);

    i2c_initialise( host->port);
    i2c_set_speed( host->port, I2C_SPEED_FAST);

    host->status = GPU_HAL_OPENED;
    host->ft_cmd_fifo_wp = 0;

    return TRUE;
}

ft_void_t  GPU_HAL_Fast(GPU_HAL_Context_t *host)
{
    i2c_set_speed( host->port, I2C_SPEED_PLAID);
}

ft_void_t  GPU_HAL_Close(GPU_HAL_Context_t *host)
{
    host->status = GPU_HAL_CLOSED;
    i2c_reset( host->port);
}

/*******************************************************************************/
/*******************************************************************************/

ft_void_t GPU_HostCommand(GPU_HAL_Context_t *host, ft_const_uint8_t cmd)
{
    static ft_uint8_t writeBuffer[3];

    writeBuffer[0] = cmd;
    writeBuffer[1] = 0;
    writeBuffer[2] = 0;
    i2c_write( host->port, GPU_ADDRESS, writeBuffer, 3, I2C_STOP|I2C_MODE_BUFFER );
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
   GPU_HostCommand(host,GPU_CORE_RESET);
}

/*******************************************************************************/
/*******************************************************************************/

ft_uint8_t  GPU_HAL_Rd8(GPU_HAL_Context_t *host, ft_const_uint32_t addr)
{
    ft_uint8_t readAddress[3];
    ft_uint8_t readByte;

    readAddress[0] = ((ft_uint8_t)(addr >> 16) & 0x3F);// we just need the bottom 6 bits.
    readAddress[1] = (ft_uint8_t)(addr >> 8);           // the middle 8 bits
    readAddress[2] = (ft_uint8_t)(addr);                // the lowest 8 bits
    i2c_write( host->port, GPU_ADDRESS, readAddress, 3, I2C_RESTART|I2C_MODE_BUFFER );

    i2c_read_set( host->port, GPU_ADDRESS, &readByte, 1, I2C_STOP|I2C_MODE_BUFFER );
    i2c_read_get( host->port, GPU_ADDRESS, 1);
    return readByte;
}

ft_uint16_t GPU_HAL_Rd16(GPU_HAL_Context_t *host, ft_const_uint32_t addr)
{
    ft_uint8_t readAddress[3];
    ft_uint16_t readWord;

    readAddress[0] = ((ft_uint8_t)(addr >> 16) & 0x3F); // we just need the bottom 6 bits.
    readAddress[1] = (ft_uint8_t)(addr >> 8);           // the middle 8 bits
    readAddress[2] = (ft_uint8_t)(addr);                // the lowest 8 bits
    i2c_write( host->port, GPU_ADDRESS, readAddress, 3, I2C_RESTART|I2C_MODE_BUFFER );

    i2c_read_set( host->port, GPU_ADDRESS, &readWord, 2, I2C_STOP|I2C_MODE_BUFFER );
    i2c_read_get( host->port, GPU_ADDRESS, 1);
    return readWord;
}

ft_uint32_t GPU_HAL_Rd32(GPU_HAL_Context_t *host, ft_const_uint32_t addr)
{
    ft_uint8_t readAddress[3];
    ft_uint32_t readLong;

    readAddress[0] = ((ft_uint8_t)(addr >> 16) & 0x3F); // we just need the bottom 6 bits.
    readAddress[1] = (ft_uint8_t)(addr >> 8);           // the middle 8 bits
    readAddress[2] = (ft_uint8_t)(addr);                // the lowest 8 bits
    i2c_write( host->port, GPU_ADDRESS, readAddress, 3, I2C_RESTART|I2C_MODE_BUFFER );

    i2c_read_set( host->port, GPU_ADDRESS, &readLong, 4, I2C_STOP|I2C_MODE_BUFFER );
    i2c_read_get( host->port, GPU_ADDRESS, 1);
    return readLong;
}

ft_void_t GPU_HAL_Wr8(GPU_HAL_Context_t *host, ft_const_uint32_t addr, ft_const_uint8_t v)
{
    static ft_uint8_t writeBuffer[4];

    writeBuffer[0] = (((ft_uint8_t)(addr >> 16) & 0x3F) | 0x80);// we just need the bottom 6 bits and add the write bit.
    writeBuffer[1] = (ft_uint8_t)(addr >> 8);           // the middle 8 bits
    writeBuffer[2] = (ft_uint8_t)(addr);                // the lowest 8 bits
    writeBuffer[3] = v;
    i2c_write( host->port, GPU_ADDRESS, writeBuffer, 4, I2C_STOP|I2C_MODE_BUFFER );
}

ft_void_t GPU_HAL_Wr16(GPU_HAL_Context_t *host, ft_const_uint32_t addr, ft_const_uint16_t v)
{
    static ft_uint8_t writeBuffer[5];

    writeBuffer[0] = (((ft_uint8_t)(addr >> 16) & 0x3F) | 0x80);// we just need the bottom 6 bits and add the write bit.
    writeBuffer[1] = (ft_uint8_t)(addr >> 8);           // the middle 8 bits
    writeBuffer[2] = (ft_uint8_t)(addr);                // the lowest 8 bits
    writeBuffer[3] = (ft_uint8_t)(v);                   // the lowest 8 bits
    writeBuffer[4] = (ft_uint8_t)(v >> 8);              // the upper 8 bits
    i2c_write( host->port, GPU_ADDRESS, writeBuffer, 5, I2C_STOP|I2C_MODE_BUFFER );
}

ft_void_t GPU_HAL_Wr32(GPU_HAL_Context_t *host, ft_const_uint32_t addr, ft_const_uint32_t v)
{
    static ft_uint8_t writeBuffer[7];

    writeBuffer[0] = (((ft_uint8_t)(addr >> 16) & 0x3F) | 0x80);// we just need the bottom 6 bits and add the write bit.
    writeBuffer[1] = (ft_uint8_t)(addr >> 8);           // the middle 8 bits
    writeBuffer[2] = (ft_uint8_t)(addr);                // the lowest 8 bits
    writeBuffer[3] = (ft_uint8_t)(v);                   // the lowest 8 bits
    writeBuffer[4] = (ft_uint8_t)(v >> 8);              // the lower middle 8 bits
    writeBuffer[5] = (ft_uint8_t)(v >> 16);             // the upper middle 8 bits
    writeBuffer[6] = (ft_uint8_t)(v >> 24);             // the upper 8 bits
    i2c_write( host->port, GPU_ADDRESS, writeBuffer, 7, I2C_STOP|I2C_MODE_BUFFER );
}

/*******************************************************************************/
/*******************************************************************************/

ft_void_t GPU_HAL_RdMem(GPU_HAL_Context_t *host, ft_uint32_t addr, ft_uint8_t *buffer, ft_const_uint16_t length)
{
    ft_uint8_t readAddress[3];
    ft_uint8_t *bufIndex;

    if(length > I2C_RX_SENTENCE) return; // maximum Rx sentence length is 68 Bytes

    bufIndex = buffer;

    readAddress[0] = ((ft_uint8_t)(addr >> 16) & 0x3F); // we just need the bottom 6 bits.
    readAddress[1] = (ft_uint8_t)(addr >> 8);           // the middle 8 bits
    readAddress[2] = (ft_uint8_t)(addr);                // the lowest 8 bits
    i2c_write( host->port, GPU_ADDRESS, readAddress, 3, I2C_STOP|I2C_MODE_BUFFER );

    i2c_read_set( host->port, GPU_ADDRESS, bufIndex, (uint8_t)length, I2C_RESTART|I2C_MODE_BUFFER );
    i2c_read_get( host->port, GPU_ADDRESS, (uint8_t)length);
}

ft_void_t GPU_HAL_WrMem(GPU_HAL_Context_t *host, ft_uint32_t addr, ft_const_uint8_t *buffer, ft_const_uint16_t length)
{
    static ft_uint8_t writeBuffer[I2C_TX_SENTENCE];
    ft_uint8_t *bufIndex;
    ft_uint16_t lengthRemaining;
    

    writeBuffer[0] = (((ft_uint8_t)(addr >> 16) & 0x3F) | 0x80);// we just need the bottom 6 bits and add the write bit.
    writeBuffer[1] = (ft_uint8_t)(addr >> 8);           // the middle 8 bits
    writeBuffer[2] = (ft_uint8_t)(addr);                // the lowest 8 bits

    bufIndex = (ft_uint8_t *)buffer;
    lengthRemaining = (ft_uint16_t)length;

    while( lengthRemaining > I2C_TX_SENTENCE-3 ) {
        i2c_available( host->port );
        memcpy( &writeBuffer[3], bufIndex, I2C_TX_SENTENCE-3 ); // do buffer copy only after some delay
        i2c_write( host->port, GPU_ADDRESS, writeBuffer, I2C_TX_SENTENCE, I2C_RESTART|I2C_MODE_BUFFER );
        bufIndex += I2C_TX_SENTENCE-3;
        lengthRemaining -= I2C_TX_SENTENCE-3;
    }

    if( lengthRemaining > 0 ) {
        i2c_available( host->port ); 
        memcpy( &writeBuffer[3], bufIndex, lengthRemaining );   // do buffer copy only after some delay
        i2c_write( host->port, GPU_ADDRESS, writeBuffer, (uint8_t)(lengthRemaining+3), I2C_STOP|I2C_MODE_BUFFER );
    }
}

/*******************************************************************************/
/*******************************************************************************/

ft_void_t GPU_HAL_Updatecmdfifo(GPU_HAL_Context_t *host, ft_const_uint16_t count)
{
    host->ft_cmd_fifo_wp  = ( host->ft_cmd_fifo_wp + count) & 0xFFF; // 4095 byte fifo.

    //4 byte alignment
    host->ft_cmd_fifo_wp = ( host->ft_cmd_fifo_wp + 3) & 0xffc;
    GPU_HAL_Wr16( host,REG_CMD_WRITE,host->ft_cmd_fifo_wp );
}

ft_uint16_t GPU_Cmdfifo_Freespace(GPU_HAL_Context_t *host)
{
    ft_uint16_t fullness;

    fullness = ( host->ft_cmd_fifo_wp - GPU_HAL_Rd16(host, REG_CMD_READ)) & 0xFFF; // 4095 byte fifo.;

    return( (CMD_FIFO_SIZE - 4) - fullness);
}

ft_void_t GPU_HAL_CheckCmdBuffer(GPU_HAL_Context_t *host, ft_const_uint16_t count)
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

ft_void_t GPU_HAL_ResetCmdFifo(GPU_HAL_Context_t *host)
{
    host->ft_cmd_fifo_wp = 0;
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

/*******************************************************************************/
/*******************************************************************************/

ft_void_t GPU_HAL_WrCmd32(GPU_HAL_Context_t *host, ft_const_uint32_t cmd)
{
    GPU_HAL_CheckCmdBuffer(host,sizeof(cmd));
    GPU_HAL_Wr32(host, RAM_CMD + host->ft_cmd_fifo_wp, cmd);
    GPU_HAL_Updatecmdfifo(host,sizeof(cmd));
}

ft_void_t GPU_HAL_WrCmdBuf(GPU_HAL_Context_t *host, ft_const_uint8_t *buffer, ft_const_uint16_t count)
{
    ft_uint16_t lengthRemaining = (ft_uint16_t)count;
    ft_uint16_t offset = 0;
    ft_uint16_t length;

    do{
        length = GPU_Cmdfifo_Freespace(host); // this checks the buffer has enough space
        if( length > lengthRemaining )
            length = lengthRemaining;

        GPU_HAL_WrMem(host, GPU_ADDRESS, &buffer[offset], length);

        GPU_HAL_Updatecmdfifo(host, length);

        offset += length;
        lengthRemaining -= length;

        if( lengthRemaining != 0 )
            GPU_HAL_WaitCmdfifo_empty(host); // this waits to make sure we're minimising I2C transfers.

    }while( lengthRemaining > 0 );
}

#if 0
ft_uint8_t GPU_HAL_TransferString(GPU_HAL_Context_t *host, ft_const_char8_t *string)
{
    ft_uint16_t length = (ft_uint16_t)strlen((const char *)string);

    spiMultiByteTx( (ft_uint8_t *)string, length);

    //Append one null as ending flag
    return GPU_HAL_Transfer8(host,0);
}
#endif

/*******************************************************************************/
/*******************************************************************************/

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
