/*******************************************************************************
* File Name: Opamp_2.h
* Version 1.0
*
* Description:
*  This file provides constants and parameter values for the Opamp_2
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

#if !defined(CY_Opamp_2_H)
#define CY_Opamp_2_H

#include <cy_device_headers.h>
#include <ctb/cy_ctb.h>

/*******************************************************************************
                            Internal Constants
*******************************************************************************/
#define Opamp_2_CTB_HW                         CTBM0
#define Opamp_2_OPAMP_NUM                      ((Opamp_2_cy_mxs40_opamp__OA_IDX == 0u) \
                                                        ? CY_CTB_OPAMP_0 : CY_CTB_OPAMP_1)

/*******************************************************************************
                                  Parameters
The parameters that are set in the customizer are redefined as constants here.
*******************************************************************************/
#define Opamp_2_PARAM_Power                    (3UL)
#define Opamp_2_PARAM_OutputDrive              (1UL)
#define Opamp_2_PARAM_DeepSleep                (0UL)

/*******************************************************************************
                    Variables with External Linkage
*******************************************************************************/

/** Tracks whether block is initialized (1) or not (0). */
extern uint8_t Opamp_2_initVar;

/** Tracks the power level setting. Initialized to the power level setting in the customizer. */
extern cy_en_ctb_power_t Opamp_2_opampPower;

/** Configuration structure for initializing one opamp using the CTB PDL. */
extern const cy_stc_ctb_opamp_config_t Opamp_2_opampConfig;


/*******************************************************************************
                        Opamp Configuration Defines
Constants used in the configuration structure for initializing the Opamp
*******************************************************************************/
#if (CYDEV_SYSTEM_AREF_CURRENT == CYDEV_SYSTEM_AREF_CURRENT_LOW)
    /* If the low reference current is used, disable the charge pump. */
    #define Opamp_2_OPAMP_CHARGEPUMP           CY_CTB_PUMP_DISABLE
#else
    /* Charge pump configuration depends on Deep Sleep selection in the customizer. */
    #define Opamp_2_OPAMP_CHARGEPUMP           CY_CTB_PUMP_ENABLE
#endif
/*******************************************************************************
                        Function Prototypes
*******************************************************************************/
void Opamp_2_Start(void);
__STATIC_INLINE void Opamp_2_Init(void);
__STATIC_INLINE void Opamp_2_Enable(void);
__STATIC_INLINE void Opamp_2_Stop(void);
__STATIC_INLINE void Opamp_2_SetPower(cy_en_ctb_power_t power);
__STATIC_INLINE void Opamp_2_SetDeepSleepMode(cy_en_ctb_deep_sleep_t deepSleep);

/*******************************************************************************
* Function Name: Opamp_2_Init
****************************************************************************//**
*
* Initialize the component according to the customizer settings.
* This function can also be used to restore the customizer settings
* if they are changed at run time.
*
* It is not necessary to call Opamp_2_Init() because
* Opamp_2_Start() calls this function. Opamp_2_Start() is the
* preferred method to begin component operation.
*
*******************************************************************************/
__STATIC_INLINE void Opamp_2_Init(void)
{
    (void) Cy_CTB_OpampInit(Opamp_2_CTB_HW, Opamp_2_OPAMP_NUM, &Opamp_2_opampConfig);
}

/*******************************************************************************
* Function Name: Opamp_2_Enable
****************************************************************************//**
*
* Turn on the CTB and set the opamp power level based on last call to
* Opamp_2_SetPower(). If Opamp_2_SetPower() was never called,
* the power level set in the customizer is used.
*
* The other half of the CTB block is unaffected.
*
*******************************************************************************/
__STATIC_INLINE void Opamp_2_Enable(void)
{
    Cy_CTB_Enable(Opamp_2_CTB_HW);
    Cy_CTB_SetPower(Opamp_2_CTB_HW, Opamp_2_OPAMP_NUM, Opamp_2_opampPower, Opamp_2_OPAMP_CHARGEPUMP);
}

/*******************************************************************************
* Function Name: Opamp_2_Stop
****************************************************************************//**
*
* Turn off the opamp in the CTB block.
*
* The other half of the CTB block is unaffected.
*
*******************************************************************************/
__STATIC_INLINE void Opamp_2_Stop(void)
{
    Cy_CTB_SetPower(Opamp_2_CTB_HW, Opamp_2_OPAMP_NUM, CY_CTB_POWER_OFF, Opamp_2_OPAMP_CHARGEPUMP);
}

/*******************************************************************************
* Function Name: Opamp_2_SetPower
****************************************************************************//**
*
* Set the power level for the opamp.
*
* \param power
* - CY_CTB_POWER_OFF: This input option does nothing. Opamp_2_Stop()
*   is the preferred method to turn off the opamp.
* - CY_CTB_POWER_LOW: Low power level
* - CY_CTB_POWER_MEDIUM: Medium power level
* - CY_CTB_POWER_HIGH: High power level
*
*******************************************************************************/
__STATIC_INLINE void Opamp_2_SetPower(cy_en_ctb_power_t power)
{
    if (CY_CTB_POWER_OFF != power)
    {
        Cy_CTB_SetPower(Opamp_2_CTB_HW, Opamp_2_OPAMP_NUM, power, Opamp_2_OPAMP_CHARGEPUMP);

        /* Update the tracking variable for the opamp power level */
        Opamp_2_opampPower = power;
    }
}

/*******************************************************************************
* Function Name: Opamp_2_SetDeepSleepMode
****************************************************************************//**
*
* Set whether the entire CTB block continues to stay powered in Deep Sleep mode.
* This impacts both opamps in the CTB. If there are multiple instances of
* the Opamp component or other components that use the opamp resource
* (for example, Comparator or VDAC12 components), this will affect all instances.
*
* If Opamp_2_Stop() is called before entering Deep Sleep mode,
* the opamp will remain off in Deep Sleep.
*
* Deep Sleep should not be enabled at run time if it was not enabled
* in the customizer. Analog routing and AREF block configuration are performed
* at build time. If Deep Sleep is not enabled in the customizer at build time
* the AREF block and the analog route may not be available during Deep Sleep.
*
* \param deepSleep
* - CY_CTB_DEEPSLEEP_DISABLE: Disable entire CTB is Deep Sleep mode
* - CY_CTB_DEEPSLEEP_ENABLE: Enable entire CTB in Deep Sleep mode
*
*******************************************************************************/
__STATIC_INLINE void Opamp_2_SetDeepSleepMode(cy_en_ctb_deep_sleep_t deepSleep)
{
    Cy_CTB_SetDeepSleepMode(Opamp_2_CTB_HW, deepSleep);
}

#endif /* !defined(CY_Opamp_2_H) */


/* [] END OF FILE */
