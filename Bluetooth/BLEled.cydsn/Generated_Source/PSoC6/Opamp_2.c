/*******************************************************************************
* File Name: Opamp_2.c
* Version 1.0
*
* Description:
*  This file provides the source code to the API for the Opamp_2
*  component.
*
* Note:
*  None.
*
********************************************************************************
* Copyright 2017, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/
#include "Opamp_2.h"

uint8_t Opamp_2_initVar = 0u;

/* Initialize opampPower to customizer selection. */
cy_en_ctb_power_t Opamp_2_opampPower = CY_CTB_POWER_HIGH;

const cy_stc_ctb_opamp_config_t Opamp_2_opampConfig =
{
    .deepSleep      = CY_CTB_DEEPSLEEP_DISABLE,
    .oaPower        = CY_CTB_POWER_HIGH,
    .oaMode         = CY_CTB_MODE_OPAMP10X,
    .oaPump         = Opamp_2_OPAMP_CHARGEPUMP,
    .oaCompEdge     = CY_CTB_COMP_EDGE_DISABLE,
    .oaCompLevel    = CY_CTB_COMP_DSI_TRIGGER_OUT_PULSE,
    .oaCompBypass   = CY_CTB_COMP_BYPASS_SYNC,
    .oaCompHyst     = CY_CTB_COMP_HYST_DISABLE,
    .oaCompIntrEn   = false,
};

/*******************************************************************************
* Function Name: Opamp_2_Start
****************************************************************************//**
*
* Initialize the Opamp_2 with default customizer
* values when called the first time and enables the Opamp_2.
* For subsequent calls the configuration is left unchanged and the opamp is
* just enabled.
*
* Opamp_2_initVar: this global variable is used to indicate the initial
* configuration of this component. The variable is initialized to zero and set
* to 1 the first time Opamp_2_Start() is called. This allows
* enabling/disabling a component without re-initialization in all subsequent
* calls to the Opamp_2_Start() routine.
*
*******************************************************************************/
void Opamp_2_Start(void)
{
    if(0u == Opamp_2_initVar)
    {
       Opamp_2_Init();         /* Initialize and turn on the hardware block. */
       Opamp_2_initVar = 1u;
    }

    /* Turn on the CTB and the opamp by setting the power level. */
    Opamp_2_Enable();
}


/* [] END OF FILE */

