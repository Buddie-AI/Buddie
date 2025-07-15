/******************** (C) COPYRIGHT 2016 STMicroelectronics ********************
* File Name          : LIS2DE12_driver.c
* Author             : MEMS Application Team
* Version            : v1.1
* Date               : 01 Mar 2016  
* Description        : EKSTM32 main file
*
********************************************************************************
* THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*
* THIS SOFTWARE IS SPECIFICALLY DESIGNED FOR EXCLUSIVE USE WITH ST PARTS.
*******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "LIS2DE12_driver.h"

//#include "i2C_mems.h"                                       //[Example]

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/

/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/

/************** Generic Function  *******************/

/*******************************************************************************
* Function Name		: LSM6DS3H_ACC_GYRO_WriteReg
* Description		: Generic Writing function. It must be fullfilled with either
*					: I2C or SPI writing function
* Input				: Register Address, ptr to buffer to be written,
*                                 length of buffer
* Output			: None
* Return			: None
*******************************************************************************/
status_t LIS2DE12_ACC_WriteReg(void *handle, u8_t Reg, u8 *Bufp, u16_t len)
{
  spi_handle_t* spi_dev_handle = (spi_handle_t*)handle;

  u8_t tx_buf[1] = {0x00};
  if (len > 1) {
    tx_buf[0] |= 0x40;
  }
  tx_buf[0] |= Reg;
  gpio_write(spi_dev_handle->spi_cs, 0);
  spi_send_byte(spi_dev_handle->spi_hdl, tx_buf[0]);
  if (len == 1)
  {
    spi_send_byte(spi_dev_handle->spi_hdl, *Bufp);
  }
  else {
    spi_dma_send(spi_dev_handle->spi_hdl, Bufp, len);
  }
  gpio_write(spi_dev_handle->spi_cs, 1);

  return MEMS_SUCCESS;
}

/*******************************************************************************
* Function Name		: LSM6DS3H_ACC_GYRO_ReadReg
* Description		: Generic Reading function. It must be fullfilled with either
*					: I2C or SPI writing function
* Input				: Register Address, ptr to buffer to be read,
*                                 length of buffer
* Output			: None
* Return			: None
*******************************************************************************/
status_t LIS2DE12_ACC_ReadReg(void *handle, u8_t Reg, u8 *Bufp, u16_t len)
{
  // spi way
  
  spi_handle_t* spi_dev_handle = (spi_handle_t*)handle;

  u8_t tx_buf[1] = {0x80 | Reg};
  if (len > 1) {
    tx_buf[0] |= 0xc0;
  }

  gpio_write(spi_dev_handle->spi_cs, 0);
  spi_send_byte(spi_dev_handle->spi_hdl, tx_buf[0]);
  // if (len == 1)
  // {
  //   *Bufp = spi_recv_byte(spi_dev_handle->spi_hdl, NULL);
  // }
  // else {
    spi_dma_recv(spi_dev_handle->spi_hdl, Bufp, len);
  // }
  gpio_write(spi_dev_handle->spi_cs, 1);


  return MEMS_SUCCESS;
}

/**************** Base Function  *******************/

/*******************************************************************************
* Function Name  : LIS2DE12_GetWHO_AM_I
* Description    : Read identification code by WHO_AM_I register
* Input          : Char to empty by Device identification Value
* Output         : None
* Return         : Status [value of FSS]
*******************************************************************************/
status_t LIS2DE12_GetWHO_AM_I(void *handle, u8_t* val){
  
  if( !LIS2DE12_ACC_ReadReg(handle, LIS2DE12_WHO_AM_I, val, 1) )
    return MEMS_ERROR;
  printf("LIS2DE12 who am i: %d", *val);
  return MEMS_SUCCESS;
}

/*******************************************************************************
* Function Name  : LIS2DE12_SetODR
* Description    : Sets LIS2DE12 Output Data Rate
* Input          : Output Data Rate
* Output         : None
* Return         : Status [MEMS_ERROR, MEMS_SUCCESS]
*******************************************************************************/
status_t LIS2DE12_SetODR(void *handle, LIS2DE12_ODR_t ov){
  u8_t value;
  
  if( !LIS2DE12_ACC_ReadReg(handle, LIS2DE12_CTRL_REG1, &value, 1) )
    return MEMS_ERROR;
  
  value &= 0x0f;
  value |= ov<<LIS2DE12_ODR_BIT;
  
  if( !LIS2DE12_ACC_WriteReg(handle, LIS2DE12_CTRL_REG1, &value, 1) )
    return MEMS_ERROR;
  
  return MEMS_SUCCESS;
}
/*******************************************************************************
* Function Name  : LIS2DE12_SetFullScale
* Description    : Sets the LIS2DE12 FullScale
* Input          : LIS2DE12_FULLSCALE_2/LIS2DE12_FULLSCALE_4/LIS2DE12_FULLSCALE_8/LIS2DE12_FULLSCALE_16
* Output         : None
* Return         : Status [MEMS_ERROR, MEMS_SUCCESS]
*******************************************************************************/
status_t LIS2DE12_SetFullScale(void *handle, LIS2DE12_Fullscale_t fs) {
  u8_t value;
  
  if( !LIS2DE12_ACC_ReadReg(handle, LIS2DE12_CTRL_REG4, &value, 1) )
    return MEMS_ERROR;
  
  value &= 0xCF;	
  value |= (fs<<LIS2DE12_FS);
  
  if( !LIS2DE12_ACC_WriteReg(handle, LIS2DE12_CTRL_REG4, &value, 1) )
    return MEMS_ERROR;
  
  return MEMS_SUCCESS;
}

/*******************************************************************************
* Function Name  : LIS2DE12_SetBDU
* Description    : Enable/Disable Block Data Update Functionality
* Input          : ENABLE/DISABLE
* Output         : None
* Return         : Status [MEMS_ERROR, MEMS_SUCCESS]
*******************************************************************************/
status_t LIS2DE12_SetBDU(void *handle, LIS2DE12_ACC_State_t bdu) {
  u8_t value;
  
  if( !LIS2DE12_ACC_ReadReg(handle, LIS2DE12_CTRL_REG4, &value, 1) )
    return MEMS_ERROR;
  
  value &= 0x7F;
  value |= (bdu<<LIS2DE12_BDU);
  
  if( !LIS2DE12_ACC_WriteReg(handle, LIS2DE12_CTRL_REG4, &value, 1) )
    return MEMS_ERROR;
  
  return MEMS_SUCCESS;
}

/*******************************************************************************
* Function Name  : LIS2DE12_GetAccAxesRaw
* Description    : Read the Acceleration Values Output Registers
* Input          : buffer to empity by AxesRaw_t Typedef
* Output         : None
* Return         : Status [MEMS_ERROR, MEMS_SUCCESS]
*******************************************************************************/
status_t LIS2DE12_GetAccAxesRaw(void *handle, u8_t* buff) {
  
  if( !LIS2DE12_ACC_ReadReg(handle, LIS2DE12_OUT_X, (u8_t*) &buff[0], 1) )
    return MEMS_ERROR;
  
  if( !LIS2DE12_ACC_ReadReg(handle, LIS2DE12_OUT_Y, (u8_t*) &buff[1], 1) )
    return MEMS_ERROR;
  
  if( !LIS2DE12_ACC_ReadReg(handle, LIS2DE12_OUT_Z, (u8_t*) &buff[2], 1) )
    return MEMS_ERROR;
  
  return MEMS_SUCCESS; 
}

/**************** Advanced Function  *******************/

/*******************************************************************************
* Function Name  : LIS2DE12_GetStatusAUXBIT
* Description    : Read the AUX status register BIT
* Input          : LIS2DE12_STATUS_AUX_TOR, LIS2DE12_STATUS_AUX_TDA
* Output         : None
* Return         : Status of BIT [MEMS_ERROR, MEMS_SUCCESS]
*******************************************************************************/
status_t LIS2DE12_GetStatusAUXBit(void *handle, u8_t statusBIT, u8_t* val) {
  u8_t value;  
  
  if( !LIS2DE12_ACC_ReadReg(handle, LIS2DE12_STATUS_AUX, &value, 1) )
    return MEMS_ERROR;
  
  if(statusBIT == LIS2DE12_STATUS_AUX_TOR){
    if(value &= LIS2DE12_STATUS_AUX_TOR){     
      *val = MEMS_SET;
      return MEMS_SUCCESS;
    }
    else{  
      *val = MEMS_RESET;
      return MEMS_SUCCESS;
    }   
  }
  
  if(statusBIT == LIS2DE12_STATUS_AUX_TDA){
    if(value &= LIS2DE12_STATUS_AUX_TDA) {     
      *val = MEMS_SET;
      return MEMS_SUCCESS;
    }
    else{  
      *val = MEMS_RESET;
      return MEMS_SUCCESS;
    }   
  }
  
  return MEMS_ERROR;
}

/*******************************************************************************
* Function Name  : LIS2DE12_SetTemperature
* Description    : Sets LIS2DE12 Output Temperature
* Input          : LIS2DE12_TEMP_ENABLE, LIS2DE12_TEMP_DISABLE
* Output         : None
* Note           : For Read Temperature by LIS2DE12_OUT_TEMP,  LIS2DE12_SetBDU 
				   functions must be ENABLE
* Return         : Status [MEMS_ERROR, MEMS_SUCCESS]
*******************************************************************************/
status_t LIS2DE12_SetTemperature(void *handle, LIS2DE12_TempMode_t tempmode){
  u8_t value;
  
  if( !LIS2DE12_ACC_ReadReg(handle, LIS2DE12_TEMP_CFG_REG, &value, 1) )
    return MEMS_ERROR;
  
  value &= 0x3F;
  value |= tempmode<<LIS2DE12_TEMP_EN;
  
  if( !LIS2DE12_ACC_WriteReg(handle, LIS2DE12_TEMP_CFG_REG, &value, 1) )
    return MEMS_ERROR;
  
  return MEMS_SUCCESS;
}

/*******************************************************************************
* Function Name  : LIS2DE12_GetStatusAUX
* Description    : Read the AUX status register
* Input          : Char to empty by status register buffer
* Output         : None
* Return         : Status [MEMS_ERROR, MEMS_SUCCESS]
*******************************************************************************/
status_t LIS2DE12_GetStatusAUX(void *handle, u8_t* val) {
  
  if( !LIS2DE12_ACC_ReadReg(handle, LIS2DE12_STATUS_AUX, val, 1) )
    return MEMS_ERROR;
  
  return MEMS_SUCCESS;  
}

/*******************************************************************************
* Function Name  : LIS2DE12_GetTempRaw
* Description    : Read the Temperature Values 
* Input          : Buffer to empty
* Output         : Temperature Values Registers buffer
* Return         : Status [MEMS_ERROR, MEMS_SUCCESS]
*******************************************************************************/
status_t LIS2DE12_GetTempRaw(void *handle, i8_t* buff) {
  u8_t valueL;
  u8_t valueH;
  
  if( !LIS2DE12_ACC_ReadReg(handle, LIS2DE12_OUT_TEMP_L, &valueL, 1) )
    return MEMS_ERROR;
  
  if( !LIS2DE12_ACC_ReadReg(handle, LIS2DE12_OUT_TEMP_H, &valueH, 1) )
    return MEMS_ERROR;
  
  *buff = (i8_t)( valueH );
  
  return MEMS_SUCCESS;  
}

/*******************************************************************************
* Function Name  : LIS2DE12_SetAxis
* Description    : Enable/Disable LIS2DE12 Axis
* Input          : LIS2DE12_X_ENABLE/DISABLE | LIS2DE12_Y_ENABLE/DISABLE | LIS2DE12_Z_ENABLE/DISABLE
* Output         : None
* Note           : You MUST use all input variable in the argument, as example
* Return         : Status [MEMS_ERROR, MEMS_SUCCESS]
*******************************************************************************/
status_t LIS2DE12_SetAxis(void *handle, LIS2DE12_Axis_t axis) {
  u8_t value;
  
  if( !LIS2DE12_ACC_ReadReg(handle, LIS2DE12_CTRL_REG1, &value, 1) )
    return MEMS_ERROR;
  value &= 0xF8;
  value |= (0x07 & axis);
  
  if( !LIS2DE12_ACC_WriteReg(handle, LIS2DE12_CTRL_REG1, &value, 1) )
    return MEMS_ERROR;   
  
  return MEMS_SUCCESS;
}

/*******************************************************************************
* Function Name  : LIS2DE12_SetSelfTest
* Description    : Set Self Test Modality
* Input          : LIS2DE12_SELF_TEST_DISABLE/ST_0/ST_1
* Output         : None
* Return         : Status [MEMS_ERROR, MEMS_SUCCESS]
*******************************************************************************/
status_t LIS2DE12_SetSelfTest(void *handle, LIS2DE12_SelfTest_t st) {
  u8_t value;
  
  if( !LIS2DE12_ACC_ReadReg(handle, LIS2DE12_CTRL_REG4, &value, 1) )
    return MEMS_ERROR;
  
  value &= 0xF9;
  value |= (st<<LIS2DE12_ST);
  
  if( !LIS2DE12_ACC_WriteReg(handle, LIS2DE12_CTRL_REG4, &value, 1) )
    return MEMS_ERROR;
  
  return MEMS_SUCCESS;
}

/*******************************************************************************
* Function Name  : LIS2DE12_HPFClick
* Description    : Enable/Disable High Pass Filter for click
* Input          : MEMS_ENABLE/MEMS_DISABLE
* Output         : None
* Return         : Status [MEMS_ERROR, MEMS_SUCCESS]
*******************************************************************************/
status_t LIS2DE12_HPFClickEnable(void *handle, LIS2DE12_ACC_State_t hpfe) {
  u8_t value;
  
  if( !LIS2DE12_ACC_ReadReg(handle, LIS2DE12_CTRL_REG2, &value, 1) )
    return MEMS_ERROR;
  
  value &= 0xFB;
  value |= (hpfe<<LIS2DE12_HPCLICK);
  
  if( !LIS2DE12_ACC_WriteReg(handle, LIS2DE12_CTRL_REG2, &value, 1) )
    return MEMS_ERROR;
  
  return MEMS_SUCCESS;
}

/*******************************************************************************
* Function Name  : LIS2DE12_HPFAOI1
* Description    : Enable/Disable High Pass Filter for AOI on INT_1
* Input          : MEMS_ENABLE/MEMS_DISABLE
* Output         : None
* Return         : Status [MEMS_ERROR, MEMS_SUCCESS]
*******************************************************************************/
status_t LIS2DE12_HPFAOI1Enable(void *handle, LIS2DE12_ACC_State_t hpfe) {
  u8_t value;
  
  if( !LIS2DE12_ACC_ReadReg(handle, LIS2DE12_CTRL_REG2, &value, 1) )
    return MEMS_ERROR;
  
  value &= 0xFE;
  value |= (hpfe<<LIS2DE12_HPIS1);
  
  if( !LIS2DE12_ACC_WriteReg(handle, LIS2DE12_CTRL_REG2, &value, 1) )
    return MEMS_ERROR;
  
  return MEMS_SUCCESS;
}

/*******************************************************************************
* Function Name  : LIS2DE12_HPFAOI2
* Description    : Enable/Disable High Pass Filter for AOI on INT_2
* Input          : MEMS_ENABLE/MEMS_DISABLE
* Output         : None
* Return         : Status [MEMS_ERROR, MEMS_SUCCESS]
*******************************************************************************/
status_t LIS2DE12_HPFAOI2Enable(void *handle, LIS2DE12_ACC_State_t hpfe) {
  u8_t value;
  
  if( !LIS2DE12_ACC_ReadReg(handle, LIS2DE12_CTRL_REG2, &value, 1) )
    return MEMS_ERROR;
  
  value &= 0xFD;
  value |= (hpfe<<LIS2DE12_HPIS2);
  
  if( !LIS2DE12_ACC_WriteReg(handle, LIS2DE12_CTRL_REG2, &value, 1) )
    return MEMS_ERROR;
  
  return MEMS_SUCCESS;
}

/*******************************************************************************
* Function Name  : LIS2DE12_SetHPFMode
* Description    : Set High Pass Filter Modality
* Input          : LIS2DE12_HPM_NORMAL_MODE_RES/LIS2DE12_HPM_REF_SIGNAL/
				   LIS2DE12_HPM_NORMAL_MODE/LIS2DE12_HPM_AUTORESET_INT
* Output         : None
* Return         : Status [MEMS_ERROR, MEMS_SUCCESS]
*******************************************************************************/
status_t LIS2DE12_SetHPFMode(void *handle, LIS2DE12_HPFMode_t hpm) {
  u8_t value;
  
  if( !LIS2DE12_ACC_ReadReg(handle, LIS2DE12_CTRL_REG2, &value, 1) )
    return MEMS_ERROR;
  
  value &= 0x3F;
  value |= (hpm<<LIS2DE12_HPM);
  
  if( !LIS2DE12_ACC_WriteReg(handle, LIS2DE12_CTRL_REG2, &value, 1) )
    return MEMS_ERROR;
  
  return MEMS_SUCCESS;
}

/*******************************************************************************
* Function Name  : LIS2DE12_SetHPFCutOFF
* Description    : Set High Pass CUT OFF Freq
* Input          : HPFCF [0,3]
* Output         : None
* Return         : Status [MEMS_ERROR, MEMS_SUCCESS]
*******************************************************************************/
status_t LIS2DE12_SetHPFCutOFF(void *handle, LIS2DE12_HPFCutOffFreq_t hpf) {
  u8_t value;
  
  if (hpf > 3)
    return MEMS_ERROR;
  
  if( !LIS2DE12_ACC_ReadReg(handle, LIS2DE12_CTRL_REG2, &value, 1) )
    return MEMS_ERROR;
  
  value &= 0xCF;
  value |= (hpf<<LIS2DE12_HPCF);
  
  if( !LIS2DE12_ACC_WriteReg(handle, LIS2DE12_CTRL_REG2, &value, 1) )
    return MEMS_ERROR;
  
  return MEMS_SUCCESS;
  
}

/*******************************************************************************
* Function Name  : LIS2DE12_SetFilterDataSel
* Description    : Set Filter Data Selection bypassed or sent to FIFO OUT register
* Input          : MEMS_SET, MEMS_RESET
* Output         : None
* Return         : Status [MEMS_ERROR, MEMS_SUCCESS]
*******************************************************************************/
status_t LIS2DE12_SetFilterDataSel(void *handle, LIS2DE12_ACC_State_t state) {
  u8_t value;
  
  if( !LIS2DE12_ACC_ReadReg(handle, LIS2DE12_CTRL_REG2, &value, 1) )
    return MEMS_ERROR;
  
  value &= 0xF7;
  value |= (state<<LIS2DE12_FDS);
  
  if( !LIS2DE12_ACC_WriteReg(handle, LIS2DE12_CTRL_REG2, &value, 1) )
    return MEMS_ERROR;
  
  return MEMS_SUCCESS;
  
}

/*******************************************************************************
* Function Name  : LIS2DE12_SetInt1Pin
* Description    : Set Interrupt1 pin Function
* Input          :  LIS2DE12_CLICK_ON_PIN_INT1_ENABLE/DISABLE    | LIS2DE12_I1_INT1_ON_PIN_INT1_ENABLE/DISABLE |              
                    LIS2DE12_I1_INT2_ON_PIN_INT1_ENABLE/DISABLE  | LIS2DE12_I1_DRDY1_ON_INT1_ENABLE/DISABLE    |              
                    LIS2DE12_I1_DRDY2_ON_INT1_ENABLE/DISABLE     | LIS2DE12_WTM_ON_INT1_ENABLE/DISABLE         |           
                    LIS2DE12_INT1_OVERRUN_ENABLE/DISABLE  
* example        : SetInt1Pin(LIS2DE12_CLICK_ON_PIN_INT1_ENABLE | LIS2DE12_I1_INT1_ON_PIN_INT1_ENABLE |              
                    LIS2DE12_I1_INT2_ON_PIN_INT1_DISABLE | LIS2DE12_I1_DRDY1_ON_INT1_ENABLE | LIS2DE12_I1_DRDY2_ON_INT1_ENABLE |
                    LIS2DE12_WTM_ON_INT1_DISABLE | LIS2DE12_INT1_OVERRUN_DISABLE   ) 
* Note           : To enable Interrupt signals on INT1 Pad (You MUST use all input variable in the argument, as example)
* Output         : None
* Return         : Status [MEMS_ERROR, MEMS_SUCCESS]
*******************************************************************************/
status_t LIS2DE12_SetInt1Pin(void *handle, LIS2DE12_IntPinConf_t pinConf) {
  u8_t value;
  
  if( !LIS2DE12_ACC_ReadReg(handle, LIS2DE12_CTRL_REG3, &value, 1) )
    return MEMS_ERROR;
  
  value &= 0x00;
  value |= pinConf;
  
  if( !LIS2DE12_ACC_WriteReg(handle, LIS2DE12_CTRL_REG3, &value, 1) )
    return MEMS_ERROR;
  
  return MEMS_SUCCESS;
}

/*******************************************************************************
* Function Name  : LIS2DE12_SetInt2Pin
* Description    : Set Interrupt2 pin Function
* Input          : LIS2DE12_CLICK_ON_PIN_INT2_ENABLE/DISABLE   | LIS2DE12_I2_INT1_ON_PIN_INT2_ENABLE/DISABLE |               
                   LIS2DE12_I2_INT2_ON_PIN_INT2_ENABLE/DISABLE | LIS2DE12_I2_BOOT_ON_INT2_ENABLE/DISABLE |                   
                   LIS2DE12_I2_ACTIVITY_ON_INT2_ENABLE/DISABLE | LIS2DE12_INT_ACTIVE_HIGH/LOW
* example        : LIS2DE12_SetInt2Pin(LIS2DE12_CLICK_ON_PIN_INT2_ENABLE/DISABLE | LIS2DE12_I2_INT1_ON_PIN_INT2_ENABLE/DISABLE |               
                   LIS2DE12_I2_INT2_ON_PIN_INT2_ENABLE/DISABLE | LIS2DE12_I2_BOOT_ON_INT2_ENABLE/DISABLE |                   
                   LIS2DE12_I2_ACTIVITY_ON_INT2_ENABLE | LIS2DE12_INT_ACTIVE_HIGH/LOW)
* Note           : To enable Interrupt signals on INT2 Pad (You MUST use all input variable in the argument, as example)
* Output         : None
* Return         : Status [MEMS_ERROR, MEMS_SUCCESS]
*******************************************************************************/
status_t LIS2DE12_SetInt2Pin(void *handle, LIS2DE12_IntPinConf_t pinConf) {
  u8_t value;
  
  if( !LIS2DE12_ACC_ReadReg(handle, LIS2DE12_CTRL_REG6, &value, 1) )
    return MEMS_ERROR;
  
  value &= 0x00;
  value |= pinConf;
  
  if( !LIS2DE12_ACC_WriteReg(handle, LIS2DE12_CTRL_REG6, &value, 1) )
    return MEMS_ERROR;
  
  return MEMS_SUCCESS;
}                       

/*******************************************************************************
* Function Name  : LIS2DE12_SetClickCFG
* Description    : Set Click Interrupt config Function
* Input          : LIS2DE12_ZD_ENABLE/DISABLE | LIS2DE12_ZS_ENABLE/DISABLE  | LIS2DE12_YD_ENABLE/DISABLE  | 
                   LIS2DE12_YS_ENABLE/DISABLE | LIS2DE12_XD_ENABLE/DISABLE  | LIS2DE12_XS_ENABLE/DISABLE 
* example        : LIS2DE12_SetClickCFG( LIS2DE12_ZD_ENABLE | LIS2DE12_ZS_DISABLE | LIS2DE12_YD_ENABLE | 
                               LIS2DE12_YS_DISABLE | LIS2DE12_XD_ENABLE | LIS2DE12_XS_ENABLE)
* Note           : You MUST use all input variable in the argument, as example
* Output         : None
* Return         : Status [MEMS_ERROR, MEMS_SUCCESS]
*******************************************************************************/
status_t LIS2DE12_SetClickCFG(void *handle, u8_t status) {
  u8_t value;
  
  if( !LIS2DE12_ACC_ReadReg(handle, LIS2DE12_CLICK_CFG, &value, 1) )
    return MEMS_ERROR;
  
  value &= 0xC0;
  value |= status;
  
  if( !LIS2DE12_ACC_WriteReg(handle, LIS2DE12_CLICK_CFG, &value, 1) )
    return MEMS_ERROR;
  
  return MEMS_SUCCESS;
}  

/*******************************************************************************
* Function Name  : LIS2DE12_SetClickTHS
* Description    : Set Click Interrupt threshold
* Input          : Click-click Threshold value [0-127]
* Output         : None
* Return         : Status [MEMS_ERROR, MEMS_SUCCESS]
*******************************************************************************/
status_t LIS2DE12_SetClickTHS(void *handle, u8_t ths) {
  
  if(ths>127)     
    return MEMS_ERROR;
  
  if( !LIS2DE12_ACC_WriteReg(handle, LIS2DE12_CLICK_THS, &ths, 1) )
    return MEMS_ERROR;
  
  return MEMS_SUCCESS;
} 

/*******************************************************************************
* Function Name  : LIS2DE12_SetClickLIMIT
* Description    : Set Click Interrupt Time Limit
* Input          : Click-click Time Limit value [0-127]
* Output         : None
* Return         : Status [MEMS_ERROR, MEMS_SUCCESS]
*******************************************************************************/
status_t LIS2DE12_SetClickLIMIT(void *handle, u8_t t_limit) {
  
  if(t_limit>127)     
    return MEMS_ERROR;
  
  if( !LIS2DE12_ACC_WriteReg(handle, LIS2DE12_TIME_LIMIT, &t_limit, 1) )
    return MEMS_ERROR;
  
  return MEMS_SUCCESS;
} 

/*******************************************************************************
* Function Name  : LIS2DE12_SetClickLATENCY
* Description    : Set Click Interrupt Time Latency
* Input          : Click-click Time Latency value [0-255]
* Output         : None
* Return         : Status [MEMS_ERROR, MEMS_SUCCESS]
*******************************************************************************/
status_t LIS2DE12_SetClickLATENCY(void *handle, u8_t t_latency) {
  
  if( !LIS2DE12_ACC_WriteReg(handle, LIS2DE12_TIME_LATENCY, &t_latency, 1) )
    return MEMS_ERROR;
  
  return MEMS_SUCCESS;
} 

/*******************************************************************************
* Function Name  : LIS2DE12_SetClickWINDOW
* Description    : Set Click Interrupt Time Window
* Input          : Click-click Time Window value [0-255]
* Output         : None
* Return         : Status [MEMS_ERROR, MEMS_SUCCESS]
*******************************************************************************/
status_t LIS2DE12_SetClickWINDOW(void *handle, u8_t t_window) {
  
  if( !LIS2DE12_ACC_WriteReg(handle, LIS2DE12_TIME_WINDOW, &t_window, 1) )
    return MEMS_ERROR;
  
  return MEMS_SUCCESS;
}

/*******************************************************************************
* Function Name  : LIS2DE12_GetClickResponse
* Description    : Get Click Interrupt Response by CLICK_SRC REGISTER
* Input          : char to empty by Click Response Typedef
* Output         : None
* Return         : Status [MEMS_ERROR, MEMS_SUCCESS]
*******************************************************************************/
status_t LIS2DE12_GetClickResponse(void *handle, u8_t* res) {
  u8_t value;
  
  if( !LIS2DE12_ACC_ReadReg(handle, LIS2DE12_CLICK_SRC, &value, 1) ) 
    return MEMS_ERROR;
  
  value &= 0x7F;
  
  if((value & LIS2DE12_IA)==0) {        
    *res = LIS2DE12_NO_CLICK;     
    return MEMS_SUCCESS;
  }
  else {
    if (value & LIS2DE12_DCLICK){
      if (value & LIS2DE12_CLICK_SIGN){
        if (value & LIS2DE12_CLICK_Z) {
          *res = LIS2DE12_DCLICK_Z_N;   
          return MEMS_SUCCESS;
        }
        if (value & LIS2DE12_CLICK_Y) {
          *res = LIS2DE12_DCLICK_Y_N;   
          return MEMS_SUCCESS;
        }
        if (value & LIS2DE12_CLICK_X) {
          *res = LIS2DE12_DCLICK_X_N;   
          return MEMS_SUCCESS;
        }
      }
      else{
        if (value & LIS2DE12_CLICK_Z) {
          *res = LIS2DE12_DCLICK_Z_P;   
          return MEMS_SUCCESS;
        }
        if (value & LIS2DE12_CLICK_Y) {
          *res = LIS2DE12_DCLICK_Y_P;   
          return MEMS_SUCCESS;
        }
        if (value & LIS2DE12_CLICK_X) {
          *res = LIS2DE12_DCLICK_X_P;   
          return MEMS_SUCCESS;
        }
      }       
    }
    else{
      if (value & LIS2DE12_CLICK_SIGN){
        if (value & LIS2DE12_CLICK_Z) {
          *res = LIS2DE12_SCLICK_Z_N;   
          return MEMS_SUCCESS;
        }
        if (value & LIS2DE12_CLICK_Y) {
          *res = LIS2DE12_SCLICK_Y_N;   
          return MEMS_SUCCESS;
        }
        if (value & LIS2DE12_CLICK_X) {
          *res = LIS2DE12_SCLICK_X_N;   
          return MEMS_SUCCESS;
        }
      }
      else{
        if (value & LIS2DE12_CLICK_Z) {
          *res = LIS2DE12_SCLICK_Z_P;   
          return MEMS_SUCCESS;
        }
        if (value & LIS2DE12_CLICK_Y) {
          *res = LIS2DE12_SCLICK_Y_P;   
          return MEMS_SUCCESS;
        }
        if (value & LIS2DE12_CLICK_X) {
          *res = LIS2DE12_SCLICK_X_P;   
          return MEMS_SUCCESS;
        }
      }
    }
  }
  return MEMS_ERROR;
} 

/*******************************************************************************
* Function Name  : LIS2DE12_Int1LatchEnable
* Description    : Enable Interrupt 1 Latching function
* Input          : ENABLE/DISABLE
* Output         : None
* Return         : Status [MEMS_ERROR, MEMS_SUCCESS]
*******************************************************************************/
status_t LIS2DE12_Int1LatchEnable(void *handle, LIS2DE12_ACC_State_t latch) {
  u8_t value;
  
  if( !LIS2DE12_ACC_ReadReg(handle, LIS2DE12_CTRL_REG5, &value, 1) )
    return MEMS_ERROR;
  
  value &= 0xF7;
  value |= latch<<LIS2DE12_LIR_INT1;
  
  if( !LIS2DE12_ACC_WriteReg(handle, LIS2DE12_CTRL_REG5, &value, 1) )
    return MEMS_ERROR;
  
  return MEMS_SUCCESS;
}

/*******************************************************************************
* Function Name  : LIS2DE12_ResetInt1Latch
* Description    : Reset Interrupt 1 Latching function
* Input          : None
* Output         : None
* Return         : Status [MEMS_ERROR, MEMS_SUCCESS]
*******************************************************************************/
status_t LIS2DE12_ResetInt1Latch(void *handle) {
  u8_t value;
  
  if( !LIS2DE12_ACC_ReadReg(handle, LIS2DE12_INT1_SRC, &value, 1) )
    return MEMS_ERROR;
  
  return MEMS_SUCCESS;
}

/*******************************************************************************
* Function Name  : LIS2DE12_Int2LatchEnable
* Description    : Enable Interrupt 2 Latching function
* Input          : ENABLE/DISABLE
* Output         : None
* Return         : Status [MEMS_ERROR, MEMS_SUCCESS]
*******************************************************************************/
status_t LIS2DE12_Int2LatchEnable(void *handle, LIS2DE12_ACC_State_t latch) {
  u8_t value;
  
  if( !LIS2DE12_ACC_ReadReg(handle, LIS2DE12_CTRL_REG5, &value, 1) )
    return MEMS_ERROR;
  
  value &= 0xFD;
  value |= latch<<LIS2DE12_LIR_INT2;
  
  if( !LIS2DE12_ACC_WriteReg(handle, LIS2DE12_CTRL_REG5, &value, 1) )
    return MEMS_ERROR;
  
  return MEMS_SUCCESS;
}

/*******************************************************************************
* Function Name  : LIS2DE12_ResetInt2Latch
* Description    : Reset Interrupt 2 Latching function
* Input          : None
* Output         : None
* Return         : Status [MEMS_ERROR, MEMS_SUCCESS]
*******************************************************************************/
status_t LIS2DE12_ResetInt2Latch(void *handle) {
  u8_t value;
  
  if( !LIS2DE12_ACC_ReadReg(handle, LIS2DE12_INT2_SRC, &value, 1) )
    return MEMS_ERROR;
  
  return MEMS_SUCCESS;
}

/*******************************************************************************
* Function Name  : LIS2DE12_SetInt1Configuration
* Description    : Interrupt 1 Configuration (without LIS2DE12_6D_INT)
* Input          : LIS2DE12_INT1_AND/OR | LIS2DE12_INT1_ZHIE_ENABLE/DISABLE | LIS2DE12_INT1_ZLIE_ENABLE/DISABLE...
* Output         : None
* Note           : You MUST use all input variable in the argument, as example
* Return         : Status [MEMS_ERROR, MEMS_SUCCESS]
*******************************************************************************/
status_t LIS2DE12_SetInt1Configuration(void *handle, LIS2DE12_Int1Conf_t ic) {
  u8_t value;
  
  if( !LIS2DE12_ACC_ReadReg(handle, LIS2DE12_INT1_CFG, &value, 1) )
    return MEMS_ERROR;
  
  value &= 0x40; 
  value |= ic;
  
  if( !LIS2DE12_ACC_WriteReg(handle, LIS2DE12_INT1_CFG, &value, 1) )
    return MEMS_ERROR;
  
  return MEMS_SUCCESS;
}

/*******************************************************************************
* Function Name  : LIS2DE12_SetInt2Configuration
* Description    : Interrupt 1 Configuration (without LIS2DE12_6D_INT)
* Input          : LIS2DE12_INT2_AND/OR | LIS2DE12_INT2_ZHIE_ENABLE/DISABLE | LIS2DE12_INT2_ZLIE_ENABLE/DISABLE...
* Output         : None
* Note           : You MUST use all input variable in the argument, as example
* Return         : Status [MEMS_ERROR, MEMS_SUCCESS]
*******************************************************************************/
status_t LIS2DE12_SetInt2Configuration(void *handle, LIS2DE12_Int2Conf_t ic) {
  u8_t value;
  
  if( !LIS2DE12_ACC_ReadReg(handle, LIS2DE12_INT2_CFG, &value, 1) )
    return MEMS_ERROR;
  
  value &= 0x40; 
  value |= ic;
  
  if( !LIS2DE12_ACC_WriteReg(handle, LIS2DE12_INT2_CFG, &value, 1) )
    return MEMS_ERROR;
  
  return MEMS_SUCCESS;
}

/*******************************************************************************
* Function Name  : LIS2DE12_SetInt1Mode
* Description    : Interrupt 1 Configuration mode (OR, 6D Movement, AND, 6D Position)
* Input          : LIS2DE12_INT_MODE_OR, LIS2DE12_INT_MODE_6D_MOVEMENT, LIS2DE12_INT_MODE_AND, 
				   LIS2DE12_INT_MODE_6D_POSITION
* Output         : None
* Return         : Status [MEMS_ERROR, MEMS_SUCCESS]
*******************************************************************************/
status_t LIS2DE12_SetInt1Mode(void *handle, LIS2DE12_IntMode_t int_mode) {
  u8_t value;
  
  if( !LIS2DE12_ACC_ReadReg(handle, LIS2DE12_INT1_CFG, &value, 1) )
    return MEMS_ERROR;
  
  value &= 0x3F; 
  value |= (int_mode<<LIS2DE12_INT_6D);
  
  if( !LIS2DE12_ACC_WriteReg(handle, LIS2DE12_INT1_CFG, &value, 1) )
    return MEMS_ERROR;
  
  return MEMS_SUCCESS;
}

/*******************************************************************************
* Function Name  : LIS2DE12_SetInt2Mode
* Description    : Interrupt 2 Configuration mode (OR, 6D Movement, AND, 6D Position)
* Input          : LIS2DE12_INT_MODE_OR, LIS2DE12_INT_MODE_6D_MOVEMENT, LIS2DE12_INT_MODE_AND, 
				   LIS2DE12_INT_MODE_6D_POSITION
* Output         : None
* Return         : Status [MEMS_ERROR, MEMS_SUCCESS]
*******************************************************************************/
status_t LIS2DE12_SetInt2Mode(void *handle, LIS2DE12_IntMode_t int_mode) {
  u8_t value;
  
  if( !LIS2DE12_ACC_ReadReg(handle, LIS2DE12_INT2_CFG, &value, 1) )
    return MEMS_ERROR;
  
  value &= 0x3F; 
  value |= (int_mode<<LIS2DE12_INT_6D);
  
  if( !LIS2DE12_ACC_WriteReg(handle, LIS2DE12_INT2_CFG, &value, 1) )
    return MEMS_ERROR;
  
  return MEMS_SUCCESS;
}
    
/*******************************************************************************
* Function Name  : LIS2DE12_SetInt16D4DConfiguration
* Description    : 6D, 4D Interrupt 1 Configuration
* Input          : LIS2DE12_INT1_6D_ENABLE, LIS2DE12_INT1_4D_ENABLE, LIS2DE12_INT1_6D_4D_DISABLE
* Output         : None
* Return         : Status [MEMS_ERROR, MEMS_SUCCESS]
*******************************************************************************/
status_t LIS2DE12_SetInt16D4DConfiguration(void *handle, LIS2DE12_INT1_6D_4D_t ic) {
  u8_t value;
  u8_t value2;
  
  if( !LIS2DE12_ACC_ReadReg(handle, LIS2DE12_INT1_CFG, &value, 1) )
    return MEMS_ERROR;
  if( !LIS2DE12_ACC_ReadReg(handle, LIS2DE12_CTRL_REG5, &value2, 1) )
    return MEMS_ERROR;
  
  if(ic == LIS2DE12_INT1_6D_ENABLE){
    value &= 0xBF; 
    value |= (MEMS_ENABLE<<LIS2DE12_INT_6D);
    value2 &= 0xFB; 
    value2 |= (MEMS_DISABLE<<LIS2DE12_D4D_INT1);
  }
  
  if(ic == LIS2DE12_INT1_4D_ENABLE){
    value &= 0xBF; 
    value |= (MEMS_ENABLE<<LIS2DE12_INT_6D);
    value2 &= 0xFB; 
    value2 |= (MEMS_ENABLE<<LIS2DE12_D4D_INT1);
  }
  
  if(ic == LIS2DE12_INT1_6D_4D_DISABLE){
    value &= 0xBF; 
    value |= (MEMS_DISABLE<<LIS2DE12_INT_6D);
    value2 &= 0xFB; 
    value2 |= (MEMS_DISABLE<<LIS2DE12_D4D_INT1);
  }
  
  if( !LIS2DE12_ACC_WriteReg(handle, LIS2DE12_INT1_CFG, &value, 1) )
    return MEMS_ERROR;
  if( !LIS2DE12_ACC_WriteReg(handle, LIS2DE12_CTRL_REG5, &value2, 1) )
    return MEMS_ERROR;
  
  return MEMS_SUCCESS;
}

/*******************************************************************************
* Function Name  : LIS2DE12_SetInt26D4DConfiguration
* Description    : 6D, 4D Interrupt 2 Configuration
* Input          : LIS2DE12_INT2_6D_ENABLE, LIS2DE12_INT2_4D_ENABLE, LIS2DE12_INT2_6D_4D_DISABLE
* Output         : None
* Return         : Status [MEMS_ERROR, MEMS_SUCCESS]
*******************************************************************************/
status_t LIS2DE12_SetInt26D4DConfiguration(void *handle, LIS2DE12_INT2_6D_4D_t ic) {
  u8_t value;
  u8_t value2;
  
  if( !LIS2DE12_ACC_ReadReg(handle, LIS2DE12_INT2_CFG, &value, 1) )
    return MEMS_ERROR;
  if( !LIS2DE12_ACC_ReadReg(handle, LIS2DE12_CTRL_REG5, &value2, 1) )
    return MEMS_ERROR;
  
  if(ic == LIS2DE12_INT2_6D_ENABLE){
    value &= 0xBF; 
    value |= (MEMS_ENABLE<<LIS2DE12_INT_6D);
    value2 &= 0xFE; 
    value2 |= (MEMS_DISABLE<<LIS2DE12_D4D_INT2);
  }
  
  if(ic == LIS2DE12_INT2_4D_ENABLE){
    value &= 0xBF; 
    value |= (MEMS_ENABLE<<LIS2DE12_INT_6D);
    value2 &= 0xFE; 
    value2 |= (MEMS_ENABLE<<LIS2DE12_D4D_INT2);
  }
  
  if(ic == LIS2DE12_INT2_6D_4D_DISABLE){
    value &= 0xBF; 
    value |= (MEMS_DISABLE<<LIS2DE12_INT_6D);
    value2 &= 0xFE; 
    value2 |= (MEMS_DISABLE<<LIS2DE12_D4D_INT2);
  }
  
  if( !LIS2DE12_ACC_WriteReg(handle, LIS2DE12_INT2_CFG, &value, 1 ) )
    return MEMS_ERROR;
  if( !LIS2DE12_ACC_WriteReg(handle, LIS2DE12_CTRL_REG5, &value2, 1) )
    return MEMS_ERROR;
  
  return MEMS_SUCCESS;
}

/*******************************************************************************
* Function Name  : LIS2DE12_Get6DPosition
* Description    : 6D, 4D Interrupt Position Detect
* Input          : Byte to empty by POSITION_6D_t Typedef and the Interrupt to check(INT1/INT2)
* Output         : None
* Return         : Status [MEMS_ERROR, MEMS_SUCCESS]
*******************************************************************************/
status_t LIS2DE12_Get6DPosition(void *handle, u8_t* val, u8_t INT){
  u8_t value;
  
  if( !LIS2DE12_ACC_ReadReg(handle, INT, &value, 1) )
    return MEMS_ERROR;
  
  value &= 0x7F;
  
  switch (value){
  case LIS2DE12_UP_SX:   
    *val = LIS2DE12_UP_SX;    
    break;
  case LIS2DE12_UP_DX:   
    *val = LIS2DE12_UP_DX;    
    break;
  case LIS2DE12_DW_SX:   
    *val = LIS2DE12_DW_SX;    
    break;
  case LIS2DE12_DW_DX:   
    *val = LIS2DE12_DW_DX;    
    break;
  case LIS2DE12_TOP:     
    *val = LIS2DE12_TOP;      
    break;
  case LIS2DE12_BOTTOM:  
    *val = LIS2DE12_BOTTOM;   
    break;
  }
  
  return MEMS_SUCCESS;  
}

/*******************************************************************************
* Function Name  : LIS2DE12_SetInt1Threshold
* Description    : Sets Interrupt 1 Threshold
* Input          : Threshold = [0,31]
* Output         : None
* Return         : Status [MEMS_ERROR, MEMS_SUCCESS]
*******************************************************************************/
status_t LIS2DE12_SetInt1Threshold(void *handle, u8_t ths) {
  if (ths > 127)
    return MEMS_ERROR;
  
  if( !LIS2DE12_ACC_WriteReg(handle, LIS2DE12_INT1_THS, &ths, 1) )
    return MEMS_ERROR;    
  
  return MEMS_SUCCESS;
}

/*******************************************************************************
* Function Name  : LIS2DE12_SetInt1Duration
* Description    : Sets Interrupt 1 Duration
* Input          : Duration value
* Output         : None
* Return         : Status [MEMS_ERROR, MEMS_SUCCESS]
*******************************************************************************/
status_t LIS2DE12_SetInt1Duration(void *handle, LIS2DE12_Int1Conf_t id) {
  
  if (id > 127)
    return MEMS_ERROR;
  
  if( !LIS2DE12_ACC_WriteReg(handle, LIS2DE12_INT1_DURATION, &id, 1) )
    return MEMS_ERROR;
  
  return MEMS_SUCCESS;
}

/*******************************************************************************
* Function Name  : LIS2DE12_SetInt2Threshold
* Description    : Sets Interrupt 2 Threshold
* Input          : Threshold = [0,31]
* Output         : None
* Return         : Status [MEMS_ERROR, MEMS_SUCCESS]
*******************************************************************************/
status_t LIS2DE12_SetInt2Threshold(void *handle, u8_t ths) {
  if (ths > 127)
    return MEMS_ERROR;
  
  if( !LIS2DE12_ACC_WriteReg(handle, LIS2DE12_INT2_THS, &ths, 1) )
    return MEMS_ERROR;    
  
  return MEMS_SUCCESS;
}

/*******************************************************************************
* Function Name  : LIS2DE12_SetInt2Duration
* Description    : Sets Interrupt 2 Duration
* Input          : Duration value
* Output         : None
* Return         : Status [MEMS_ERROR, MEMS_SUCCESS]
*******************************************************************************/
status_t LIS2DE12_SetInt2Duration(void *handle, LIS2DE12_Int2Conf_t id) {
  
  if (id > 127)
    return MEMS_ERROR;
  
  if( !LIS2DE12_ACC_WriteReg(handle, LIS2DE12_INT2_DURATION, &id, 1) )
    return MEMS_ERROR;
  
  return MEMS_SUCCESS;
}

/*******************************************************************************
* Function Name  : LIS2DE12_FIFOModeEnable
* Description    : Sets Fifo Modality
* Input          : LIS2DE12_FIFO_DISABLE, LIS2DE12_FIFO_BYPASS_MODE, LIS2DE12_FIFO_MODE, 
				   LIS2DE12_FIFO_STREAM_MODE, LIS2DE12_FIFO_TRIGGER_MODE
* Output         : None
* Return         : Status [MEMS_ERROR, MEMS_SUCCESS]
*******************************************************************************/
status_t LIS2DE12_FIFOModeEnable(void *handle, LIS2DE12_FifoMode_t fm) {
  u8_t value;  
  
  if(fm == LIS2DE12_FIFO_DISABLE) { 
    if( !LIS2DE12_ACC_ReadReg(handle, LIS2DE12_FIFO_CTRL_REG, &value, 1) )
      return MEMS_ERROR;
    
    value &= 0x1F;
    value |= (LIS2DE12_FIFO_BYPASS_MODE<<LIS2DE12_FM);                     
    
    if( !LIS2DE12_ACC_WriteReg(handle, LIS2DE12_FIFO_CTRL_REG, &value, 1) )           //fifo mode bypass
      return MEMS_ERROR;   
    if( !LIS2DE12_ACC_ReadReg(handle, LIS2DE12_CTRL_REG5, &value, 1) )
      return MEMS_ERROR;
    
    value &= 0xBF;    
    
    if( !LIS2DE12_ACC_WriteReg(handle, LIS2DE12_CTRL_REG5, &value, 1) )               //fifo disable
      return MEMS_ERROR;   
  }

  if(fm == LIS2DE12_FIFO_BYPASS_MODE)   {  
    if( !LIS2DE12_ACC_ReadReg(handle, LIS2DE12_CTRL_REG5, &value, 1) )
      return MEMS_ERROR;
    
    value &= 0xBF;
    value |= MEMS_SET<<LIS2DE12_FIFO_EN;
    
    if( !LIS2DE12_ACC_WriteReg(handle, LIS2DE12_CTRL_REG5, &value, 1) )               //fifo enable
      return MEMS_ERROR;  
    if( !LIS2DE12_ACC_ReadReg(handle, LIS2DE12_FIFO_CTRL_REG, &value, 1) )
      return MEMS_ERROR;
    
    value &= 0x1f;
    value |= (fm<<LIS2DE12_FM);                     //fifo mode configuration
    
    if( !LIS2DE12_ACC_WriteReg(handle, LIS2DE12_FIFO_CTRL_REG, &value, 1) )
      return MEMS_ERROR;
  }
  
  if(fm == LIS2DE12_FIFO_MODE)   {
    if( !LIS2DE12_ACC_ReadReg(handle, LIS2DE12_CTRL_REG5, &value, 1) )
      return MEMS_ERROR;
    
    value &= 0xBF;
    value |= MEMS_SET<<LIS2DE12_FIFO_EN;
    
    if( !LIS2DE12_ACC_WriteReg(handle, LIS2DE12_CTRL_REG5, &value, 1) )               //fifo enable
      return MEMS_ERROR;  
    if( !LIS2DE12_ACC_ReadReg(handle, LIS2DE12_FIFO_CTRL_REG, &value, 1) )
      return MEMS_ERROR;
    
    value &= 0x1f;
    value |= (fm<<LIS2DE12_FM);                      //fifo mode configuration
    
    if( !LIS2DE12_ACC_WriteReg(handle, LIS2DE12_FIFO_CTRL_REG, &value, 1) )
      return MEMS_ERROR;
  }
  
  if(fm == LIS2DE12_FIFO_STREAM_MODE)   {  
    if( !LIS2DE12_ACC_ReadReg(handle, LIS2DE12_CTRL_REG5, &value, 1) )
      return MEMS_ERROR;
    
    value &= 0xBF;
    value |= MEMS_SET<<LIS2DE12_FIFO_EN;
    
    if( !LIS2DE12_ACC_WriteReg(handle, LIS2DE12_CTRL_REG5, &value, 1) )               //fifo enable
      return MEMS_ERROR;   
    if( !LIS2DE12_ACC_ReadReg(handle, LIS2DE12_FIFO_CTRL_REG, &value, 1) )
      return MEMS_ERROR;
    
    value &= 0x1f;
    value |= (fm<<LIS2DE12_FM);                      //fifo mode configuration
    
    if( !LIS2DE12_ACC_WriteReg(handle, LIS2DE12_FIFO_CTRL_REG, &value, 1) )
      return MEMS_ERROR;
  }
  
  if(fm == LIS2DE12_FIFO_TRIGGER_MODE)   {  
    if( !LIS2DE12_ACC_ReadReg(handle, LIS2DE12_CTRL_REG5, &value, 1) )
      return MEMS_ERROR;
    
    value &= 0xBF;
    value |= MEMS_SET<<LIS2DE12_FIFO_EN;
    
    if( !LIS2DE12_ACC_WriteReg(handle, LIS2DE12_CTRL_REG5, &value, 1) )               //fifo enable
      return MEMS_ERROR;    
    if( !LIS2DE12_ACC_ReadReg(handle, LIS2DE12_FIFO_CTRL_REG, &value, 1) )
      return MEMS_ERROR;
    
    value &= 0x1f;
    value |= (fm<<LIS2DE12_FM);                      //fifo mode configuration
    
    if( !LIS2DE12_ACC_WriteReg(handle, LIS2DE12_FIFO_CTRL_REG, &value, 1) )
      return MEMS_ERROR;
  }
  
  return MEMS_SUCCESS;
}

/*******************************************************************************
* Function Name  : LIS2DE12_SetTriggerInt
* Description    : Trigger event liked to trigger signal INT1/INT2
* Input          : LIS2DE12_TRIG_INT1/LIS2DE12_TRIG_INT2
* Output         : None
* Return         : Status [MEMS_ERROR, MEMS_SUCCESS]
*******************************************************************************/
status_t LIS2DE12_SetTriggerInt(void *handle, LIS2DE12_TrigInt_t tr) {
  u8_t value;  
  
  if( !LIS2DE12_ACC_ReadReg(handle, LIS2DE12_FIFO_CTRL_REG, &value, 1) )
    return MEMS_ERROR;
  
  value &= 0xDF;
  value |= (tr<<LIS2DE12_TR); 
  
  if( !LIS2DE12_ACC_WriteReg(handle, LIS2DE12_FIFO_CTRL_REG, &value, 1) )
    return MEMS_ERROR;
  
  return MEMS_SUCCESS;
}

/*******************************************************************************
* Function Name  : LIS2DE12_SetWaterMark
* Description    : Sets Watermark Value
* Input          : Watermark = [0,31]
* Output         : None
* Return         : Status [MEMS_ERROR, MEMS_SUCCESS]
*******************************************************************************/
status_t LIS2DE12_SetWaterMark(void *handle, u8_t wtm) {
  u8_t value;
  
  if(wtm > 31)
    return MEMS_ERROR;  
  
  if( !LIS2DE12_ACC_ReadReg(handle, LIS2DE12_FIFO_CTRL_REG, &value, 1) )
    return MEMS_ERROR;
  
  value &= 0xE0;
  value |= wtm; 
  
  if( !LIS2DE12_ACC_WriteReg(handle, LIS2DE12_FIFO_CTRL_REG, &value, 1) )
    return MEMS_ERROR;
  
  return MEMS_SUCCESS;
}
  
/*******************************************************************************
* Function Name  : LIS2DE12_GetStatusReg
* Description    : Read the status register
* Input          : char to empty by Status Reg Value
* Output         : None
* Return         : Status [MEMS_ERROR, MEMS_SUCCESS]
*******************************************************************************/
status_t LIS2DE12_GetStatusReg(void *handle, u8_t* val) {
  if( !LIS2DE12_ACC_ReadReg(handle, LIS2DE12_STATUS_REG, val, 1) )
    return MEMS_ERROR;
  
  return MEMS_SUCCESS;  
}

/*******************************************************************************
* Function Name  : LIS2DE12_GetStatusBIT
* Description    : Read the status register BIT
* Input          : LIS2DE12_STATUS_REG_ZYXOR, LIS2DE12_STATUS_REG_ZOR, LIS2DE12_STATUS_REG_YOR, LIS2DE12_STATUS_REG_XOR,
                   LIS2DE12_STATUS_REG_ZYXDA, LIS2DE12_STATUS_REG_ZDA, LIS2DE12_STATUS_REG_YDA, LIS2DE12_STATUS_REG_XDA, 
				   LIS2DE12_DATAREADY_BIT
				   val: Byte to be filled with the status bit	
* Output         : status register BIT
* Return         : Status [MEMS_ERROR, MEMS_SUCCESS]
*******************************************************************************/
status_t LIS2DE12_GetStatusBit(void *handle, u8_t statusBIT, u8_t* val) {
  u8_t value;  
  
  if( !LIS2DE12_ACC_ReadReg(handle, LIS2DE12_STATUS_REG, &value, 1) )
    return MEMS_ERROR;
  
  switch (statusBIT){
  case LIS2DE12_STATUS_REG_ZYXOR:     
    if(value &= LIS2DE12_STATUS_REG_ZYXOR){     
      *val = MEMS_SET;
      return MEMS_SUCCESS;
    }
    else{  
      *val = MEMS_RESET;
      return MEMS_SUCCESS;
    }  
  case LIS2DE12_STATUS_REG_ZOR:       
    if(value &= LIS2DE12_STATUS_REG_ZOR){     
      *val = MEMS_SET;
      return MEMS_SUCCESS;
    }
    else{  
      *val = MEMS_RESET;
      return MEMS_SUCCESS;
    }  
  case LIS2DE12_STATUS_REG_YOR:       
    if(value &= LIS2DE12_STATUS_REG_YOR){     
      *val = MEMS_SET;
      return MEMS_SUCCESS;
    }
    else{  
      *val = MEMS_RESET;
      return MEMS_SUCCESS;
    }                                 
  case LIS2DE12_STATUS_REG_XOR:       
    if(value &= LIS2DE12_STATUS_REG_XOR){     
      *val = MEMS_SET;
      return MEMS_SUCCESS;
    }
    else{  
      *val = MEMS_RESET;
      return MEMS_SUCCESS;
    }     
  case LIS2DE12_STATUS_REG_ZYXDA:     
    if(value &= LIS2DE12_STATUS_REG_ZYXDA){     
      *val = MEMS_SET;
      return MEMS_SUCCESS;
    }
    else{  
      *val = MEMS_RESET;
      return MEMS_SUCCESS;
    }   
  case LIS2DE12_STATUS_REG_ZDA:       
    if(value &= LIS2DE12_STATUS_REG_ZDA){     
      *val = MEMS_SET;
      return MEMS_SUCCESS;
    }
    else{  
      *val = MEMS_RESET;
      return MEMS_SUCCESS;
    }   
  case LIS2DE12_STATUS_REG_YDA:       
    if(value &= LIS2DE12_STATUS_REG_YDA){     
      *val = MEMS_SET;
      return MEMS_SUCCESS;
    }
    else{  
      *val = MEMS_RESET;
      return MEMS_SUCCESS;
    }   
  case LIS2DE12_STATUS_REG_XDA:       
    if(value &= LIS2DE12_STATUS_REG_XDA){     
      *val = MEMS_SET;
      return MEMS_SUCCESS;
    }
    else{  
      *val = MEMS_RESET;
      return MEMS_SUCCESS;
    }                                  
    
  }
  return MEMS_ERROR;
}

/*******************************************************************************
* Function Name  : LIS2DE12_GetInt1Src
* Description    : Reset Interrupt 1 Latching function
* Input          : Char to empty by Int1 source value
* Output         : None
* Return         : Status [MEMS_ERROR, MEMS_SUCCESS]
*******************************************************************************/
status_t LIS2DE12_GetInt1Src(void *handle, u8_t* val) {
  
  if( !LIS2DE12_ACC_ReadReg(handle, LIS2DE12_INT1_SRC, val, 1) )
    return MEMS_ERROR;
  
  return MEMS_SUCCESS;
}

/*******************************************************************************
* Function Name  : LIS2DE12_GetInt2Src
* Description    : Reset Interrupt 2 Latching function
* Input          : Char to empty by Int1 source value
* Output         : None
* Return         : Status [MEMS_ERROR, MEMS_SUCCESS]
*******************************************************************************/
status_t LIS2DE12_GetInt2Src(void *handle, u8_t* val) {
  
  if( !LIS2DE12_ACC_ReadReg(handle, LIS2DE12_INT2_SRC, val, 1) )
    return MEMS_ERROR;
  
  return MEMS_SUCCESS;
}

/*******************************************************************************
* Function Name  : LIS2DE12_GetInt1SrcBit
* Description    : Reset Interrupt 1 Latching function
* Input          : statusBIT: LIS2DE12_INT_SRC_IA, LIS2DE12_INT_SRC_ZH, LIS2DE12_INT_SRC_ZL.....
*                  val: Byte to be filled with the status bit
* Output         : None
* Return         : Status of BIT [MEMS_ERROR, MEMS_SUCCESS]
*******************************************************************************/
status_t LIS2DE12_GetInt1SrcBit(void *handle, u8_t statusBIT, u8_t* val) {
  u8_t value;  
   
  if( !LIS2DE12_ACC_ReadReg(handle, LIS2DE12_INT1_SRC, &value, 1) )
      return MEMS_ERROR;
   
  if(statusBIT == LIS2DE12_INT1_SRC_IA){
    if(value &= LIS2DE12_INT1_SRC_IA){     
      *val = MEMS_SET;
      return MEMS_SUCCESS;
    }
    else{  
      *val = MEMS_RESET;
      return MEMS_SUCCESS;
    }  
  }
  
  if(statusBIT == LIS2DE12_INT1_SRC_ZH){
    if(value &= LIS2DE12_INT1_SRC_ZH){     
      *val = MEMS_SET;
      return MEMS_SUCCESS;
    }
    else{  
      *val = MEMS_RESET;
      return MEMS_SUCCESS;
    }  
  }
  
  if(statusBIT == LIS2DE12_INT1_SRC_ZL){
    if(value &= LIS2DE12_INT1_SRC_ZL){     
      *val = MEMS_SET;
      return MEMS_SUCCESS;
    }
    else{  
      *val = MEMS_RESET;
      return MEMS_SUCCESS;
    }  
  }
  
  if(statusBIT == LIS2DE12_INT1_SRC_YH){
    if(value &= LIS2DE12_INT1_SRC_YH){     
      *val = MEMS_SET;
      return MEMS_SUCCESS;
    }
    else{  
      *val = MEMS_RESET;
      return MEMS_SUCCESS;
    }  
  }
  
  if(statusBIT == LIS2DE12_INT1_SRC_YL){
    if(value &= LIS2DE12_INT1_SRC_YL){     
      *val = MEMS_SET;
      return MEMS_SUCCESS;
    }
    else{  
      *val = MEMS_RESET;
      return MEMS_SUCCESS;
    }  
  }
  if(statusBIT == LIS2DE12_INT1_SRC_XH){
    if(value &= LIS2DE12_INT1_SRC_XH){     
      *val = MEMS_SET;
      return MEMS_SUCCESS;
    }
    else{  
      *val = MEMS_RESET;
      return MEMS_SUCCESS;
    }  
  }
  
  if(statusBIT == LIS2DE12_INT1_SRC_XL){
    if(value &= LIS2DE12_INT1_SRC_XL){     
      *val = MEMS_SET;
      return MEMS_SUCCESS;
    }
    else{  
      *val = MEMS_RESET;
      return MEMS_SUCCESS;
    }  
  }
  return MEMS_ERROR;
}

/*******************************************************************************
* Function Name  : LIS2DE12_GetInt2SrcBit
* Description    : Reset Interrupt 2 Latching function
* Input          : statusBIT: LIS2DE12_INT_SRC_IA, LIS2DE12_INT_SRC_ZH, LIS2DE12_INT_SRC_ZL.....
*                  val: Byte to be filled with the status bit
* Output         : None
* Return         : Status of BIT [MEMS_ERROR, MEMS_SUCCESS]
*******************************************************************************/
status_t LIS2DE12_GetInt2SrcBit(void *handle, u8_t statusBIT, u8_t* val) {
  u8_t value;  
   //TODO
  if( !LIS2DE12_ACC_ReadReg(handle, LIS2DE12_INT2_SRC, &value, 1) )
      return MEMS_ERROR;
   
  if(statusBIT == LIS2DE12_INT2_SRC_IA){
    if(value &= LIS2DE12_INT2_SRC_IA){     
      *val = MEMS_SET;
      return MEMS_SUCCESS;
    }
    else{  
      *val = MEMS_RESET;
      return MEMS_SUCCESS;
    }  
  }
  
  if(statusBIT == LIS2DE12_INT2_SRC_ZH){
    if(value &= LIS2DE12_INT2_SRC_ZH){     
      *val = MEMS_SET;
      return MEMS_SUCCESS;
    }
    else{  
      *val = MEMS_RESET;
      return MEMS_SUCCESS;
    }  
  }
  
  if(statusBIT == LIS2DE12_INT2_SRC_ZL){
    if(value &= LIS2DE12_INT2_SRC_ZL){     
      *val = MEMS_SET;
      return MEMS_SUCCESS;
    }
    else{  
      *val = MEMS_RESET;
      return MEMS_SUCCESS;
    }  
  }
  
  if(statusBIT == LIS2DE12_INT2_SRC_YH){
    if(value &= LIS2DE12_INT2_SRC_YH){     
      *val = MEMS_SET;
      return MEMS_SUCCESS;
    }
    else{  
      *val = MEMS_RESET;
      return MEMS_SUCCESS;
    }  
  }
  
  if(statusBIT == LIS2DE12_INT2_SRC_YL){
    if(value &= LIS2DE12_INT2_SRC_YL){     
      *val = MEMS_SET;
      return MEMS_SUCCESS;
    }
    else{  
      *val = MEMS_RESET;
      return MEMS_SUCCESS;
    }  
  }
  if(statusBIT == LIS2DE12_INT2_SRC_XH){
    if(value &= LIS2DE12_INT2_SRC_XH){     
      *val = MEMS_SET;
      return MEMS_SUCCESS;
    }
    else{  
      *val = MEMS_RESET;
      return MEMS_SUCCESS;
    }  
  }
  
  if(statusBIT == LIS2DE12_INT2_SRC_XL){
    if(value &= LIS2DE12_INT2_SRC_XL){     
      *val = MEMS_SET;
      return MEMS_SUCCESS;
    }
    else{  
      *val = MEMS_RESET;
      return MEMS_SUCCESS;
    }  
  }
  return MEMS_ERROR;
}

/*******************************************************************************
* Function Name  : LIS2DE12_GetFifoSourceReg
* Description    : Read Fifo source Register
* Input          : Byte to empty by FIFO source register value
* Output         : None
* Return         : Status [MEMS_ERROR, MEMS_SUCCESS]
*******************************************************************************/
status_t LIS2DE12_GetFifoSourceReg(void *handle, u8_t* val) {
  
  if( !LIS2DE12_ACC_ReadReg(handle, LIS2DE12_FIFO_SRC_REG, val, 1) )
    return MEMS_ERROR;
  
  return MEMS_SUCCESS;
}

/*******************************************************************************
* Function Name  : LIS2DE12_GetFifoSourceBit
* Description    : Read Fifo WaterMark source bit
* Input          : statusBIT: LIS2DE12_FIFO_SRC_WTM, LIS2DE12_FIFO_SRC_OVRUN, LIS2DE12_FIFO_SRC_EMPTY
*				   val: Byte to fill  with the bit value
* Output         : None
* Return         : Status of BIT [MEMS_ERROR, MEMS_SUCCESS]
*******************************************************************************/
status_t LIS2DE12_GetFifoSourceBit(void *handle, u8_t statusBIT,  u8_t* val){
  u8_t value;  
  
  if( !LIS2DE12_ACC_ReadReg(handle, LIS2DE12_FIFO_SRC_REG, &value, 1) )
    return MEMS_ERROR;
  
  
  if(statusBIT == LIS2DE12_FIFO_SRC_WTM){
    if(value &= LIS2DE12_FIFO_SRC_WTM){     
      *val = MEMS_SET;
      return MEMS_SUCCESS;
    }
    else{  
      *val = MEMS_RESET;
      return MEMS_SUCCESS;
    }  
  }
  
  if(statusBIT == LIS2DE12_FIFO_SRC_OVRUN){
    if(value &= LIS2DE12_FIFO_SRC_OVRUN){     
      *val = MEMS_SET;
      return MEMS_SUCCESS;
    }
    else{  
      *val = MEMS_RESET;
      return MEMS_SUCCESS;
    }  
  }
  if(statusBIT == LIS2DE12_FIFO_SRC_EMPTY){
    if(value &= statusBIT == LIS2DE12_FIFO_SRC_EMPTY){     
      *val = MEMS_SET;
      return MEMS_SUCCESS;
    }
    else{  
      *val = MEMS_RESET;
      return MEMS_SUCCESS;
    }  
  }
  return MEMS_ERROR;
}

/*******************************************************************************
* Function Name  : LIS2DE12_GetFifoSourceFSS
* Description    : Read current number of unread samples stored in FIFO
* Input          : Byte to empty by FIFO unread sample value
* Output         : None
* Return         : Status [value of FSS]
*******************************************************************************/
status_t LIS2DE12_GetFifoSourceFSS(void *handle, u8_t* val){
  u8_t value;
  
  if( !LIS2DE12_ACC_ReadReg(handle, LIS2DE12_FIFO_SRC_REG, &value, 1) )
    return MEMS_ERROR;
  
  value &= 0x1F;
  
  *val = value;
  
  return MEMS_SUCCESS;
}
  
/*******************************************************************************
* Function Name  : LIS2DE12_SetSPIInterface
* Description    : Set SPI mode: 3 Wire Interface OR 4 Wire Interface
* Input          : LIS2DE12_SPI_3_WIRE, LIS2DE12_SPI_4_WIRE
* Output         : None
* Return         : Status [MEMS_ERROR, MEMS_SUCCESS]
*******************************************************************************/
status_t LIS2DE12_SetSPIInterface(void *handle, LIS2DE12_SPIMode_t spi) {
  u8_t value;
  
  if( !LIS2DE12_ACC_ReadReg(handle, LIS2DE12_CTRL_REG4, &value, 1) )
    return MEMS_ERROR;
  
  value &= 0xFE;
  value |= spi<<LIS2DE12_SIM;
  
  if( !LIS2DE12_ACC_WriteReg(handle, LIS2DE12_CTRL_REG4, &value, 1) )
    return MEMS_ERROR;
  
  return MEMS_SUCCESS;
}

/*******************************************************************************
* Function Name  : LIS2DE12_SetActTHS
* Description    : Set sleep to wake threshold
* Input          : Sleep to wake Threshold value [0-127]
* Output         : None
* Return         : Status [MEMS_ERROR, MEMS_SUCCESS]
*******************************************************************************/
status_t LIS2DE12_SetActTHS(void *handle, u8_t ths) {
  
  if(ths>127)     
    return MEMS_ERROR;
  
  if( !LIS2DE12_ACC_WriteReg(handle, LIS2DE12_ACT_THS, &ths, 1) )
    return MEMS_ERROR;
  
  return MEMS_SUCCESS;
}

/*******************************************************************************
* Function Name  : LIS2DE12_SetActDUR
* Description    : Set sleep to wake duration
* Input          : Sleep to wake duration value [0-127]
* Output         : None
* Return         : Status [MEMS_ERROR, MEMS_SUCCESS]
*******************************************************************************/
status_t LIS2DE12_SetActDUR(void *handle, u8_t dur) {
  
  if(dur>127)     
    return MEMS_ERROR;
  
  if( !LIS2DE12_ACC_WriteReg(handle, LIS2DE12_ACT_DUR, &dur, 1) )
    return MEMS_ERROR;
  
  return MEMS_SUCCESS;
}